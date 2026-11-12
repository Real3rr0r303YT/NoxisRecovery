/* Noxis Recovery (native) - touch input
 *
 * Reads the raw evdev multitouch protocol. We only care about the first
 * finger: a recovery needs taps, not gestures.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#define _GNU_SOURCE
#include "input.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <linux/input.h>

#define BITS_PER_LONG (8 * (int)sizeof(long))
#define NBITS(x)      (((x) - 1) / BITS_PER_LONG + 1)
#define TEST_BIT(bit, array) ((array[(bit) / BITS_PER_LONG] >> ((bit) % BITS_PER_LONG)) & 1)

/* A touchscreen reports absolute X/Y and a touch contact. A mouse reports
 * relative motion, a button reports keys - neither is what we want. */
static int is_touchscreen(int fd)
{
    unsigned long ev[NBITS(EV_MAX)], abs[NBITS(ABS_MAX)];
    memset(ev, 0, sizeof ev);
    memset(abs, 0, sizeof abs);
    if (ioctl(fd, EVIOCGBIT(0, sizeof ev), ev) < 0) return 0;
    if (!TEST_BIT(EV_ABS, ev)) return 0;
    if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof abs), abs) < 0) return 0;
    int has_xy = TEST_BIT(ABS_X, abs) || TEST_BIT(ABS_MT_POSITION_X, abs);
    int has_mt = TEST_BIT(ABS_MT_POSITION_X, abs) || TEST_BIT(ABS_MT_TRACKING_ID, abs);
    return has_xy && (has_mt || TEST_BIT(ABS_Y, abs));
}

static void read_range(int fd, int axis, int *lo, int *hi)
{
    struct input_absinfo a;
    if (ioctl(fd, EVIOCGABS(axis), &a) == 0 && a.maximum > a.minimum) {
        *lo = a.minimum;
        *hi = a.maximum;
    }
}

int nx_input_open(nx_input *in)
{
    memset(in, 0, sizeof(*in));
    in->fd = -1;
    in->max_x = in->max_y = 0;

    DIR *d = opendir("/dev/input");
    if (!d) { fprintf(stderr, "noxis-ui: /dev/input: %s\n", strerror(errno)); return -1; }

    struct dirent *e;
    while ((e = readdir(d))) {
        if (strncmp(e->d_name, "event", 5) != 0) continue;
        char path[300];
        snprintf(path, sizeof path, "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) continue;
        if (is_touchscreen(fd)) {
            char name[128] = "?";
            ioctl(fd, EVIOCGNAME(sizeof name), name);
            in->fd = fd;
            read_range(fd, ABS_MT_POSITION_X, &in->min_x, &in->max_x);
            read_range(fd, ABS_MT_POSITION_Y, &in->min_y, &in->max_y);
            if (!in->max_x) read_range(fd, ABS_X, &in->min_x, &in->max_x);
            if (!in->max_y) read_range(fd, ABS_Y, &in->min_y, &in->max_y);
            fprintf(stderr, "noxis-ui: touch '%s' on %s (%d..%d, %d..%d)\n",
                    name, path, in->min_x, in->max_x, in->min_y, in->max_y);
            break;
        }
        close(fd);
    }
    closedir(d);
    return in->fd >= 0 ? 0 : -1;
}

/* Raw coordinates are in the panel's own range, which need not match the
 * framebuffer. Scale, and guard against a zero range. */
static int scale(int v, int lo, int hi, int out)
{
    if (hi <= lo || out <= 0) return v;
    long s = (long)(v - lo) * (out - 1) / (hi - lo);
    if (s < 0) s = 0;
    if (s > out - 1) s = out - 1;
    return (int)s;
}

int nx_input_poll(nx_input *in, nx_touch *t, int timeout_ms)
{
    if (in->fd < 0) return 0;

    struct pollfd p = { .fd = in->fd, .events = POLLIN };
    if (poll(&p, 1, timeout_ms) <= 0) return 0;

    struct input_event ev;
    int reported = 0;
    while (read(in->fd, &ev, sizeof ev) == (ssize_t)sizeof ev) {
        if (ev.type == EV_ABS) {
            switch (ev.code) {
            case ABS_MT_POSITION_X: case ABS_X: in->cur_x = ev.value; break;
            case ABS_MT_POSITION_Y: case ABS_Y: in->cur_y = ev.value; break;
            case ABS_MT_TRACKING_ID: in->down = (ev.value != -1); break;
            }
        } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
            in->down = (ev.value != 0);
        } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
            t->x = in->screen_w ? scale(in->cur_x, in->min_x, in->max_x, in->screen_w)
                                : in->cur_x;
            t->y = in->screen_h ? scale(in->cur_y, in->min_y, in->max_y, in->screen_h)
                                : in->cur_y;
            t->pressed = in->down;
            reported = 1;
        }
    }
    return reported;
}

void nx_input_close(nx_input *in)
{
    if (in->fd >= 0) close(in->fd);
    in->fd = -1;
}
