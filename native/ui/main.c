/* Noxis Recovery (native) - entry point
 *
 *   noxis-ui              draw on /dev/dri/card0, read touch, run
 *   noxis-ui --png FILE   render one frame to a PNG and exit
 *
 * The PNG mode exists so the screens can be checked against the mockups on a
 * desktop, without flashing anything.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "screens.h"
#ifndef NX_NO_DRM
#include "drm.h"
#include "input.h"
#endif

static nx_state state = {
    .version        = "v1.0",
    .system_name    = "Noxis OS 1.0",
    .slot           = "A (B = vorher)",
    .storage        = "34 GB frei",
    .wifi           = "aus",
    .wifi_connected = 0,
    .nav_active     = 0,
    .pressed_tile   = -1,
};

static int render_png(const char *path, int w, int h)
{
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
    if (cairo_surface_status(s) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "noxis-ui: cannot create %dx%d surface\n", w, h);
        return 1;
    }
    cairo_t *cr = cairo_create(s);

    nx_home home;
    nx_home_layout(&home, w, h);
    nx_home_draw(cr, &home, &state);

    cairo_destroy(cr);
    cairo_status_t rc = cairo_surface_write_to_png(s, path);
    cairo_surface_destroy(s);
    if (rc != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "noxis-ui: writing %s: %s\n", path, cairo_status_to_string(rc));
        return 1;
    }
    fprintf(stderr, "noxis-ui: wrote %s (%dx%d)\n", path, w, h);
    return 0;
}

#ifdef NX_NO_DRM
/* Built without libdrm: the PNG preview still works, the device path does not. */
static int run_device(void)
{
    fprintf(stderr, "noxis-ui: built without DRM support - use --png FILE\n");
    return 1;
}
#else
static int run_device(void)
{
    nx_drm drm;
    if (nx_drm_open(&drm, NULL) < 0) return 1;

    nx_input in;
    if (nx_input_open(&in) < 0)
        fprintf(stderr, "noxis-ui: no touch device - display only\n");

    nx_home home;
    nx_home_layout(&home, drm.mode.hdisplay, drm.mode.vdisplay);

    int running = 1, dirty = 1;
    while (running) {
        if (dirty) {
            cairo_surface_t *s = nx_drm_back_surface(&drm);
            cairo_t *cr = cairo_create(s);
            nx_home_draw(cr, &home, &state);
            cairo_destroy(cr);
            if (nx_drm_present(&drm) < 0) break;
            dirty = 0;
        }

        nx_touch t;
        if (nx_input_poll(&in, &t, 50) > 0) {
            if (t.pressed) {
                int tile = nx_home_hit(&home, t.x, t.y);
                if (tile != state.pressed_tile) { state.pressed_tile = tile; dirty = 1; }
            } else {
                int tile = nx_home_hit(&home, t.x, t.y);
                int nav  = nx_home_nav_hit(&home, t.x, t.y);
                if (tile >= 0)
                    fprintf(stderr, "noxis-ui: tile '%s'\n", nx_home_tile_title(tile));
                if (nav >= 0 && nav != state.nav_active) state.nav_active = nav;
                if (state.pressed_tile != -1) { state.pressed_tile = -1; }
                dirty = 1;
            }
        }
    }

    nx_input_close(&in);
    nx_drm_close(&drm);
    return 0;
}
#endif /* NX_NO_DRM */

int main(int argc, char **argv)
{
    int w = NX_W, h = NX_H;
    const char *png = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--png") && i + 1 < argc)      png = argv[++i];
        else if (!strcmp(argv[i], "--size") && i + 1 < argc) sscanf(argv[++i], "%dx%d", &w, &h);
        else if (!strcmp(argv[i], "--wifi") && i + 1 < argc) {
            state.wifi = argv[++i];
            state.wifi_connected = 1;
        } else {
            fprintf(stderr, "usage: %s [--png FILE] [--size WxH] [--wifi SSID]\n", argv[0]);
            return 2;
        }
    }

    return png ? render_png(png, w, h) : run_device();
}
