/* Noxis Recovery (native) - operations
 *
 * The things that actually change the device: writing an image to a
 * partition, formatting, backing up, switching slots.
 *
 * Every one of these can brick a phone, so they check before they write:
 * the target has to exist, the source has to fit, and a refusal is always
 * cheaper than a half-written boot partition. Progress goes to a callback so
 * the UI can show it without this file knowing about cairo.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#define _GNU_SOURCE
#include "ops.h"

#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHUNK (4 * 1024 * 1024)

static void report(nx_progress_fn cb, void *ud, const char *stage,
                   uint64_t done, uint64_t total)
{
    if (cb) cb(ud, stage, done, total);
}

static int fail(char *err, size_t n, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (err && n) vsnprintf(err, n, fmt, ap);
    va_end(ap);
    return -1;
}

static uint64_t file_size(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    if (S_ISBLK(st.st_mode)) {
        int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0) return 0;
        off_t end = lseek(fd, 0, SEEK_END);
        close(fd);
        return end > 0 ? (uint64_t)end : 0;
    }
    return (uint64_t)st.st_size;
}

/* Streams src to dst with progress. Both may be files or block devices. */
static int copy_stream(const char *src, const char *dst, uint64_t expect,
                       const char *stage, nx_progress_fn cb, void *ud,
                       char *err, size_t errn)
{
    int in = open(src, O_RDONLY | O_CLOEXEC);
    if (in < 0) return fail(err, errn, "%s: %s", src, strerror(errno));

    int out = open(dst, O_WRONLY | O_CREAT | O_CLOEXEC, 0600);
    if (out < 0) { close(in); return fail(err, errn, "%s: %s", dst, strerror(errno)); }

    char *buf = malloc(CHUNK);
    if (!buf) { close(in); close(out); return fail(err, errn, "out of memory"); }

    uint64_t done = 0;
    int rc = 0;
    for (;;) {
        ssize_t r = read(in, buf, CHUNK);
        if (r < 0) { rc = fail(err, errn, "read %s: %s", src, strerror(errno)); break; }
        if (r == 0) break;

        ssize_t off = 0;
        while (off < r) {
            ssize_t w = write(out, buf + off, (size_t)(r - off));
            if (w <= 0) { rc = fail(err, errn, "write %s: %s", dst, strerror(errno)); break; }
            off += w;
        }
        if (rc) break;
        done += (uint64_t)r;
        report(cb, ud, stage, done, expect);
    }

    free(buf);
    /* a partition write that is not on the flash when we return is not done */
    if (rc == 0 && fsync(out) != 0)
        rc = fail(err, errn, "fsync %s: %s", dst, strerror(errno));
    close(in);
    close(out);
    return rc;
}

int nx_op_flash_image(const char *image, const nx_partition *target,
                      nx_progress_fn cb, void *ud, char *err, size_t errn)
{
    if (!image || !target) return fail(err, errn, "no image or target");

    uint64_t isz = file_size(image);
    if (isz == 0) return fail(err, errn, "%s is empty or missing", image);

    /* The one check that matters: an image larger than the partition would be
     * written until the device errors out, leaving it unbootable. */
    if (target->size && isz > target->size)
        return fail(err, errn, "image is %llu bytes, %s holds %llu",
                    (unsigned long long)isz, target->name,
                    (unsigned long long)target->size);

    return copy_stream(image, target->path, isz, "flash", cb, ud, err, errn);
}

int nx_op_backup_partition(const nx_partition *src, const char *dest_dir,
                           nx_progress_fn cb, void *ud, char *err, size_t errn)
{
    if (!src || !dest_dir) return fail(err, errn, "no source or destination");

    char out[512];
    snprintf(out, sizeof out, "%s/%s.img", dest_dir, src->name);

    uint64_t free_bytes = 0;
    if (nx_dev_free_space(dest_dir, &free_bytes) == 0 &&
        src->size && free_bytes < src->size)
        return fail(err, errn, "%s needs %llu bytes, %llu free",
                    src->name, (unsigned long long)src->size,
                    (unsigned long long)free_bytes);

    return copy_stream(src->path, out, src->size, "backup", cb, ud, err, errn);
}

/* Runs a helper and waits. Returns its exit status, or -1 if it could not
 * start. argv must be NULL terminated. */
static int run(char *const argv[], char *err, size_t errn)
{
    pid_t p = fork();
    if (p < 0) return fail(err, errn, "fork: %s", strerror(errno));
    if (p == 0) {
        execvp(argv[0], argv);
        _exit(127);
    }
    int st = 0;
    if (waitpid(p, &st, 0) < 0) return fail(err, errn, "waitpid: %s", strerror(errno));
    if (!WIFEXITED(st)) return fail(err, errn, "%s died", argv[0]);
    if (WEXITSTATUS(st) == 127) return fail(err, errn, "%s not found", argv[0]);
    if (WEXITSTATUS(st) != 0)
        return fail(err, errn, "%s exited %d", argv[0], WEXITSTATUS(st));
    return 0;
}

int nx_op_format(const nx_partition *target, nx_fs_type fs,
                 char *err, size_t errn)
{
    if (!target) return fail(err, errn, "no target");

    /* Android keeps userdata on f2fs and metadata on f2fs; formatting one as
     * ext4 leaves a system that boots into a loop. Pick deliberately. */
    char *const ext4[] = { "mkfs.ext4", "-F", "-q", (char *)target->path, NULL };
    char *const f2fs[] = { "mkfs.f2fs", "-f", (char *)target->path, NULL };

    switch (fs) {
    case NX_FS_EXT4: return run(ext4, err, errn);
    case NX_FS_F2FS: return run(f2fs, err, errn);
    default:         return fail(err, errn, "unknown filesystem");
    }
}

/* Zeroing the first megabytes is enough to make a partition look empty to
 * anything that reads a superblock, and is far quicker than a full wipe. */
int nx_op_wipe(const nx_partition *target, char *err, size_t errn)
{
    if (!target) return fail(err, errn, "no target");

    int fd = open(target->path, O_WRONLY | O_CLOEXEC);
    if (fd < 0) return fail(err, errn, "%s: %s", target->path, strerror(errno));

    size_t n = 16 * 1024 * 1024;
    if (target->size && target->size < n) n = (size_t)target->size;

    char *zeros = calloc(1, CHUNK);
    if (!zeros) { close(fd); return fail(err, errn, "out of memory"); }

    size_t done = 0;
    int rc = 0;
    while (done < n) {
        size_t want = n - done > CHUNK ? CHUNK : n - done;
        ssize_t w = write(fd, zeros, want);
        if (w <= 0) { rc = fail(err, errn, "write: %s", strerror(errno)); break; }
        done += (size_t)w;
    }
    free(zeros);
    if (rc == 0 && fsync(fd) != 0) rc = fail(err, errn, "fsync: %s", strerror(errno));
    close(fd);
    return rc;
}

/* The bootloader control block decides which slot boots next. Going through
 * bootctl rather than writing the block ourselves keeps us out of a format
 * that differs between vendors. */
int nx_op_set_active_slot(const char *slot, char *err, size_t errn)
{
    if (!slot || !*slot) return fail(err, errn, "no slot");

    const char *s = slot;
    if (*s == '_') s++;                       /* accept "_a" and "a" */
    if ((*s != 'a' && *s != 'b') || s[1])
        return fail(err, errn, "slot must be a or b, got '%s'", slot);

    char idx[2] = { (char)(*s == 'a' ? '0' : '1'), 0 };
    char *const argv[] = { "bootctl", "set-active-boot-slot", idx, NULL };
    return run(argv, err, errn);
}
