#!/bin/bash
# Applies the Noxis changes on top of the OrangeFox asteroids tree.
# usage: apply.sh <fox source dir> <device path>
set -euo pipefail
SRC="$1"; DEV="$SRC/$2"
HERE="$(cd "$(dirname "$0")" && pwd)"

# Noxis tools inside the recovery (OrangeFox terminal: noxis-rollback)
cp -r "$HERE/root/." "$DEV/recovery/root/"
chmod 0755 "$DEV/recovery/root/system/bin/"noxis-*

# maintainer name shown in OrangeFox
sed -i 's/^OF_MAINTAINER := .*/OF_MAINTAINER := 3rr0r303YT/' "$DEV/fox.mk"
grep -n '^OF_MAINTAINER' "$DEV/fox.mk"

# list theme files so the liquid-glass reskin can target them
find "$SRC/bootable/recovery/gui/theme" -maxdepth 3 \( -name '*.xml' -o -iname '*splash*' -o -iname '*logo*' \) 2>/dev/null | head -60 || true
