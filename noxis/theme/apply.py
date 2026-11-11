#!/usr/bin/env python3
"""Noxis liquid-glass theme for OrangeFox.

usage: apply.py <extracted ramdisk root>

Turns the stock OrangeFox theme in <root>/twres into the Noxis look:
- blurred red/violet wallpaper with the Noxis emblem behind every full page
- frosted-glass cards, lists, nav panel and buttons (white with low alpha)
- Noxis red accent, white text, Inter Display font
- Noxis splash screen and logo, "Noxis Recovery" name
The OrangeFox layout and functions stay the same.
"""
import os, re, shutil, sys
import xml.etree.ElementTree as ET

ROOT = sys.argv[1]
TW = os.path.join(ROOT, "twres")
HERE = os.path.dirname(os.path.abspath(__file__))
ACCENT, ACCENT_DARK = "#FF2A2A", "#D41E1E"

def xml_ok(path):
    try: ET.parse(path); return True
    except ET.ParseError: return False
# OrangeFox ships a few files only its own (lenient) parser accepts – only check the others
PRE_OK = {os.path.join(dp, n) for dp, _, fs in os.walk(TW) for n in fs if n.endswith(".xml") and xml_ok(os.path.join(dp, n))}

def rd(p): return open(os.path.join(TW, p), encoding="utf-8").read()
def wr(p, s): open(os.path.join(TW, p), "w", encoding="utf-8").write(s)

# ---------------------------------------------------------------- assets
dst = os.path.join(TW, "images", "Noxis"); os.makedirs(dst, exist_ok=True)
for f in os.listdir(os.path.join(HERE, "assets")):
    shutil.copy(os.path.join(HERE, "assets", f), dst)

# ---------------------------------------------------------------- style (based on the Dark style)
s = rd("themes/styles/Dark.xml")
GLASS = "#FFFFFF1F"          # frosted card
GLASS_2 = "#FFFFFF14"        # list groups
INK, MUTED = "#F2F2F2", "#A3A3A3"
# icon / control tints of the dark style -> white ink
s = s.replace('tint="#ffe2bd"', f'tint="{INK}"')
# dark tints: active nav icons sit on the red pill -> white; dialogs/menus -> near-black glass
s = re.sub(r'(name="nav_(?:menu|wipe|files|backup)" [^>]*tint=")#100E0D', r'\g<1>#FFFFFF', s)
s = s.replace('tint="#100E0D"', 'tint="#161618"')
s = s.replace('tint="#1f1811"', 'tint="#202024"')
s = s.replace('<shape name="fs_bg_big" color="#1f1811"', f'<shape name="fs_bg_big" color="{GLASS}"')
VARS = {
    "background": "#050507", "power_bg": "#050507F2", "power_hl": "#FFFFFF22",
    "highlight_color": "#FFFFFF26", "highlight_color_full": "#FFFFFF1A",
    "text": INK, "text_secondary": MUTED, "text_action": INK, "text_status": "#FFFFFF",
    "text_menu": INK, "text_light": INK, "text_hl_btn": "#FFFFFF", "text_dark": INK,
    "disabled_bg": GLASS_2, "disabled_text": "#BDBDBD",
    "linecolor": "#FFFFFF1A", "darktransparent": "#000000B3",
    "groupitems_clr": GLASS_2, "neutral_light": GLASS, "cards_bg": GLASS,
    "keyboard_bg": "#101012F2", "keyboard_hl": "#FFFFFF22",
    "neutral_100": INK, "neutral_200": "#E6E6E6", "neutral_300": "#BDBDBD", "neutral_400": "#9E9E9E",
    "neutral_500": "#7A7A7A", "neutral_600": "#4A4A4A", "neutral_700": "#2E2E2E",
    "primary_500": ACCENT, "primary_600": ACCENT_DARK, "primary_700": "#B71C1C",
}
for k, v in VARS.items():
    s, n = re.subn(rf'(<variable name="{k}" value=")[^"]*(")', rf'\g<1>{v}\2', s)
    if n == 0:
        s = s.replace("</variables>", f'\t\t<variable name="{k}" value="{v}"/>\n\t</variables>', 1)
wr("themes/style.xml", s)
# keep the Dark choice in the theme picker identical to Noxis
wr("themes/styles/Dark.xml", s)

# ---------------------------------------------------------------- accent
a = rd("themes/accent.xml")
a = re.sub(r'#ed6f02', ACCENT, a, flags=re.I).replace("#D86100", ACCENT_DARK).replace('value="Fox"', 'value="Noxis"')
wr("themes/accent.xml", a)

# ---------------------------------------------------------------- fonts: Inter Display everywhere
f = rd("themes/font.xml")
f = re.sub(r'filename="(?:Roboto|GoogleSans)-(Regular|Medium)\.ttf"', r'filename="InterDisplay-\1.ttf"', f)
wr("themes/font.xml", f)

# ---------------------------------------------------------------- resources: wallpaper + logo
im = rd("resources/images.xml")
im = im.replace("<resources>", '<resources>\n'
    '\t\t<image name="noxis_wall" filename="Noxis/wallpaper"/>\n'
    '\t\t<image name="noxis_wall_dim" filename="Noxis/wallpaper_dim"/>\n'
    '\t\t<image name="noxis_logo" filename="Noxis/logo" retainaspect="1"/>\n'
    '\t\t<image name="noxis_logo_small" filename="Noxis/logo_small" retainaspect="1"/>', 1)
# rounder, glassier buttons and panels
im = im.replace('<shape name="nav_hl" color="#B8B8B842" radius="36"', '<shape name="nav_hl" color="#FFFFFF2E" radius="48"')
im = im.replace('<shape name="nb_hl" color="#B8B8B842"', '<shape name="nb_hl" color="#FFFFFF2E"')
im = im.replace('<shape name="search_bar_bg" color="%cards_bg%" radius="36"', '<shape name="search_bar_bg" color="%cards_bg%" radius="64"')
im = im.replace('<shape name="nav_bg" color="%cards_bg%" radius="36"', '<shape name="nav_bg" color="%cards_bg%" radius="72"')
im = im.replace('<shape name="power_card" color="%cards_bg%" radius="32"', '<shape name="power_card" color="%cards_bg%" radius="48"')
wr("resources/images.xml", im)
# the About logo
st = rd("themes/style.xml")
st = re.sub(r'<image name="logo_about" [^>]*/>', '<image name="logo_about" filename="Noxis/logo_small" retainaspect="1"/>', st)
wr("themes/style.xml", st)

# ---------------------------------------------------------------- wallpaper behind every full page
OVERLAYS = set()
for p in os.listdir(os.path.join(TW, "pages")):
    if p.endswith(".xml"):
        OVERLAYS |= set(re.findall(r'function="overlay">([^<]+)<', rd("pages/" + p)))
WALL = '<image><placement x="0" y="0"/><image resource="noxis_wall"/></image>'
count = 0
for p in sorted(os.listdir(os.path.join(TW, "pages"))):
    if not p.endswith(".xml") or p in ("recovery.xml", "dummy.xml"): continue
    s = rd("pages/" + p)
    def add(m):
        global count
        name, body = m.group(1), m.group(3)
        if name in OVERLAYS or "dialog_body" in body or "%transparent%" in body or "%power_bg%" in body:
            return m.group(0)
        count += 1
        return m.group(0).replace(m.group(2), m.group(2) + "\n\t\t\t" + WALL, 1)
    s = re.sub(r'<page name="([^"]+)"[^>]*?(>)(.*?)</page>', lambda m: add(m), s, flags=re.S)
    wr("pages/" + p, s)
print(f"wallpaper added to {count} pages, {len(OVERLAYS)} overlays left transparent")

# ---------------------------------------------------------------- splash
sp = rd("splash.xml")
sp = re.sub(r'<image name="splash_logo"[^>]*/>', '<image name="splash_logo" filename="Noxis/logo" retainaspect="1"/>', sp)
sp = re.sub(r'<image name="splash_bg"[^>]*/>', '<image name="splash_bg" filename="Noxis/wallpaper"/>', sp)
sp = sp.replace('<placement x="540" y="%center_y%" placement="4"/>\n\t\t\t</image>\n\n\t\t\t<image>\n\t\t\t\t<image resource="splash_logo"/>',
                '<placement x="540" y="%center_y%" placement="4"/>\n\t\t\t</image>\n\n\t\t\t<image>\n\t\t\t\t<image resource="splash_logo"/>')
# full-screen wallpaper instead of a centred picture
sp = sp.replace('<image resource="splash_bg"/>\n\t\t\t\t<placement x="540" y="%center_y%" placement="4"/>',
                '<image resource="splash_bg"/>\n\t\t\t\t<placement x="0" y="0"/>')
sp = sp.replace("<text>OrangeFox</text>", "<text>Noxis</text>")
sp = sp.replace('<text>Recovery</text>', '<text>Recovery · made by 3rr0r303YT</text>')
wr("splash.xml", sp)

# ---------------------------------------------------------------- name in the UI (English + German)
for lang in ("en.xml", "de_DE.xml"):
    p = "languages/" + lang
    if not os.path.exists(os.path.join(TW, p)): continue
    t = rd(p)
    t = re.sub(r'(<string name="twrp_watch_header">)OrangeFox', r'\1Noxis Recovery', t)
    wr(p, t)

# ---------------------------------------------------------------- sanity: every XML must still parse
bad = []
for dp, _, fs in os.walk(TW):
    for n in fs:
        if n.endswith(".xml") and os.path.join(dp, n) in PRE_OK:
            try: ET.parse(os.path.join(dp, n))
            except ET.ParseError as e: bad.append(f"{os.path.relpath(os.path.join(dp, n), TW)}: {e}")
if bad:
    print("XML errors:\n" + "\n".join(bad)); sys.exit(1)
print("Noxis theme applied")
