#!/usr/bin/env python3
"""Download an Alpine aarch64 package with all its shared-library dependencies and unpack
them into one folder, to run musl programs (wpa_supplicant) inside the Android recovery.
usage: alpine-bundle.py <alpine branch e.g. v3.22> <outdir> <pkg> [pkg ...]"""
import io, sys, tarfile, urllib.request

branch, out, wanted = sys.argv[1], sys.argv[2], sys.argv[3:]
pkgs, provides = {}, {}
for repo in ("main", "community"):
    base = f"https://dl-cdn.alpinelinux.org/alpine/{branch}/{repo}/aarch64"
    idx = tarfile.open(fileobj=io.BytesIO(urllib.request.urlopen(f"{base}/APKINDEX.tar.gz").read()))
    for block in idx.extractfile("APKINDEX").read().decode().split("\n\n"):
        f = {l[0]: l[2:] for l in block.splitlines() if len(l) > 2 and l[1] == ":"}
        if "P" not in f or f["P"] in pkgs: continue
        f["base"] = base; pkgs[f["P"]] = f
        for p in f.get("p", "").split():
            provides.setdefault(p.split("=")[0], f["P"])

todo, done = list(wanted), []
while todo:
    n = todo.pop()
    if n in done: continue
    if n not in pkgs: n = provides.get(n.split("=")[0].split(">")[0].split("<")[0], None)
    if not n or n in done: continue
    done.append(n)
    for d in pkgs[n].get("D", "").split():
        if d.startswith("!") or d.startswith("/"): continue
        todo.append(d)

for n in done:
    f = pkgs[n]; url = f"{f['base']}/{n}-{f['V']}.apk"
    data = urllib.request.urlopen(url).read()
    # an .apk is several gzip streams; tarfile reads them one after another
    with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz", ignore_zeros=True) as t:
        t.extractall(out, members=[m for m in t.getmembers() if not m.name.startswith(".")], filter="tar")
    print(f"{n} {f['V']}")
