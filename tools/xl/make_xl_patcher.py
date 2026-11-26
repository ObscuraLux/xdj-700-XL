#!/usr/bin/env python3
# Copyright (C) 2026 Mike Wolf, Obscura Lux (XDJ-700 XL)
# Based on tools/make_patcher.py of XDJ-700 PLUS, Copyright (C) 2026 low trust society
# SPDX-License-Identifier: GPL-3.0-only
"""Write the XDJ-700 XL patcher: patcher/xdj700_xl_patcher.html.

One offline web page. The user drops their own official XDJ-700 firmware 1.15 UPD file on it; the page
rebuilds XDJ-700 XL from it on their computer (our bytes at the hook places, our code after the
application's end), checks the result byte for byte against the tested build, and saves a zip with
XDJ-700 XL, the restore file and the steps. It holds nothing of Pioneer's.

It uses the XDJ-700 PLUS patching engine (tools/xl/web_patcher_core.js, a copy with XL's wording) and
the same data format as tools/make_patcher.py. Generating needs what the XL build needs (the official
file in firmware/original and both cross compilers).

    python3 tools/xl/make_xl_patcher.py
    node tools/xl/web_patcher_check.js build DATA.json firmware/original/XDJ700.UPD out.UPD restore.UPD
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import make_patcher as mp  # noqa: E402
from build_flac_firmware import build, build_options  # noqa: E402
from build_stock_restore import build_restore  # noqa: E402
from check_lzss_roundtrip import stock_application  # noqa: E402

import base64  # noqa: E402
import tempfile  # noqa: E402
import zlib  # noqa: E402

NAME = "XDJ-700 XL V1"
APPROVED = "2026-11-26"
MOD_TAG = "1.17"
RESTORE_TAG = "1.18"
# PLUS's hooks (118 bytes) plus XL's: 10 address swaps and two 10-byte jumps (60 bytes).
MOST_CHANGED_BYTES = 200
OUTPUT = ROOT / "patcher" / "xdj700_xl_patcher.html"

KNOWN_ISSUES = [
    "The 3-band waveform needs tracks analysed and exported with rekordbox 6 or 7 (they come with an "
    ".2EX file). Other tracks show the stock blue waveform, as before.",
    "WAVE COLOR (UTILITY, at the end of the list) is 3BAND again every time the player is switched on.",
    "The small whole-track overview at the bottom of the screen stays blue.",
    "From XDJ-700 PLUS: 88.2 and 96 kHz files are played at 44.1 and 48 kHz. ALAC, 32-bit, 176.4 kHz and "
    "192 kHz files are not supported.",
    "From XDJ-700 PLUS: right after loading a FLAC or hi-res file the screen can be slower for a few "
    "moments. The sound is not affected.",
    "The update check, other players, rekordbox and the error log still see version 1.15, for "
    "compatibility.",
]

NOTICE = """\
==============================================================================
  XDJ-700 XL, UNOFFICIAL AND EXPERIMENTAL

  FLASHING FIRMWARE IS AT YOUR OWN RISK.
  It can leave a player unusable. There is no warranty of any kind, and no
  one who made or shared it is responsible for any damage or loss.

  - This is not a Pioneer DJ or AlphaTheta product. They have nothing to do
    with it, do not support it, and may refuse service on a modified player.
  - XDJ-700 XL is by Obscura Lux, designed and built by Mike Wolf. Its FLAC
    playback and other player fixes come from XDJ-700 PLUS by low trust
    society. It is free software under the GNU General Public License
    version 3, provided as is, with no warranty of any kind.
  - You use it entirely at your own risk. In no case is anyone who made or
    shared it responsible for any damage or loss, to your player, your
    media, your gig or anything else.
  - The 3-band waveform has been tested on the author's XDJ-700. That is not
    proof that it works the same on yours. Treat it as experimental, and do
    not rely on it for a show before you have tried it yourself at home.
  - A failed or interrupted update can leave a player unusable, as with any
    firmware update. Never switch the player off during an update.
  - The boot and recovery region and the front panel firmware stay as
    Pioneer made them, so the player's own update can always put the
    official firmware back.
  - Keep the restore file that this page makes, in case you want to go back
    to stock.
  - Only for the XDJ-700, and only from your own official XDJ-700 firmware
    version 1.15 UPD file.
"""

HOW_TO = """\
{title}

1_xdj700_xl/XDJ700.UPD      XDJ-700 XL
2_stock_restore/XDJ700.UPD  the restore file (the official firmware 1.15)

You need a FAT32 USB stick with about 18 MB free.

To install:
1. Copy 1_xdj700_xl/XDJ700.UPD to the top folder of a FAT32 USB stick.
2. Eject the stick properly, plug it back in, and check the file on the
   stick with the page ("steps and file check").
3. With the player switched off, put the stick in. Hold IN and RELOOP/EXIT,
   switch the player on, and keep holding until the update screen appears.
4. Wait until it has finished and the player has restarted. Do not switch
   it off before that. The start-up screen then shows XDJ-700 XL.

To go back to stock: the same steps with 2_stock_restore/XDJ700.UPD. Keep
that file. Both files have the same name, so keep one at a time on the
stick. The update screen shows {mod_tag} for XDJ-700 XL and {restore_tag} for the
restore file: those numbers are only there so that the player accepts the
file.

XDJ-700 XL's SHA-256: {mod_sha256}
The restore file's SHA-256: {restore_sha256}

"""


def patch_data(firmware: Path, header: Path, tool_prefix: Path, c6x_prefix: Path | None) -> dict:
    stock = firmware.read_bytes()
    options = build_options(True)
    options.update(version_label=NAME, wave3band=True)
    with tempfile.TemporaryDirectory(prefix="xdj700-xl-patcher-") as folder:
        built = Path(folder) / "xl.UPD"
        build(firmware, header, tool_prefix, built, version=MOD_TAG, c6x_prefix=c6x_prefix, **options)
        mod = built.read_bytes()
    _, stock_application_bytes, _ = stock_application(stock)
    mod_application = mp.unpacked_application(mod)
    if len(mod_application) <= len(stock_application_bytes):
        raise ValueError("the built application is not longer than the stock one")
    runs = mp.changes(stock_application_bytes, mod_application)
    if sum(len(run[1]) // 2 for run in runs) > MOST_CHANGED_BYTES:
        raise ValueError("more of the stock application changed than XL's hooks account for")
    return {
        "build": NAME,
        "date": APPROVED,
        "known_issues": KNOWN_ISSUES,
        "mod_tag": MOD_TAG,
        "restore_tag": RESTORE_TAG,
        "stock_sha256": mp.sha256(stock),
        "stock_application_sha256": mp.sha256(stock_application_bytes),
        "mod_application_sha256": mp.sha256(mod_application),
        "mod_sha256": mp.sha256(mod),
        "restore_sha256": mp.sha256(build_restore(stock, RESTORE_TAG)),
        "changes": runs,
        "tail": base64.b64encode(zlib.compress(mod_application[len(stock_application_bytes):], 9)).decode("ascii"),
    }


def render_page(data: dict) -> str:
    """The PLUS page renderer, with XL's template, engine, notice, steps and title."""
    mp.PAGE_TEMPLATE = HERE / "web_patcher_template.html"
    mp.PAGE_CORE = HERE / "web_patcher_core.js"
    mp.HOW_TO = HOW_TO
    mp.template_notice = lambda: NOTICE
    mp.zip_title = lambda d: d["build"] + (" (" + d["date"] + ")" if d.get("date") else "") + ". Obscura Lux. Designed and built by Mike Wolf"
    return mp.render_page(data)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--firmware", type=Path, default=ROOT / "firmware" / "original" / "XDJ700.UPD")
    parser.add_argument("--header", type=Path, default=ROOT / "src" / "dr_flac.h")
    parser.add_argument("--tool-prefix", type=Path, default=ROOT / "toolchains" / "installed" / "bin" / "sh-elf-")
    parser.add_argument("--c6x-prefix", type=Path, default=None)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--data-output", type=Path, default=None, help="also write the page's data as JSON")
    args = parser.parse_args()
    data = patch_data(args.firmware, args.header, args.tool_prefix, args.c6x_prefix)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(render_page(data))
    if args.data_output:
        args.data_output.write_text(json.dumps(data))
    print(json.dumps({
        "page_file": str(args.output),
        "bytes": args.output.stat().st_size,
        "build": data["build"],
        "mod_sha256": data["mod_sha256"],
        "restore_sha256": data["restore_sha256"],
        "changed_runs": len(data["changes"]),
        "changed_bytes": sum(len(run[1]) // 2 for run in data["changes"]),
    }, indent=2))


if __name__ == "__main__":
    main()
