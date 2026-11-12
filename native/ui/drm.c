/* Noxis Recovery (native) - DRM/KMS output
 *
 * Opens the panel, sets a mode and hands out a cairo surface per frame. No
 * compositor, no X, no Wayland: the recovery owns the screen.
 *
 * Double buffered - we draw into the back buffer and page-flip, so a half
 * drawn frame is never on screen.
 *
 * Made by 3rr0r303YT on TikTok - GNU GPL v3
 */
#define _GNU_SOURCE
#include "drm.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

static int create_fb(nx_drm *d, nx_fb *fb)
{
    struct drm_mode_create_dumb creq = {
        .width  = d->mode.hdisplay,
        .height = d->mode.vdisplay,
        .bpp    = 32,
    };
    if (drmIoctl(d->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0) {
        fprintf(stderr, "noxis-ui: CREATE_DUMB: %s\n", strerror(errno));
        return -1;
    }
    fb->handle = creq.handle;
    fb->stride = creq.pitch;
    fb->size   = creq.size;

    uint32_t handles[4] = { fb->handle }, pitches[4] = { fb->stride }, offsets[4] = { 0 };
    if (drmModeAddFB2(d->fd, creq.width, creq.height, DRM_FORMAT_XRGB8888,
                      handles, pitches, offsets, &fb->id, 0) < 0) {
        fprintf(stderr, "noxis-ui: AddFB2: %s\n", strerror(errno));
        return -1;
    }

    struct drm_mode_map_dumb mreq = { .handle = fb->handle };
    if (drmIoctl(d->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0) {
        fprintf(stderr, "noxis-ui: MAP_DUMB: %s\n", strerror(errno));
        return -1;
    }
    fb->map = mmap(NULL, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED, d->fd, mreq.offset);
    if (fb->map == MAP_FAILED) {
        fprintf(stderr, "noxis-ui: mmap: %s\n", strerror(errno));
        return -1;
    }
    memset(fb->map, 0, fb->size);

    fb->surface = cairo_image_surface_create_for_data(
        fb->map, CAIRO_FORMAT_RGB24, creq.width, creq.height, fb->stride);
    if (cairo_surface_status(fb->surface) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "noxis-ui: cairo surface failed\n");
        return -1;
    }
    return 0;
}

static void destroy_fb(nx_drm *d, nx_fb *fb)
{
    if (fb->surface) cairo_surface_destroy(fb->surface);
    if (fb->map && fb->map != MAP_FAILED) munmap(fb->map, fb->size);
    if (fb->id) drmModeRmFB(d->fd, fb->id);
    if (fb->handle) {
        struct drm_mode_destroy_dumb dreq = { .handle = fb->handle };
        drmIoctl(d->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    }
    memset(fb, 0, sizeof(*fb));
}

/* Pick the connected connector with the largest mode - on a phone there is
 * exactly one, but a cable or test rig can add more. */
static int pick_connector(nx_drm *d, drmModeRes *res)
{
    for (int i = 0; i < res->count_connectors; i++) {
        drmModeConnector *c = drmModeGetConnector(d->fd, res->connectors[i]);
        if (!c) continue;
        if (c->connection == DRM_MODE_CONNECTED && c->count_modes > 0) {
            d->connector_id = c->connector_id;
            d->mode = c->modes[0];
            for (int m = 1; m < c->count_modes; m++)
                if (c->modes[m].hdisplay * c->modes[m].vdisplay >
                    d->mode.hdisplay * d->mode.vdisplay)
                    d->mode = c->modes[m];

            /* the encoder tells us which CRTC can drive it */
            drmModeEncoder *e = c->encoder_id ? drmModeGetEncoder(d->fd, c->encoder_id) : NULL;
            if (e) { d->crtc_id = e->crtc_id; drmModeFreeEncoder(e); }
            if (!d->crtc_id && c->count_encoders > 0) {
                e = drmModeGetEncoder(d->fd, c->encoders[0]);
                if (e) {
                    for (int k = 0; k < res->count_crtcs; k++)
                        if (e->possible_crtcs & (1 << k)) { d->crtc_id = res->crtcs[k]; break; }
                    drmModeFreeEncoder(e);
                }
            }
            drmModeFreeConnector(c);
            return d->crtc_id ? 0 : -1;
        }
        drmModeFreeConnector(c);
    }
    return -1;
}

int nx_drm_open(nx_drm *d, const char *path)
{
    memset(d, 0, sizeof(*d));
    d->fd = open(path ? path : "/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (d->fd < 0) {
        fprintf(stderr, "noxis-ui: open %s: %s\n", path ? path : "/dev/dri/card0", strerror(errno));
        return -1;
    }

    drmModeRes *res = drmModeGetResources(d->fd);
    if (!res) { fprintf(stderr, "noxis-ui: GetResources: %s\n", strerror(errno)); goto fail; }
    if (pick_connector(d, res) < 0) {
        fprintf(stderr, "noxis-ui: no connected display\n");
        drmModeFreeResources(res); goto fail;
    }
    d->saved_crtc = drmModeGetCrtc(d->fd, d->crtc_id);
    drmModeFreeResources(res);

    if (create_fb(d, &d->fb[0]) < 0 || create_fb(d, &d->fb[1]) < 0) goto fail;

    if (drmModeSetCrtc(d->fd, d->crtc_id, d->fb[0].id, 0, 0,
                       &d->connector_id, 1, &d->mode) < 0) {
        fprintf(stderr, "noxis-ui: SetCrtc: %s\n", strerror(errno));
        goto fail;
    }
    d->front = 0;
    fprintf(stderr, "noxis-ui: %ux%u on connector %u\n",
            d->mode.hdisplay, d->mode.vdisplay, d->connector_id);
    return 0;

fail:
    nx_drm_close(d);
    return -1;
}

cairo_surface_t *nx_drm_back_surface(nx_drm *d)
{
    return d->fb[d->front ^ 1].surface;
}

int nx_drm_present(nx_drm *d)
{
    int back = d->front ^ 1;
    cairo_surface_flush(d->fb[back].surface);
    if (drmModeSetCrtc(d->fd, d->crtc_id, d->fb[back].id, 0, 0,
                       &d->connector_id, 1, &d->mode) < 0) {
        fprintf(stderr, "noxis-ui: present: %s\n", strerror(errno));
        return -1;
    }
    d->front = back;
    return 0;
}

void nx_drm_close(nx_drm *d)
{
    if (d->fd < 0) return;
    if (d->saved_crtc) {
        drmModeSetCrtc(d->fd, d->saved_crtc->crtc_id, d->saved_crtc->buffer_id,
                       d->saved_crtc->x, d->saved_crtc->y,
                       &d->connector_id, 1, &d->saved_crtc->mode);
        drmModeFreeCrtc(d->saved_crtc);
        d->saved_crtc = NULL;
    }
    destroy_fb(d, &d->fb[0]);
    destroy_fb(d, &d->fb[1]);
    close(d->fd);
    d->fd = -1;
}
