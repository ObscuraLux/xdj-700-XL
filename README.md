# XDJ-700 XL

Unofficial firmware for the Pioneer DJ XDJ-700: **3-band scrolling waveforms** like the CDJ-3000X, plus **FLAC** and **hi-res WAV / AIFF** playback. Based on the official firmware 1.15.

**Obscura Lux.** Designed and built by **Mike Wolf** in Berlin. FLAC playback and the other player fixes come from [XDJ-700 PLUS](https://github.com/lowtrustsociety/XDJ-700-PLUS) by low trust society, which XDJ-700 XL includes in full.

> **Unofficial and experimental. FLASHING FIRMWARE IS AT YOUR OWN RISK.**
> It can leave a player unusable. There is no warranty of any kind, and no one who made or shared it is responsible for any damage or loss.
> This is not a Pioneer DJ or AlphaTheta product.

**Download: [latest release](https://github.com/ObscuraLux/xdj-700-XL/releases/latest)** (the patcher page and the step-by-step guide with pictures, [also here](docs/XDJ-700-XL-Guide.pdf))

## What it adds

- **3-band scrolling waveform**, as rekordbox and the CDJ-3000X draw it: bass in blue, mids in amber, bass and mids together in brown, highs in cream. It uses rekordbox's own 3-band analysis (the `.2EX` files that rekordbox 6 and 7 export). Tracks without it show the classic blue waveform.
- **WAVE COLOR** in UTILITY (at the end of the list): **3BAND** or the classic **BLUE**. It is 3BAND again every time the player is switched on.
- Everything in **XDJ-700 PLUS V0.90**: FLAC (16/24-bit, up to 96 kHz, on the player's audio processor), hi-res WAV and AIFF, Bandcamp WAVs, mono files, the front panel guard, and more. See [docs/XDJ-700-PLUS.md](docs/XDJ-700-PLUS.md).

## Good to know

- The small whole-track overview at the bottom of the screen stays blue.
- 88.2 and 96 kHz files are played at 44.1 and 48 kHz. ALAC, 32-bit, 176.4 kHz and 192 kHz files are not supported.
- Right after loading a FLAC or hi-res file, the screen can be slower for a few moments. The sound is not affected.
- The update check, other players, rekordbox and the error log still see version 1.15, for compatibility.
- The 3-band waveform has been tested on the author's XDJ-700. Try XL at home before you rely on it at a gig.

## Install

This repository holds no Pioneer firmware. You bring your own official XDJ-700 firmware version 1.15 and a FAT32 USB stick with about 18 MB free.

1. Download `xdj700_xl_patcher.html` (and the PDF guide) from the [latest release](https://github.com/ObscuraLux/xdj-700-XL/releases/latest) and open it in a current browser (double-click it). It works offline: your firmware file never leaves your computer.
2. Drop the official firmware 1.15 file (`XDJ700.UPD`, or the zip it comes in) onto the page, press **build XDJ-700 XL**, read and accept the notice. The page saves a zip with:
   - `1_xdj700_xl/XDJ700.UPD`: XDJ-700 XL
   - `2_stock_restore/XDJ700.UPD`: the restore file (the official firmware 1.15), to go back
3. Copy `1_xdj700_xl/XDJ700.UPD` to the top folder of the USB stick. Eject it, plug it back in, and check the file on the stick with the page (**steps and file check**).
4. With the player switched off, put the stick in. Hold **IN** and **RELOOP/EXIT**, switch the player on, and keep holding until the update screen appears (it shows 1.15 → 1.17).
5. Wait until the update has finished and the player has restarted by itself. Do not switch it off before that. The start-up screen then shows **XDJ-700 XL V1**.

To go back to stock, do the same with `2_stock_restore/XDJ700.UPD` (the update screen shows 1.15 → 1.18). The boot and recovery region and the front panel firmware are never changed.

The page refuses any file that is not the official XDJ-700 firmware 1.15, and only hands out XDJ-700 XL if the result is, byte for byte, the build tested on the player:

| file | SHA-256 |
| :--- | :--- |
| XDJ-700 XL V1 | `e5135de546176275a0a559d7efdc184f39fecaf6015f9c107c10039d3d05de9a` |
| restore file (official 1.15, tagged 1.18) | `00fe053245e6b0ebbaf75ebdd815af00f325b2a5dfc78e55e808d13118ccd96d` |
| official firmware 1.15 (input) | `73edec9802da51672257c2599efc04209dc92478fcbaa1a0425b3b122e33f99c` |

## How the 3-band waveform works

The XDJ-700 asks its own built-in Pro DJ Link database server for a track's scrolling waveform (the blue `PWV3` data from the `.EXT` analysis file), caches it, and draws it column by column into an off-screen picture that the screen mirrors around the centre line. XDJ-700 XL adds three things (`src/wave3band.c`, `src/wave3band_glue.S`):

1. **Loading.** The server's waveform getter is wrapped. After it, XL opens the track's `ANLZ0000.2EX` (same folder, found from the track path with the player's own hash), reads the 3-band `PWV7` data (three bytes per column: low, mid, high, the same number of columns as `PWV3`) and keeps it, tagged with the size and checksum of the blue data.
2. **Matching.** When the screen unpacks a track's blue waveform, XL finds the 3-band data with the same tag (the screen's copy carries a 20-byte header, which is allowed for).
3. **Drawing.** At the end of each column of both waveform drawers, XL repaints the column: the stock background from the beat grid, then the bands with rekordbox's rule and colours (the largest value of each band over the entries a column covers, scaled as rekordbox scales them).

The UTILITY item is a 17th entry in a copy of the stock UTILITY table, which reuses the stock menu routines. Every change to the stock application is an address swapped in a literal pool or one of two 10-byte jumps, each checked against the stock bytes (by hash) before it is made. Compared with XDJ-700 PLUS V0.90, XL changes 9 places in the stock application; everything else is byte for byte PLUS.

## Build

You need your own official XDJ-700 firmware version 1.15 as `firmware/original/XDJ700.UPD` (see `firmware/README.md`), the two cross compilers (`toolchains/README.md`), `python3`, and for the page check Node.

```sh
python3 tools/build_flac_firmware.py --output build/xl/XDJ700.UPD --wave3band --version 1.17 --version-label "XDJ-700 XL V1"
python3 tools/xl/make_xl_patcher.py --data-output build/xl_data.json      # patcher/xdj700_xl_patcher.html
node tools/xl/web_patcher_check.js build build/xl_data.json firmware/original/XDJ700.UPD build/page.UPD build/restore.UPD
```

The build is deterministic: it gives the SHA-256 above, and the page's engine, run on the official file, gives the same file byte for byte. `--wave3band-diag` adds four WAVE INFO rows to UTILITY for debugging. Without `--wave3band`, the build is XDJ-700 PLUS V0.90 as published (SHA-256 `84f7f0653a4c66cf5b902edf1d837989e443ab5ab3462aa0ffed8b6530076f56`). How PLUS works: [docs/HOW_IT_WORKS.md](docs/HOW_IT_WORKS.md).

## Credits and licence

- **XDJ-700 XL** (3-band waveform, WAVE COLOR, the XL patcher page, the guide): Obscura Lux, designed and built by Mike Wolf.
- **XDJ-700 PLUS** (FLAC and hi-res playback, the other player fixes, the build tools and the patching engine the XL page is built on): low trust society, [github.com/lowtrustsociety/XDJ-700-PLUS](https://github.com/lowtrustsociety/XDJ-700-PLUS).
- **dr_flac** by David Reid (public domain or MIT-0).

Free software under the GNU General Public License version 3 (`LICENSE`), provided as is, with no warranty of any kind.

Pioneer DJ, XDJ-700, CDJ-3000X and rekordbox are trademarks of their owners, used here only to say what this works with. This repository contains no Pioneer firmware or code.
