/* Noxis Recovery (native) - icons
 *
 * Drawn as paths, not bitmaps: they scale to any tile size and recolour with
 * the theme. Each one is authored in a 24x24 box and scaled to the requested
 * size, the same way the mockups use their icon set.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#define _GNU_SOURCE
#include "icons.h"

#include <math.h>

/* Set up a 24x24 user space centred on (x, y) at the requested size. */
static void icon_begin(cairo_t *cr, double x, double y, double size)
{
    cairo_save(cr);
    /* cairo_show_text leaves a current point behind; without clearing it the
       first arc of an icon draws a line back to the end of the last label */
    cairo_new_path(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, size / 24.0, size / 24.0);
    cairo_set_line_width(cr, 2.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
}

static void icon_end(cairo_t *cr) { cairo_restore(cr); }

/* ---------------------------------------------------------------- shapes */

static void path_download(cairo_t *cr)      /* Installieren */
{
    cairo_move_to(cr, 12, 3);  cairo_line_to(cr, 12, 15);
    cairo_move_to(cr, 6.5, 10); cairo_line_to(cr, 12, 15.5); cairo_line_to(cr, 17.5, 10);
    cairo_stroke(cr);
    cairo_move_to(cr, 4, 19); cairo_line_to(cr, 20, 19);
    cairo_stroke(cr);
}

static void path_trash(cairo_t *cr)         /* Wipe */
{
    cairo_move_to(cr, 4, 6.5); cairo_line_to(cr, 20, 6.5);
    cairo_stroke(cr);
    cairo_move_to(cr, 9, 6.5); cairo_line_to(cr, 9, 4.2);
    cairo_line_to(cr, 15, 4.2); cairo_line_to(cr, 15, 6.5);
    cairo_stroke(cr);
    cairo_move_to(cr, 6.2, 6.5); cairo_line_to(cr, 7.2, 20); cairo_line_to(cr, 16.8, 20);
    cairo_line_to(cr, 17.8, 6.5);
    cairo_stroke(cr);
    cairo_move_to(cr, 10, 10); cairo_line_to(cr, 10.4, 16.5);
    cairo_move_to(cr, 14, 10); cairo_line_to(cr, 13.6, 16.5);
    cairo_stroke(cr);
}

static void path_plus_box(cairo_t *cr)      /* Backup */
{
    cairo_rectangle(cr, 3.5, 3.5, 17, 17);
    cairo_stroke(cr);
    cairo_move_to(cr, 12, 8);  cairo_line_to(cr, 12, 16);
    cairo_move_to(cr, 8,  12); cairo_line_to(cr, 16, 12);
    cairo_stroke(cr);
}

static void path_restore(cairo_t *cr)       /* Wiederherstellen */
{
    cairo_arc(cr, 12, 12, 8, M_PI * 0.35, M_PI * 1.85);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.2, 7.0); cairo_line_to(cr, 4.0, 12.2); cairo_line_to(cr, 9.2, 11.0);
    cairo_close_path(cr);
    cairo_fill(cr);
}

static void path_rollback(cairo_t *cr)      /* Rollback */
{
    cairo_move_to(cr, 20, 17); cairo_curve_to(cr, 20, 9, 14, 7, 6, 7);
    cairo_stroke(cr);
    cairo_move_to(cr, 10.5, 3); cairo_line_to(cr, 5, 7); cairo_line_to(cr, 10.5, 11);
    cairo_close_path(cr);
    cairo_fill(cr);
}

static void path_globe(cairo_t *cr)         /* Browser */
{
    cairo_arc(cr, 12, 12, 8.5, 0, 2 * M_PI);
    cairo_stroke(cr);
    cairo_move_to(cr, 3.5, 12); cairo_line_to(cr, 20.5, 12);
    cairo_stroke(cr);
    /* two meridians */
    cairo_save(cr);
    cairo_translate(cr, 12, 12);
    cairo_scale(cr, 0.45, 1.0);
    cairo_translate(cr, -12, -12);
    cairo_arc(cr, 12, 12, 8.5, 0, 2 * M_PI);
    cairo_restore(cr);
    cairo_stroke(cr);
}

static void path_folder(cairo_t *cr)        /* Dateien */
{
    cairo_move_to(cr, 3.5, 19);
    cairo_line_to(cr, 3.5, 6);
    cairo_line_to(cr, 9.5, 6);
    cairo_line_to(cr, 11.5, 8.5);
    cairo_line_to(cr, 20.5, 8.5);
    cairo_line_to(cr, 20.5, 19);
    cairo_close_path(cr);
    cairo_stroke(cr);
}

static void path_terminal(cairo_t *cr)      /* Terminal */
{
    cairo_rectangle(cr, 3.5, 4.5, 17, 15);
    cairo_stroke(cr);
    cairo_move_to(cr, 7, 9.5); cairo_line_to(cr, 10.5, 12.5); cairo_line_to(cr, 7, 15.5);
    cairo_stroke(cr);
    cairo_move_to(cr, 12.5, 15.5); cairo_line_to(cr, 17, 15.5);
    cairo_stroke(cr);
}

static void path_home(cairo_t *cr)          /* nav: Start */
{
    cairo_move_to(cr, 3.5, 11); cairo_line_to(cr, 12, 3.5); cairo_line_to(cr, 20.5, 11);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.5, 10.5); cairo_line_to(cr, 5.5, 20); cairo_line_to(cr, 18.5, 20);
    cairo_line_to(cr, 18.5, 10.5);
    cairo_stroke(cr);
}

static void path_gear(cairo_t *cr)          /* nav: Einstellungen */
{
    /* outline with eight teeth, then the hub */
    const double ri = 6.4, ro = 9.3, half = M_PI / 16.0;
    for (int i = 0; i < 8; i++) {
        double a = i * M_PI / 4.0;
        if (i == 0) cairo_move_to(cr, 12 + cos(a - half * 2) * ri,
                                      12 + sin(a - half * 2) * ri);
        cairo_line_to(cr, 12 + cos(a - half) * ro, 12 + sin(a - half) * ro);
        cairo_line_to(cr, 12 + cos(a + half) * ro, 12 + sin(a + half) * ro);
        cairo_line_to(cr, 12 + cos(a + half * 2) * ri, 12 + sin(a + half * 2) * ri);
        double next = (i + 1) * M_PI / 4.0;
        cairo_line_to(cr, 12 + cos(next - half * 2) * ri,
                          12 + sin(next - half * 2) * ri);
    }
    cairo_close_path(cr);
    cairo_stroke(cr);
    cairo_new_path(cr);
    cairo_arc(cr, 12, 12, 3.2, 0, 2 * M_PI);
    cairo_stroke(cr);
}

static void path_power(cairo_t *cr)         /* nav: Neustart */
{
    cairo_arc(cr, 12, 13, 7.5, -M_PI * 0.35, M_PI * 1.35);
    cairo_stroke(cr);
    cairo_move_to(cr, 12, 3); cairo_line_to(cr, 12, 11);
    cairo_stroke(cr);
}

/* ---------------------------------------------------------------- public */

void nx_icon(cairo_t *cr, nx_icon_id id, double x, double y,
             double size, nx_rgba col)
{
    icon_begin(cr, x, y, size);
    nx_set(cr, col);
    /* strokes are authored at 2.0 in a 24px box; keep that ratio at any size */
    cairo_set_line_width(cr, 2.0);

    switch (id) {
    case NX_ICON_INSTALL:  path_download(cr);  break;
    case NX_ICON_WIPE:     path_trash(cr);     break;
    case NX_ICON_BACKUP:   path_plus_box(cr);  break;
    case NX_ICON_RESTORE:  path_restore(cr);   break;
    case NX_ICON_ROLLBACK: path_rollback(cr);  break;
    case NX_ICON_BROWSER:  path_globe(cr);     break;
    case NX_ICON_FILES:    path_folder(cr);    break;
    case NX_ICON_TERMINAL: path_terminal(cr);  break;
    case NX_ICON_HOME:     path_home(cr);      break;
    case NX_ICON_GEAR:     path_gear(cr);      break;
    case NX_ICON_POWER:    path_power(cr);     break;
    default: break;
    }
    icon_end(cr);
}
