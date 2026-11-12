/* Noxis Recovery (native) - DRM/KMS output
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#ifndef NOXIS_DRM_H
#define NOXIS_DRM_H

#include <stdint.h>
#include <cairo/cairo.h>
#include <xf86drmMode.h>

typedef struct {
    uint32_t         handle, id, stride;
    uint64_t         size;
    void            *map;
    cairo_surface_t *surface;
} nx_fb;

typedef struct {
    int              fd;
    uint32_t         connector_id, crtc_id;
    drmModeModeInfo  mode;
    drmModeCrtc     *saved_crtc;
    nx_fb            fb[2];
    int              front;      /* index of the buffer currently on screen */
} nx_drm;

/* Opens the panel and sets a mode. path NULL means /dev/dri/card0. */
int  nx_drm_open(nx_drm *d, const char *path);

/* The surface to draw this frame into - never the one being displayed. */
cairo_surface_t *nx_drm_back_surface(nx_drm *d);

/* Puts the drawn buffer on screen and makes the other one the back buffer. */
int  nx_drm_present(nx_drm *d);

/* Restores the mode that was set before we took over. */
void nx_drm_close(nx_drm *d);

#endif /* NOXIS_DRM_H */
