/* Noxis Recovery (native) - device probe
 *
 * Everything the UI needs to know about the phone: which partitions exist,
 * which slot is active, how much room is left. Read-only - nothing here
 * changes the device.
 *
 * Paths are taken from a root prefix so the whole thing can be pointed at a
 * fixture directory and tested on a desktop.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#define _GNU_SOURCE
#include "device.h"

#include <dirent.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <limits.h>
#include <unistd.h>

static const char *root_prefix(void)
{
    const char *p = getenv("NOXIS_ROOT");
    return p ? p : "";
}

static void joinp(char *out, size_t n, const char *fmt, ...)
{
    va_list ap;
    int k = snprintf(out, n, "%s", root_prefix());
    if (k < 0 || (size_t)k >= n) { out[0] = 0; return; }
    va_start(ap, fmt);
    vsnprintf(out + k, n - k, fmt, ap);
    va_end(ap);
}

/* Reads a whole small file, trims the trailing newline. */
static int read_line(const char *path, char *buf, size_t n)
{
    FILE *f = fopen(path, "re");
    if (!f) return -1;
    if (!fgets(buf, (int)n, f)) { fclose(f); return -1; }
    fclose(f);
    buf[strcspn(buf, "\r\n")] = 0;
    return 0;
}

/* The active slot comes from the kernel command line on A/B devices. It is
 * "_a" or "_b"; anything else means a device without slots. */
int nx_dev_slot(char *out, size_t n)
{
    char path[512], line[4096];
    joinp(path, sizeof path, "/proc/cmdline");
    if (read_line(path, line, sizeof line) < 0) return -1;

    const char *key = "androidboot.slot_suffix=";
    char *p = strstr(line, key);
    if (!p) {
        key = "androidboot.slot=";
        p = strstr(line, key);
        if (!p) return -1;
    }
    p += strlen(key);
    size_t i = 0;
    while (p[i] && p[i] != ' ' && i + 1 < n) { out[i] = p[i]; i++; }
    out[i] = 0;
    return i ? 0 : -1;
}

/* /dev/block/by-name is the map from partition name to device node. */
int nx_dev_partitions(nx_partition *out, int max)
{
    char dir[512];
    joinp(dir, sizeof dir, "/dev/block/by-name");
    DIR *d = opendir(dir);
    if (!d) return 0;

    int n = 0;
    struct dirent *e;
    while ((e = readdir(d)) && n < max) {
        if (e->d_name[0] == '.') continue;
        snprintf(out[n].name, sizeof out[n].name, "%s", e->d_name);
        snprintf(out[n].path, sizeof out[n].path, "%s/%s", dir, e->d_name);

        /* size via the block device's sysfs entry, falling back to stat */
        out[n].size = 0;
        struct stat st;
        if (stat(out[n].path, &st) == 0) {
            char real[PATH_MAX];
            if (realpath(out[n].path, real)) {
                const char *base = strrchr(real, '/');
                if (base) {
                    char sp[PATH_MAX], sz[64];
                    snprintf(sp, sizeof sp, "/sys/class/block/%s/size", base + 1);
                    if (read_line(sp, sz, sizeof sz) == 0)
                        out[n].size = strtoull(sz, NULL, 10) * 512ULL;
                }
            }
        }
        n++;
    }
    closedir(d);
    return n;
}

const nx_partition *nx_dev_find(const nx_partition *parts, int n, const char *name)
{
    for (int i = 0; i < n; i++)
        if (strcmp(parts[i].name, name) == 0) return &parts[i];
    return NULL;
}

/* Partition for a name plus the active slot: "boot" + "_a" -> "boot_a",
 * falling back to the unsuffixed name on devices without slots. */
const nx_partition *nx_dev_find_slotted(const nx_partition *parts, int n,
                                        const char *name, const char *slot)
{
    if (slot && *slot) {
        char with[96];
        snprintf(with, sizeof with, "%s%s", name, slot);
        const nx_partition *p = nx_dev_find(parts, n, with);
        if (p) return p;
    }
    return nx_dev_find(parts, n, name);
}

int nx_dev_free_space(const char *mount_point, uint64_t *free_bytes)
{
    struct statvfs s;
    if (statvfs(mount_point, &s) != 0) return -1;
    *free_bytes = (uint64_t)s.f_bavail * s.f_frsize;
    return 0;
}

void nx_dev_human_size(uint64_t bytes, char *out, size_t n)
{
    const char *u[] = { "B", "KB", "MB", "GB", "TB" };
    double v = (double)bytes;
    int i = 0;
    while (v >= 1024.0 && i < 4) { v /= 1024.0; i++; }
    if (i == 0) snprintf(out, n, "%llu %s", (unsigned long long)bytes, u[i]);
    else if (v >= 100) snprintf(out, n, "%.0f %s", v, u[i]);
    else snprintf(out, n, "%.1f %s", v, u[i]);
}

/* The running system, read from the build properties of the mounted system
 * partition. Returns "unbekannt" rather than failing: the UI has to show
 * something. */
int nx_dev_system_name(char *out, size_t n)
{
    static const char *files[] = {
        "/system/build.prop", "/system_root/system/build.prop", "/vendor/build.prop",
    };
    static const char *keys[] = {
        "ro.noxis.version=", "ro.build.display.id=", "ro.product.system.name=",
    };
    for (size_t f = 0; f < sizeof files / sizeof *files; f++) {
        char path[512];
        joinp(path, sizeof path, "%s", files[f]);
        FILE *fp = fopen(path, "re");
        if (!fp) continue;
        char line[512];
        while (fgets(line, sizeof line, fp)) {
            for (size_t k = 0; k < sizeof keys / sizeof *keys; k++) {
                if (strncmp(line, keys[k], strlen(keys[k])) == 0) {
                    const char *v = line + strlen(keys[k]);
                    snprintf(out, n, "%s", v);
                    out[strcspn(out, "\r\n")] = 0;
                    fclose(fp);
                    return 0;
                }
            }
        }
        fclose(fp);
    }
    snprintf(out, n, "unbekannt");
    return -1;
}
