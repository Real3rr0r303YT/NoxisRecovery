#!/bin/bash
# Repacks a built OrangeFox recovery image with the Noxis theme and tools.
# usage: noxis/repack.sh <input.img> <output.img>   (needs magiskboot in PATH, python3)
set -euo pipefail
IN=$(readlink -f "$1"); OUT=$(readlink -f "$2")
HERE=$(cd "$(dirname "$0")" && pwd)
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
cd "$W"
magiskboot unpack -h "$IN" >/dev/null 2>&1
mkdir orig new
(cd orig && magiskboot cpio ../ramdisk.cpio extract >/dev/null 2>&1)
cp -a orig/. new/
mkdir -p new/system/bin
cp "$HERE/root/system/bin/noxis-rollback" new/system/bin/
python3 "$HERE/theme/apply.py" new
# add every new or changed file back into the ramdisk
python3 - "$W" > cmds.txt <<'PY'
import os, sys, filecmp
w = sys.argv[1]; a, b = os.path.join(w, "orig"), os.path.join(w, "new")
for top in ("twres", "system/bin/noxis-rollback"):
    p = os.path.join(b, top)
    walk = os.walk(p) if os.path.isdir(p) else [(os.path.dirname(p), [], [os.path.basename(p)])]
    for dp, _, fs in walk:
        rel = os.path.relpath(dp, b)
        if not os.path.isdir(os.path.join(a, rel)): print(f"mkdir 0755 {rel}")
        for f in fs:
            r = os.path.join(rel, f); src = os.path.join(b, r)
            mode = "0755" if os.access(src, os.X_OK) else "0644"
            if not os.path.exists(os.path.join(a, r)) or not filecmp.cmp(os.path.join(a, r), src, shallow=False):
                print(f"add {mode} {r} {src}")
PY
mapfile -t CMDS < cmds.txt
echo "[noxis] ${#CMDS[@]} ramdisk changes"
magiskboot cpio ramdisk.cpio "${CMDS[@]}" >/dev/null
magiskboot cpio ramdisk.cpio "exists twres/images/Noxis/wallpaper.png" "exists system/bin/noxis-rollback"
magiskboot repack "$IN" "$OUT" >/dev/null 2>&1
ls -la "$OUT"
