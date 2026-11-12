# Noxis Recovery (native)

An own recovery for the Nothing Phone (3a), built from scratch on the Noxis
kernel - not a reskin of OrangeFox or TWRP.

The OrangeFox-based recovery in `noxis/` stays as it is. This is the second,
independent track.

## Why a second track

`noxis/` themes a finished OrangeFox image. That gets a familiar recovery with
the Noxis look in minutes, but the layout is OrangeFox's and the limits are
OrangeFox's - the Wi-Fi and rollback screens from `design/mock.html` cannot be
built there, only bolted on as shell scripts.

This track builds the whole thing: kernel, initramfs, UI. Everything on screen
is ours.

## Size budget

The recovery partition holds about 100 MB:

| | |
|---|---|
| kernel (Image.gz) | ~13 MB |
| busybox + tools | ~2 MB |
| UI binary + Cairo/Pango | ~8 MB |
| fonts, assets | ~4 MB |
| headroom | the rest |

A WebKit build would need 80-120 MB on its own, so `design/mock.html` cannot be
rendered directly on the device. The UI reproduces those screens natively.

## Status

Early. The build produces a bootable image; the UI is being built screen by
screen against the mockups.

Made by 3rr0r303YT on TikTok - GNU GPL v3
