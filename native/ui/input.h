/* Noxis Recovery (native) - touch input
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#ifndef NOXIS_INPUT_H
#define NOXIS_INPUT_H

typedef struct { int x, y, pressed; } nx_touch;

typedef struct {
    int fd;
    int min_x, max_x, min_y, max_y;   /* raw ranges, for scaling to pixels */
    int screen_w, screen_h;
    int cur_x, cur_y, down;
} nx_input;

/* Finds the touchscreen under /dev/input and reads its axis ranges. */
int  nx_input_open(nx_input *in);

/* Waits up to timeout_ms for a finger. Returns 1 when *t was filled. */
int  nx_input_poll(nx_input *in, nx_touch *t, int timeout_ms);

void nx_input_close(nx_input *in);

#endif /* NOXIS_INPUT_H */
