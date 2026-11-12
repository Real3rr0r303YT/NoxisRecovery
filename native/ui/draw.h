/* Noxis Recovery (native) - drawing primitives
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#ifndef NOXIS_DRAW_H
#define NOXIS_DRAW_H

#include <cairo/cairo.h>
#include "theme.h"

typedef struct { int x, y, w, h; } nx_rect;

void   nx_set(cairo_t *cr, nx_rgba c);
void   nx_rounded_path(cairo_t *cr, double x, double y, double w, double h, double r);
void   nx_card(cairo_t *cr, double x, double y, double w, double h,
               double r, nx_rgba fill, int hairline);
void   nx_shadow(cairo_t *cr, double x, double y, double w, double h,
                 double r, double spread);
void   nx_pill(cairo_t *cr, double x, double y, double w, double h, nx_rgba col);
void   nx_vgradient(cairo_t *cr, double x, double y, double w, double h,
                    nx_rgba top, nx_rgba bottom);

void   nx_text(cairo_t *cr, const char *s, double x, double y,
               const char *family, double size, nx_rgba col, int weight);
void   nx_text_centered(cairo_t *cr, const char *s, double cx, double y,
                        const char *family, double size, nx_rgba col, int weight);
double nx_text_width(cairo_t *cr, const char *s, const char *family,
                     double size, int weight);

void   nx_wallpaper(cairo_t *cr, double w, double h);
int    nx_hit(const nx_rect *r, int x, int y);

#endif /* NOXIS_DRAW_H */
