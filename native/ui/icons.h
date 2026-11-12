/* Noxis Recovery (native) - icons
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#ifndef NOXIS_ICONS_H
#define NOXIS_ICONS_H

#include "draw.h"

typedef enum {
    NX_ICON_NONE = 0,
    NX_ICON_INSTALL,
    NX_ICON_WIPE,
    NX_ICON_BACKUP,
    NX_ICON_RESTORE,
    NX_ICON_ROLLBACK,
    NX_ICON_BROWSER,
    NX_ICON_FILES,
    NX_ICON_TERMINAL,
    NX_ICON_HOME,
    NX_ICON_GEAR,
    NX_ICON_POWER,
} nx_icon_id;

/* Draws the icon with its top-left corner at (x, y), size px on a side. */
void nx_icon(cairo_t *cr, nx_icon_id id, double x, double y,
             double size, nx_rgba col);

#endif /* NOXIS_ICONS_H */
