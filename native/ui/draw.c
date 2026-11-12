/* Noxis Recovery (native) - drawing primitives
 *
 * The pieces the mockups are made of: rounded cards, the glass fill, the red
 * accent pill, text. Everything else composes these.
 *
 * On the device there is no backdrop-filter, so "glass" is a translucent white
 * over a pre-blurred wallpaper rather than a live blur.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#define _GNU_SOURCE   /* M_PI under -std=c11 */
#include "draw.h"

#include <math.h>
#include <string.h>

void nx_set(cairo_t *cr, nx_rgba c)
{
    cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
}

void nx_rounded_path(cairo_t *cr, double x, double y, double w, double h, double r)
{
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r,     r, -M_PI / 2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0,          M_PI / 2);
    cairo_arc(cr, x + r,     y + h - r, r, M_PI / 2,   M_PI);
    cairo_arc(cr, x + r,     y + r,     r, M_PI,       3 * M_PI / 2);
    cairo_close_path(cr);
}

void nx_card(cairo_t *cr, double x, double y, double w, double h,
             double r, nx_rgba fill, int hairline)
{
    nx_rounded_path(cr, x, y, w, h, r);
    nx_set(cr, fill);
    cairo_fill_preserve(cr);
    if (hairline) {
        nx_set(cr, NX_HAIRLINE);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
    } else {
        cairo_new_path(cr);
    }
}

/* A soft drop shadow under a card. Cheap approximation: a few stacked rounded
 * rects with falling alpha - a real gaussian is not worth the cycles here. */
void nx_shadow(cairo_t *cr, double x, double y, double w, double h,
               double r, double spread)
{
    for (int i = (int)spread; i > 0; i--) {
        double a = 0.045 * (1.0 - (double)i / spread);
        cairo_set_source_rgba(cr, 0, 0, 0, a);
        nx_rounded_path(cr, x - i, y - i + spread * 0.35,
                        w + 2 * i, h + 2 * i, r + i);
        cairo_fill(cr);
    }
}

void nx_text(cairo_t *cr, const char *s, double x, double y,
             const char *family, double size, nx_rgba col, int weight)
{
    if (!s || !*s) return;
    cairo_select_font_face(cr, family,
                           CAIRO_FONT_SLANT_NORMAL,
                           weight ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, size);
    nx_set(cr, col);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, s);
}

double nx_text_width(cairo_t *cr, const char *s, const char *family,
                     double size, int weight)
{
    if (!s || !*s) return 0;
    cairo_text_extents_t e;
    cairo_select_font_face(cr, family, CAIRO_FONT_SLANT_NORMAL,
                           weight ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, size);
    cairo_text_extents(cr, s, &e);
    return e.x_advance;
}

void nx_text_centered(cairo_t *cr, const char *s, double cx, double y,
                      const char *family, double size, nx_rgba col, int weight)
{
    double w = nx_text_width(cr, s, family, size, weight);
    nx_text(cr, s, cx - w / 2, y, family, size, col, weight);
}

/* The accent pill behind the active nav item, and the primary buttons. */
void nx_pill(cairo_t *cr, double x, double y, double w, double h, nx_rgba col)
{
    nx_rounded_path(cr, x, y, w, h, h / 2);
    nx_set(cr, col);
    cairo_fill(cr);
}

/* Vertical gradient, used for the wallpaper stand-in and sheet backgrounds. */
void nx_vgradient(cairo_t *cr, double x, double y, double w, double h,
                  nx_rgba top, nx_rgba bottom)
{
    cairo_pattern_t *p = cairo_pattern_create_linear(x, y, x, y + h);
    cairo_pattern_add_color_stop_rgba(p, 0, top.r, top.g, top.b, top.a);
    cairo_pattern_add_color_stop_rgba(p, 1, bottom.r, bottom.g, bottom.b, bottom.a);
    cairo_rectangle(cr, x, y, w, h);
    cairo_set_source(cr, p);
    cairo_fill(cr);
    cairo_pattern_destroy(p);
}

/* The mockup wallpaper: blurred colour blobs over near-black. A 120px CSS
 * blur on a circle is, to the eye, a radial gradient falling to transparent -
 * which costs one pattern instead of a convolution. */
void nx_wallpaper(cairo_t *cr, double w, double h)
{
    nx_set(cr, NX_WALL_BG);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);

    const nx_blob blobs[] = NX_BLOBS;
    for (int i = 0; i < NX_BLOB_COUNT; i++) {
        const nx_blob *b = &blobs[i];
        nx_rgba c = NX_RGBA(b->hex, b->alpha);
        cairo_pattern_t *p = cairo_pattern_create_radial(b->cx, b->cy, 0,
                                                         b->cx, b->cy, b->r);
        cairo_pattern_add_color_stop_rgba(p, 0.00, c.r, c.g, c.b, c.a);
        cairo_pattern_add_color_stop_rgba(p, 0.45, c.r, c.g, c.b, c.a * 0.55);
        cairo_pattern_add_color_stop_rgba(p, 1.00, c.r, c.g, c.b, 0.0);
        cairo_set_source(cr, p);
        cairo_rectangle(cr, 0, 0, w, h);
        cairo_fill(cr);
        cairo_pattern_destroy(p);
    }
}

int nx_hit(const nx_rect *r, int x, int y)
{
    return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h;
}
