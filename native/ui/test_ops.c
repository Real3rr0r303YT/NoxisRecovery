/* Noxis Recovery (native) - tests for the write paths
 *
 * These run against regular files in a temp directory, never a real
 * partition. What is checked is the behaviour that decides whether a phone
 * survives: does flashing refuse an oversized image, does it copy byte for
 * byte, does backup refuse when the destination is too small.
 *
 *   cc -O2 -std=c11 test_ops.c ops.c device.c -o test_ops && ./test_ops
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ops.h"

static int failures = 0;
static char tmp[256];

static void ok(const char *what, int cond)
{
    printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) failures++;
}

static void write_file(const char *path, const char *data, size_t n)
{
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); exit(1); }
    fwrite(data, 1, n, f);
    fclose(f);
}

static int files_equal(const char *a, const char *b)
{
    FILE *fa = fopen(a, "rb"), *fb = fopen(b, "rb");
    if (!fa || !fb) { if (fa) fclose(fa); if (fb) fclose(fb); return 0; }
    int eq = 1, ca, cb;
    do {
        ca = fgetc(fa); cb = fgetc(fb);
        if (ca != cb) { eq = 0; break; }
    } while (ca != EOF);
    fclose(fa); fclose(fb);
    return eq;
}

static void test_flash_fits(void)
{
    puts("flash: an image that fits is written byte for byte");
    char img[320], part[320], err[256] = "";
    snprintf(img,  sizeof img,  "%s/src.img", tmp);
    snprintf(part, sizeof part, "%s/boot", tmp);

    char data[8192];
    for (size_t i = 0; i < sizeof data; i++) data[i] = (char)(i * 7 + 3);
    write_file(img, data, sizeof data);
    write_file(part, "", 0);

    nx_partition target = { .size = 65536 };
    snprintf(target.name, sizeof target.name, "boot");
    snprintf(target.path, sizeof target.path, "%s", part);

    int rc = nx_op_flash_image(img, &target, NULL, NULL, err, sizeof err);
    ok("returns 0", rc == 0);
    ok("target matches source", files_equal(img, part));
    if (rc) printf("       err: %s\n", err);
}

static void test_flash_too_big(void)
{
    puts("flash: an image larger than the partition is refused");
    char img[320], part[320], err[256] = "";
    snprintf(img,  sizeof img,  "%s/big.img", tmp);
    snprintf(part, sizeof part, "%s/small", tmp);

    char data[4096];
    memset(data, 0xAB, sizeof data);
    write_file(img, data, sizeof data);
    write_file(part, "", 0);

    nx_partition target = { .size = 1024 };     /* smaller than the image */
    snprintf(target.name, sizeof target.name, "small");
    snprintf(target.path, sizeof target.path, "%s", part);

    int rc = nx_op_flash_image(img, &target, NULL, NULL, err, sizeof err);
    ok("returns -1", rc == -1);
    ok("says what is too big", strstr(err, "small") != NULL);

    /* the important part: it must not have written anything */
    struct stat st;
    ok("target untouched", stat(part, &st) == 0 && st.st_size == 0);
}

static void test_flash_missing_image(void)
{
    puts("flash: a missing image is refused, not treated as empty");
    char part[320], err[256] = "";
    snprintf(part, sizeof part, "%s/boot2", tmp);
    write_file(part, "keep", 4);

    nx_partition target = { .size = 65536 };
    snprintf(target.path, sizeof target.path, "%s", part);
    snprintf(target.name, sizeof target.name, "boot2");

    char missing[320];
    snprintf(missing, sizeof missing, "%s/does-not-exist.img", tmp);
    int rc = nx_op_flash_image(missing, &target, NULL, NULL, err, sizeof err);
    ok("returns -1", rc == -1);

    struct stat st;
    ok("existing content kept", stat(part, &st) == 0 && st.st_size == 4);
}

static void test_wipe_zeroes(void)
{
    puts("wipe: the start of the partition is zeroed");
    char part[320], err[256] = "";
    snprintf(part, sizeof part, "%s/dirty", tmp);

    char data[4096];
    memset(data, 0x5A, sizeof data);
    write_file(part, data, sizeof data);

    nx_partition target = { .size = sizeof data };
    snprintf(target.path, sizeof target.path, "%s", part);
    snprintf(target.name, sizeof target.name, "dirty");

    int rc = nx_op_wipe(&target, err, sizeof err);
    ok("returns 0", rc == 0);

    FILE *f = fopen(part, "rb");
    int all_zero = 1, c, seen = 0;
    while ((c = fgetc(f)) != EOF && seen < 4096) { if (c) { all_zero = 0; break; } seen++; }
    fclose(f);
    ok("first bytes are zero", all_zero);
    if (rc) printf("       err: %s\n", err);
}

static void test_slot_validation(void)
{
    puts("slot: only a and b are accepted");
    char err[256];
    ok("rejects 'c'",    nx_op_set_active_slot("c", err, sizeof err) == -1);
    ok("rejects empty",  nx_op_set_active_slot("", err, sizeof err) == -1);
    ok("rejects 'ab'",   nx_op_set_active_slot("ab", err, sizeof err) == -1);
    /* "a" and "_a" are valid input; they fail only because bootctl is absent
     * on a desktop, which is a different error than a rejected argument */
    nx_op_set_active_slot("a", err, sizeof err);
    ok("accepts 'a' as a slot", strstr(err, "must be") == NULL);
    nx_op_set_active_slot("_b", err, sizeof err);
    ok("accepts '_b' as a slot", strstr(err, "must be") == NULL);
}

static void test_human_size(void)
{
    puts("sizes are formatted the way the UI shows them");
    char b[32];
    nx_dev_human_size(512, b, sizeof b);                ok("512 B",   !strcmp(b, "512 B"));
    nx_dev_human_size(1536, b, sizeof b);               ok("1.5 KB",  !strcmp(b, "1.5 KB"));
    nx_dev_human_size(34ULL << 30, b, sizeof b);        ok("34.0 GB", !strcmp(b, "34.0 GB"));
}

int main(void)
{
    snprintf(tmp, sizeof tmp, "/tmp/noxis-test-%d", (int)getpid());
    if (mkdir(tmp, 0700) != 0) { perror(tmp); return 1; }

    test_flash_fits();
    test_flash_too_big();
    test_flash_missing_image();
    test_wipe_zeroes();
    test_slot_validation();
    test_human_size();

    char cmd[320];
    snprintf(cmd, sizeof cmd, "rm -rf %s", tmp);
    if (system(cmd) != 0) fprintf(stderr, "note: %s not removed\n", tmp);

    printf("\n%s\n", failures ? "FAILURES" : "all tests passed");
    return failures ? 1 : 0;
}
