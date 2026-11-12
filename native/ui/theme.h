/* Noxis Recovery (native) - design tokens
 *
 * Taken straight out of design/mock.html so the device draws what the mockups
 * show. When a value changes there, change it here too; nothing reads the HTML
 * at runtime.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#ifndef NOXIS_THEME_H
#define NOXIS_THEME_H

/* The mockups are authored at the panel's native resolution. */
#define NX_W 1080
#define NX_H 2392

/* rgba() helper: components are 0..1 for cairo_set_source_rgba */
typedef struct { double r, g, b, a; } nx_rgba;

#define NX_RGB(hex)  ((nx_rgba){ ((hex) >> 16 & 0xff) / 255.0, \
                                 ((hex) >>  8 & 0xff) / 255.0, \
                                 ((hex)       & 0xff) / 255.0, 1.0 })
#define NX_RGBA(hex, alpha) ((nx_rgba){ ((hex) >> 16 & 0xff) / 255.0, \
                                        ((hex) >>  8 & 0xff) / 255.0, \
                                        ((hex)       & 0xff) / 255.0, (alpha) })

/* ---------------------------------------------------------------- colours */
#define NX_ACCENT      NX_RGB(0xff2a2a)   /* Noxis red, the one accent        */
#define NX_ACCENT_DIM  NX_RGBA(0xff2a2a, 0.60)
#define NX_OK          NX_RGB(0x5dff9a)   /* "connected", success             */
#define NX_WARN        NX_RGB(0xff7a5a)
#define NX_BG          NX_RGB(0x000000)

/* Glass layers. The mockups use backdrop-filter, which we cannot do on a
 * framebuffer: we blur the wallpaper once at startup and composite these
 * translucent whites on top of the blurred copy. */
#define NX_GLASS_LOW   NX_RGBA(0xffffff, 0.08)
#define NX_GLASS       NX_RGBA(0xffffff, 0.12)
#define NX_GLASS_HIGH  NX_RGBA(0xffffff, 0.18)
#define NX_GLASS_TOP   NX_RGBA(0xffffff, 0.22)
#define NX_HAIRLINE    NX_RGBA(0xffffff, 0.10)
#define NX_SHADOW      NX_RGBA(0x000000, 0.35)

/* Text */
#define NX_TEXT        NX_RGBA(0xffffff, 1.00)
#define NX_TEXT_STRONG NX_RGBA(0xffffff, 0.85)
#define NX_TEXT_MID    NX_RGBA(0xffffff, 0.70)
#define NX_TEXT_DIM    NX_RGBA(0xffffff, 0.60)
#define NX_TEXT_FAINT  NX_RGBA(0xffffff, 0.50)

/* ---------------------------------------------------------------- shape */
/* The radii the mockups actually use. */
#define NX_R_CHIP   11.0
#define NX_R_SMALL  20.0
#define NX_R_CARD   28.0
#define NX_R_TILE   34.0
#define NX_R_SHEET  40.0
#define NX_R_PILL   43.0

/* ---------------------------------------------------------------- type */
#define NX_FONT       "Inter"
#define NX_FONT_MONO  "DejaVu Sans Mono"

#define NX_SZ_TITLE   68.0
#define NX_SZ_HEAD    44.0
#define NX_SZ_BODY    34.0
#define NX_SZ_LABEL   28.0
#define NX_SZ_SMALL   24.0

/* ---------------------------------------------------------------- layout */
#define NX_PAD        56.0   /* .screen padding in the mockups */
#define NX_GAP        24.0   /* between tiles         */
#define NX_STATUS_H   96.0   /* top status strip      */
#define NX_NAV_H     160.0   /* bottom nav bar        */

/* ---------------------------------------------------------------- wallpaper
 * design/mock.html paints three .blob circles, blurred 120px, over #050507.
 * We reproduce them as radial gradients - same colours, same positions. */
#define NX_WALL_BG NX_RGB(0x050507)

typedef struct {
    double  cx, cy, r, alpha;
    int     hex;
} nx_blob;

/* centre and radius derived from the mockup's left/top/right/bottom + size,
 * radius widened by the blur so the falloff matches */
#define NX_BLOBS { \
    { 150.0,  250.0, 570.0, 0.90, 0xff2a2a }, \
    {1030.0, 1000.0, 520.0, 0.90, 0x5b2bff }, \
    { 250.0, 2242.0, 570.0, 0.60, 0xff2a7a }, \
}
#define NX_BLOB_COUNT 3

#endif /* NOXIS_THEME_H */
