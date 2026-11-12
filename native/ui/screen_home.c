/* Noxis Recovery (native) - home screen
 *
 * The layout from design/mock.html#home: header with the Noxis mark, a status
 * card (system / active slot / storage / Wi-Fi), six action tiles in two
 * columns, and the nav bar.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#include "screens.h"
#include "icons.h"

#include <stdio.h>
#include <string.h>

/* The six tiles, in the order the mockup shows them. */
static const struct {
    const char *title;
    const char *sub;
    int         accent;     /* drawn in red rather than glass */
    nx_icon_id  icon;
} TILES[] = {
    { "Installieren",   "ZIP · IMG · OTA",      0, NX_ICON_INSTALL  },
    { "Wipe",           "Cache · Data · Format",0, NX_ICON_WIPE     },
    { "Backup",         "Partitionen sichern",  0, NX_ICON_BACKUP   },
    { "Wiederherstellen","aus Backup",          0, NX_ICON_RESTORE  },
    { "Rollback",       "zurück zu altem OS",   1, NX_ICON_ROLLBACK },
    { "Browser",        "WLAN · Downloads",     0, NX_ICON_BROWSER  },
    { "Dateien",        "Manager · MTP · ADB",  0, NX_ICON_FILES    },
    { "Terminal",       "Shell · Logs",         0, NX_ICON_TERMINAL },
};
#define TILE_COUNT ((int)(sizeof(TILES) / sizeof(TILES[0])))

static const char *NAV[] = { "Start", "Dateien", "Einstellungen", "Neustart" };
static const nx_icon_id NAV_ICON[] = {
    NX_ICON_HOME, NX_ICON_FILES, NX_ICON_GEAR, NX_ICON_POWER
};
#define NAV_COUNT 4

void nx_home_layout(nx_home *h, int w, int h_px)
{
    memset(h, 0, sizeof(*h));
    h->screen_w = w;
    h->screen_h = h_px;

    double pad   = NX_PAD;
    double top   = NX_STATUS_H + 40;

    /* header block: mark + title + subtitle, then the status card */
    h->header = (nx_rect){ (int)pad, (int)top, (int)(w - 2 * pad), 150 };
    h->status = (nx_rect){ (int)pad, (int)(top + 170), (int)(w - 2 * pad), 190 };

    /* two columns of tiles below the status card */
    double grid_top = top + 170 + 190 + NX_GAP * 1.5;
    double col_w = (w - 2 * pad - NX_GAP) / 2;
    /* fill the space between the status card and the nav bar instead of
       leaving a gap at the bottom */
    int rows = (TILE_COUNT + 1) / 2;
    double grid_bottom = h_px - NX_NAV_H - pad - NX_GAP * 1.5;
    double row_h = (grid_bottom - grid_top - NX_GAP * (rows - 1)) / rows;
    if (row_h < 150) row_h = 150;

    for (int i = 0; i < TILE_COUNT; i++) {
        int col = i % 2, row = i / 2;
        h->tiles[i] = (nx_rect){
            (int)(pad + col * (col_w + NX_GAP)),
            (int)(grid_top + row * (row_h + NX_GAP)),
            (int)col_w, (int)row_h
        };
    }
    h->tile_count = TILE_COUNT;

    /* nav bar pinned to the bottom */
    double nav_h = NX_NAV_H;
    h->nav = (nx_rect){ (int)pad, (int)(h_px - nav_h - pad), (int)(w - 2 * pad), (int)nav_h };
    double item_w = (w - 2 * pad) / NAV_COUNT;
    for (int i = 0; i < NAV_COUNT; i++)
        h->nav_items[i] = (nx_rect){
            (int)(pad + i * item_w), h->nav.y, (int)item_w, (int)nav_h
        };
    h->nav_count = NAV_COUNT;
}

void nx_home_draw(cairo_t *cr, const nx_home *h, const nx_state *st)
{
    nx_wallpaper(cr, h->screen_w, h->screen_h);

    /* ---- header */
    const nx_rect *hd = &h->header;
    nx_text(cr, "Noxis Recovery", hd->x, hd->y + 56, NX_FONT, NX_SZ_TITLE, NX_TEXT, 1);
    char sub[160];
    snprintf(sub, sizeof sub, "Nothing Phone (3a) · asteroids · %s",
             st->version ? st->version : "v1.0");
    nx_text(cr, sub, hd->x, hd->y + 100, NX_FONT, NX_SZ_LABEL, NX_TEXT_DIM, 0);
    nx_text(cr, "Made by 3rr0r303YT on TikTok · GNU GPL v3",
            hd->x, hd->y + 136, NX_FONT, NX_SZ_SMALL, NX_TEXT_FAINT, 0);

    /* ---- status card: two columns of label/value */
    const nx_rect *s = &h->status;
    nx_shadow(cr, s->x, s->y, s->w, s->h, NX_R_CARD, 10);
    nx_card(cr, s->x, s->y, s->w, s->h, NX_R_CARD, NX_GLASS, 1);

    double colx[2] = { s->x + 32, s->x + s->w / 2.0 + 16 };
    const char *labels[4] = { "SYSTEM", "AKTIVER SLOT", "SPEICHER", "WLAN" };
    const char *values[4] = {
        st->system_name ? st->system_name : "unbekannt",
        st->slot        ? st->slot        : "?",
        st->storage     ? st->storage     : "-",
        st->wifi        ? st->wifi        : "aus",
    };
    for (int i = 0; i < 4; i++) {
        double x = colx[i % 2];
        double y = s->y + 52 + (i / 2) * 84;
        nx_text(cr, labels[i], x, y, NX_FONT, NX_SZ_SMALL, NX_TEXT_FAINT, 0);
        nx_rgba vc = (i == 3 && st->wifi_connected) ? NX_OK : NX_TEXT;
        nx_text(cr, values[i], x, y + 40, NX_FONT, NX_SZ_BODY, vc, 1);
    }

    /* ---- tiles */
    for (int i = 0; i < h->tile_count; i++) {
        const nx_rect *t = &h->tiles[i];
        int pressed = (st->pressed_tile == i);
        nx_rgba fill = TILES[i].accent
                     ? (pressed ? NX_ACCENT : NX_ACCENT_DIM)
                     : (pressed ? NX_GLASS_TOP : NX_GLASS_LOW);

        nx_shadow(cr, t->x, t->y, t->w, t->h, NX_R_TILE, 8);
        nx_card(cr, t->x, t->y, t->w, t->h, NX_R_TILE, fill, 1);
        nx_icon(cr, TILES[i].icon, t->x + 28, t->y + 30, 52,
                TILES[i].accent ? NX_TEXT : NX_TEXT_STRONG);
        nx_text(cr, TILES[i].title, t->x + 28, t->y + t->h - 62,
                NX_FONT, NX_SZ_HEAD, NX_TEXT, 1);
        nx_text(cr, TILES[i].sub, t->x + 28, t->y + t->h - 26,
                NX_FONT, NX_SZ_SMALL, NX_TEXT_DIM, 0);
    }

    /* ---- nav bar */
    const nx_rect *n = &h->nav;
    nx_card(cr, n->x, n->y, n->w, n->h, NX_R_PILL, NX_GLASS, 1);
    for (int i = 0; i < h->nav_count; i++) {
        const nx_rect *it = &h->nav_items[i];
        double cx = it->x + it->w / 2.0;
        if (i == st->nav_active) {
            double pw = it->w - 36, ph = n->h - 36;
            nx_pill(cr, cx - pw / 2, n->y + 18, pw, ph, NX_ACCENT);
        }
        nx_rgba nc = (i == st->nav_active) ? NX_TEXT : NX_TEXT_MID;
        nx_icon(cr, NAV_ICON[i], cx - 19, n->y + 32, 38, nc);
        nx_text_centered(cr, NAV[i], cx, n->y + n->h - 34,
                         NX_FONT, NX_SZ_SMALL, nc, i == st->nav_active);
    }
}

int nx_home_hit(const nx_home *h, int x, int y)
{
    for (int i = 0; i < h->tile_count; i++)
        if (nx_hit(&h->tiles[i], x, y)) return i;
    return -1;
}

int nx_home_nav_hit(const nx_home *h, int x, int y)
{
    for (int i = 0; i < h->nav_count; i++)
        if (nx_hit(&h->nav_items[i], x, y)) return i;
    return -1;
}

const char *nx_home_tile_title(int i)
{
    return (i >= 0 && i < TILE_COUNT) ? TILES[i].title : NULL;
}
