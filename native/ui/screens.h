/* Noxis Recovery (native) - screens
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#ifndef NOXIS_SCREENS_H
#define NOXIS_SCREENS_H

#include "draw.h"

#define NX_MAX_TILES 12
#define NX_MAX_NAV    6

/* What the UI shows about the device. Filled by the probe layer; the screens
 * only read it. */
typedef struct {
    const char *version;
    const char *system_name;    /* "Noxis OS 1.0", "Nothing OS 3.0", ... */
    const char *slot;           /* "A (B = vorher)"                      */
    const char *storage;        /* "34 GB frei"                          */
    const char *wifi;           /* SSID or "aus"                         */
    int         wifi_connected;
    int         nav_active;
    int         pressed_tile;   /* -1 when nothing is held               */
} nx_state;

typedef struct {
    int     screen_w, screen_h;
    nx_rect header, status, nav;
    nx_rect tiles[NX_MAX_TILES];
    int     tile_count;
    nx_rect nav_items[NX_MAX_NAV];
    int     nav_count;
} nx_home;

void        nx_home_layout(nx_home *h, int w, int h_px);
void        nx_home_draw(cairo_t *cr, const nx_home *h, const nx_state *st);
int         nx_home_hit(const nx_home *h, int x, int y);
int         nx_home_nav_hit(const nx_home *h, int x, int y);
const char *nx_home_tile_title(int i);

#endif /* NOXIS_SCREENS_H */
