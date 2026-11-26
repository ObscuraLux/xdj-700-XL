/* 3-band waveform test build: the UTILITY items (step 1).
 *
 * Not part of XDJ-700 PLUS. Private test code, kept out of git until tested.
 *
 * The UTILITY list (stock RAM table 0x0E736410, 16 entries of 0x34 bytes, count
 * at 0x0E736750) is reached by the list code only through four literal copies
 * of each address. The build points them at xdj_wave_items / xdj_wave_item_count
 * below and copies the 16 stock entries into the first 16 slots of
 * xdj_wave_items (from the user's own firmware, at build time). Two test items
 * follows:
 *   WAVE COLOR  : BLUE / 3BAND     (xdj_wave_color, default 3BAND)
 * Not saved: it is 3BAND again at power-on. Built with XDJ_WAVE_DIAG, WAVE INFO 1-4 follow
 * (diagnostics: loads, errors, matches, paints, sizes).
 *
 * Each item reuses Pioneer's generic routines (select, turn, greyed, cancel) and
 * SLIP FLASHING's row filler, which draws a two-value item; only the value
 * source and the commit are ours. Pioneer's code calls ours through the small
 * wrappers in wave3band_glue.S, which keep MACH/MACL as the stock compiler
 * expects.
 */
#include <stdint.h>

typedef int (*xdj_item_fn)(void *entry, void *rows);

struct xdj_utility_item {
    const uint16_t *label;
    const uint16_t *const *values;
    uint32_t count;
    int32_t current;
    int32_t pending;
    uint32_t enabled;
    const void *init;
    const void *fill;
    const void *select;
    const void *turn;
    const void *commit;
    const void *greyed;
    const void *cancel;
};

#ifndef XDJ_HOST_TEST
_Static_assert(sizeof(struct xdj_utility_item) == 0x34, "UTILITY entry is 0x34 bytes");
#endif

/* Stock routines (XDJ-700 1.15, found by reading the firmware). */
#define STOCK_ITEM_INIT_NOTHING ((const void *)0x08DD0C6A) /* rts */
#define STOCK_ITEM_SELECT       ((const void *)0x08DD198C)
#define STOCK_ITEM_TURN         ((const void *)0x08DD1AD0)
#define STOCK_ITEM_GREYED       ((const void *)0x08DD20A0)
#define STOCK_ITEM_CANCEL       ((const void *)0x08DD20E2)
#define STOCK_SLIP_FLASHING_FILL ((xdj_item_fn)0x08DD0EE4)

#define XDJ_STOCK_ITEMS 16
#ifdef XDJ_WAVE_DIAG
#define XDJ_OUR_ITEMS 5 /* WAVE COLOR + WAVE INFO 1-4 */
#else
#define XDJ_OUR_ITEMS 1 /* WAVE COLOR */
#endif

/* The values the drawing code reads. */
volatile uint32_t xdj_wave_color = 1;  /* 0 BLUE, 1 3BAND */

static const uint16_t label_color[] = u"WAVE COLOR";
static const uint16_t value_blue[] = u"BLUE";
static const uint16_t value_3band[] = u"3BAND";
static const uint16_t *const values_color[] = {value_blue, value_3band};
#ifdef XDJ_WAVE_DIAG
/* Diagnostics rows (test builds): one value each, rebuilt every time the row is drawn. */
static const uint16_t label_info1[] = u"WAVE INFO 1";
static const uint16_t label_info2[] = u"WAVE INFO 2";
static const uint16_t label_info3[] = u"WAVE INFO 3";
static const uint16_t label_info4[] = u"WAVE INFO 4";
static uint16_t info1_text[32];
static uint16_t info2_text[32];
static uint16_t info3_text[32];
static uint16_t info4_text[32];
static const uint16_t *const values_info1[] = {info1_text};
static const uint16_t *const values_info2[] = {info2_text};
static const uint16_t *const values_info3[] = {info3_text};
static const uint16_t *const values_info4[] = {info4_text};
#endif

/* In glue (wrappers that call the C functions below). */
extern const char xdj_wave_fill_color_entry[];
extern const char xdj_wave_commit_color_entry[];
#ifdef XDJ_WAVE_DIAG
extern const char xdj_wave_fill_info1_entry[];
extern const char xdj_wave_fill_info2_entry[];
extern const char xdj_wave_fill_info3_entry[];
extern const char xdj_wave_fill_info4_entry[];
extern const char xdj_wave_commit_none_entry[];
#endif

/* The whole list: 16 stock entries (filled in by the build) and ours. */
struct xdj_utility_item xdj_wave_items[XDJ_STOCK_ITEMS + XDJ_OUR_ITEMS] __attribute__((aligned(4))) = {
    [XDJ_STOCK_ITEMS] = {
        label_color, values_color, 2, -1, -1, 1,
        STOCK_ITEM_INIT_NOTHING, xdj_wave_fill_color_entry, STOCK_ITEM_SELECT, STOCK_ITEM_TURN,
        xdj_wave_commit_color_entry, STOCK_ITEM_GREYED, STOCK_ITEM_CANCEL,
    },
#ifdef XDJ_WAVE_DIAG
    [XDJ_STOCK_ITEMS + 1] = {
        label_info1, values_info1, 1, -1, -1, 1,
        STOCK_ITEM_INIT_NOTHING, xdj_wave_fill_info1_entry, STOCK_ITEM_SELECT, STOCK_ITEM_TURN,
        xdj_wave_commit_none_entry, STOCK_ITEM_GREYED, STOCK_ITEM_CANCEL,
    },
    [XDJ_STOCK_ITEMS + 2] = {
        label_info2, values_info2, 1, -1, -1, 1,
        STOCK_ITEM_INIT_NOTHING, xdj_wave_fill_info2_entry, STOCK_ITEM_SELECT, STOCK_ITEM_TURN,
        xdj_wave_commit_none_entry, STOCK_ITEM_GREYED, STOCK_ITEM_CANCEL,
    },
    [XDJ_STOCK_ITEMS + 3] = {
        label_info3, values_info3, 1, -1, -1, 1,
        STOCK_ITEM_INIT_NOTHING, xdj_wave_fill_info3_entry, STOCK_ITEM_SELECT, STOCK_ITEM_TURN,
        xdj_wave_commit_none_entry, STOCK_ITEM_GREYED, STOCK_ITEM_CANCEL,
    },
    [XDJ_STOCK_ITEMS + 4] = {
        label_info4, values_info4, 1, -1, -1, 1,
        STOCK_ITEM_INIT_NOTHING, xdj_wave_fill_info4_entry, STOCK_ITEM_SELECT, STOCK_ITEM_TURN,
        xdj_wave_commit_none_entry, STOCK_ITEM_GREYED, STOCK_ITEM_CANCEL,
    },
#endif
};

uint32_t xdj_wave_item_count = XDJ_STOCK_ITEMS + XDJ_OUR_ITEMS;

/* SLIP FLASHING's filler shows values[pending] when an edit is pending, and
 * otherwise asks the SLIP FLASHING setting. Give it our value as the pending one
 * for the duration of the call. */
static int fill_with(struct xdj_utility_item *entry, void *rows, uint32_t value)
{
    int32_t pending = entry->pending;
    if (pending == -1) {
        entry->current = (int32_t)value;
        entry->pending = (int32_t)value;
    }
    int result = STOCK_SLIP_FLASHING_FILL(entry, rows);
    entry->pending = pending;
    return result;
}

/* As the stock commit (0x08DD1BF4): take the pending value if it is a new, valid one. */
static int commit_to(struct xdj_utility_item *entry, volatile uint32_t *setting)
{
    uint32_t pending = (uint32_t)entry->pending;
    if ((pending & 0xFFFFu) < (entry->count & 0xFFFFu) && pending != (uint32_t)entry->current) {
        entry->current = (int32_t)pending;
        *setting = pending;
        return 1;
    }
    return 0;
}

int xdj_wave_fill_color(struct xdj_utility_item *entry, void *rows) { return fill_with(entry, rows, xdj_wave_color); }
int xdj_wave_commit_color(struct xdj_utility_item *entry) { return commit_to(entry, &xdj_wave_color); }

#ifdef XDJ_WAVE_DIAG
/* ---- diagnostics rows ---- */
extern volatile uint32_t xdj_wave_loads_ok, xdj_wave_matches, xdj_wave_misses, xdj_wave_getter_calls,
    xdj_wave_unpack_ok, xdj_wave_paint_calls, xdj_wave_paint_active, xdj_wave_last_error,
    xdj_wave_last_getter_bytes, xdj_wave_last_unpack_bytes, xdj_wave_last_getter_sum, xdj_wave_last_unpack_sum;

static uint16_t digit(uint32_t n) { return (uint16_t)(n > 9 ? '+' : '0' + n); }

static uint16_t *pair(uint16_t *at, char tag, uint32_t n)
{
    *at++ = (uint16_t)tag;
    *at++ = digit(n);
    *at++ = ' ';
    return at;
}

static uint16_t *hex5(uint16_t *at, uint32_t v)
{
    for (int shift = 16; shift >= 0; shift -= 4) {
        uint32_t nibble = (v >> shift) & 0xFu;
        *at++ = (uint16_t)(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
    return at;
}

extern volatile uint32_t xdj_wave_unpack_calls, xdj_wave_paint2_calls, xdj_wave_last_getter_head,
    xdj_wave_last_unpack_head;

static uint16_t *hex8(uint16_t *at, uint32_t v)
{
    for (int shift = 28; shift >= 0; shift -= 4) {
        uint32_t nibble = (v >> shift) & 0xFu;
        *at++ = (uint16_t)(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
    return at;
}

int xdj_wave_fill_info1(struct xdj_utility_item *entry, void *rows)
{
    /* G getter calls, O 3-band loads, E last error, M matched on screen, X not matched */
    uint16_t *at = info1_text;
    at = pair(at, 'G', xdj_wave_getter_calls);
    at = pair(at, 'O', xdj_wave_loads_ok);
    at = pair(at, 'E', xdj_wave_last_error);
    at = pair(at, 'M', xdj_wave_matches);
    at = pair(at, 'X', xdj_wave_misses);
    at[-1] = 0;
    return fill_with(entry, rows, 0);
}

int xdj_wave_fill_info2(struct xdj_utility_item *entry, void *rows)
{
    /* U screen unpacks, P drawer 1 columns, Q drawer 2 columns, A columns painted in 3-band */
    uint16_t *at = info2_text;
    at = pair(at, 'U', xdj_wave_unpack_ok);
    at = pair(at, 'P', xdj_wave_paint_calls);
    at = pair(at, 'Q', xdj_wave_paint2_calls);
    at = pair(at, 'A', xdj_wave_paint_active);
    at[-1] = 0;
    return fill_with(entry, rows, 0);
}

int xdj_wave_fill_info3(struct xdj_utility_item *entry, void *rows)
{
    /* PWV3 bytes: loader = or ! screen (checksums equal or not) */
    uint16_t *at = info3_text;
    at = hex5(at, xdj_wave_last_getter_bytes);
    *at++ = (uint16_t)(xdj_wave_last_getter_sum == xdj_wave_last_unpack_sum ? '=' : '!');
    at = hex5(at, xdj_wave_last_unpack_bytes);
    *at = 0;
    return fill_with(entry, rows, 0);
}

int xdj_wave_fill_info4(struct xdj_utility_item *entry, void *rows)
{
    /* the first 4 PWV3 bytes: loader / screen */
    uint16_t *at = info4_text;
    at = hex8(at, xdj_wave_last_getter_head);
    *at++ = '/';
    at = hex8(at, xdj_wave_last_unpack_head);
    *at = 0;
    return fill_with(entry, rows, 0);
}

int xdj_wave_commit_none(struct xdj_utility_item *entry)
{
    (void)entry;
    return 0;
}
#endif

/* ------------------------------------------------------------------------
 * Step 2: loading PWV7 (3-band detail) from the track's ANLZ0000.2EX.
 *
 * The player fetches the blue scrolling waveform (PWV3) from its own database
 * server; the server's getter 0x08D01524 (record, track path, out) reads it
 * from the .EXT file. We wrap that getter: after it, we open
 * /PIONEER/USBANLZ/Pxxx/xxxxxxxx/ANLZ0000.2EX on the same drive (folder from
 * the stock hash of the track path, 0x08D08878), read PWV7 into a slot, and tag
 * the slot with the PWV3's size and checksum.
 *
 * The screen task's unpacker 0x08DB4B5E (wrapped) then finds the slot whose tag
 * matches the PWV3 it is showing and takes it over. Hand-over rules: the loader
 * only ever claims FREE slots; only the screen side frees memory and returns
 * slots to FREE. Every state change is one aligned word write.
 */

#ifndef XDJ_HOST_TEST
#define STOCK_PWV3_GETTER ((int (*)(void *, const uint16_t *, uint32_t *))0x08D01524)
#define STOCK_PWV3_UNPACK ((int (*)(void))0x08DB4B5E)
#define STOCK_PATH_HASH   ((uint32_t (*)(const uint16_t *))0x08D08878)
#define STOCK_OPEN  ((void *(*)(const uint16_t *, int, const uint16_t *, const char *, const void *))0x08C544A4)
#define STOCK_SEEK  ((int (*)(void *, uint32_t, int, const void *))0x08C545AE)
#define STOCK_READ  ((int (*)(void *, uint32_t, void *, const void *))0x08C54706)
#define STOCK_CLOSE ((int (*)(void *))0x08C54526)
#define STOCK_ALLOC ((uint8_t *(*)(uint32_t))0x08C54D44) /* zero-filled */
#define STOCK_FREE  ((void (*)(void *))0x08C52942)
#define STOCK_RGB565 ((uint32_t (*)(int, int, int))0x08DB6CD4)
/* What the stock PWV3 getter passes as the file calls' last argument. */
#define STOCK_FILE_OPTIONS ((const void *)0xA80B85CC)

/* The screen side (stock RAM). */
#define SHOWN_RECORD      0x0C0CE408u /* +0xC: PWV3 block (+0x10 -> bytes), +0x10: byte count */
#define DISPLAY_STATE     0x0BA3C050u /* +0x2EE10: per-entry beat flags (8 bytes each) */
#define CANVAS            0x0C09408Cu /* RGB565, 504 px per row */
#endif
#define CANVAS_ROW_BYTES  (504u * 2u)
#define CANVAS_BOTTOM_ROW 65u
#define WAVE_ROWS         50u

#define SLOTS 4
enum { SLOT_FREE = 0, SLOT_LOADING = 1, SLOT_READY = 2, SLOT_SHOWN = 3 };
struct wave_slot {
    volatile uint32_t state;
    uint32_t pwv3_bytes;
    uint32_t pwv3_sum;
    uint32_t columns;
    uint8_t *bands; /* 3 bytes per column: low, mid, high */
    uint32_t sequence; /* load order */
};
static uint32_t load_sequence;
static struct wave_slot slots[SLOTS];
static struct wave_slot *volatile shown_slot;
static const uint8_t *last_unpacked_data;
static uint32_t last_unpacked_bytes;
static uint32_t last_unpacked_sample;

/* For checking on the player (VERSION No. stays our label; these are for a later debug view). */
volatile uint32_t xdj_wave_loads_ok, xdj_wave_loads_missing, xdj_wave_matches, xdj_wave_misses;
/* Diagnostics shown in UTILITY (WAVE INFO 1 / 2) by the test builds. */
volatile uint32_t xdj_wave_getter_calls, xdj_wave_unpack_calls, xdj_wave_unpack_ok, xdj_wave_paint_calls,
    xdj_wave_paint_active, xdj_wave_last_error, xdj_wave_last_getter_bytes, xdj_wave_last_unpack_bytes,
    xdj_wave_last_getter_sum, xdj_wave_last_unpack_sum, xdj_wave_paint2_calls, xdj_wave_last_getter_head,
    xdj_wave_last_unpack_head;
enum { ERR_NONE = 0, ERR_NO_SLOT = 1, ERR_OPEN = 2, ERR_HEADER = 3, ERR_NO_PWV7 = 4, ERR_PWV7 = 5, ERR_ALLOC = 6,
       ERR_READ = 7, ERR_NO_PATH = 8 };

static uint32_t checksum(const uint8_t *data, uint32_t bytes)
{
    uint32_t sum = 0x811C9DC5u;
    for (uint32_t i = 0; i < bytes; i++) {
        sum = (sum ^ data[i]) * 0x01000193u;
    }
    return sum;
}

/* A quick look at 256 bytes spread over the data, for "is it still the same track". */
static uint32_t sample(const uint8_t *data, uint32_t bytes)
{
    uint32_t sum = bytes;
    uint32_t stride = bytes / 256u + 1u;
    for (uint32_t i = 0; i < bytes; i += stride) {
        sum = sum * 31u + data[i];
    }
    return sum;
}

static uint32_t big32(const uint8_t *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}

static uint16_t *put(uint16_t *at, const char *text)
{
    while (*text) {
        *at++ = (uint16_t)(uint8_t)*text++;
    }
    return at;
}

static uint16_t *put_hex(uint16_t *at, uint32_t value, int digits)
{
    for (int shift = (digits - 1) * 4; shift >= 0; shift -= 4) {
        uint32_t nibble = (value >> shift) & 0xFu;
        *at++ = (uint16_t)(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
    return at;
}

/* The rekordbox folder: P%03X from seven bits of the hash, then %08X of it (as 0x08D09CF6 / 0x08D09EAC). */
static void anlz_2ex_path(uint16_t *path, uint32_t hash, int hidden_folder)
{
    uint32_t p = (hash & 1u) | (hash >> 2 & 1u) << 1 | (hash >> 6 & 1u) << 2 | (hash >> 7 & 1u) << 3 |
                 (hash >> 9 & 1u) << 4 | (hash >> 13 & 1u) << 5 | (hash >> 16 & 1u) << 6;
    uint16_t *at = put(path, hidden_folder ? "/.PIONEER/USBANLZ/P" : "/PIONEER/USBANLZ/P");
    at = put_hex(at, p, 3);
    at = put(at, "/");
    at = put_hex(at, hash, 8);
    at = put(at, "/ANLZ0000.2EX");
    *at = 0;
}

/* Reads the PWV7 body of an open .2EX into a new buffer. Returns its columns, 0 if none. */
static uint32_t read_pwv7(void *file, uint8_t **bands)
{
    uint8_t head[24];
    if (STOCK_SEEK(file, 0, 0, STOCK_FILE_OPTIONS) != 0 || STOCK_READ(head, 12, file, STOCK_FILE_OPTIONS) != 12) {
        xdj_wave_last_error = ERR_HEADER;
        return 0;
    }
    if (head[0] != 'P' || head[1] != 'M' || head[2] != 'A' || head[3] != 'I') {
        xdj_wave_last_error = ERR_HEADER;
        return 0;
    }
    uint32_t position = big32(head + 4);
    uint32_t file_bytes = big32(head + 8);
    for (int sections = 0; sections < 32 && position + 24 <= file_bytes; sections++) {
        if (STOCK_SEEK(file, position, 0, STOCK_FILE_OPTIONS) != 0 ||
            STOCK_READ(head, 24, file, STOCK_FILE_OPTIONS) != 24) {
            xdj_wave_last_error = ERR_READ;
            return 0;
        }
        uint32_t header_bytes = big32(head + 4);
        uint32_t total_bytes = big32(head + 8);
        if (total_bytes < 12 || header_bytes < 12 || header_bytes > total_bytes || position + total_bytes > file_bytes) {
            xdj_wave_last_error = ERR_NO_PWV7;
            return 0;
        }
        if (head[0] == 'P' && head[1] == 'W' && head[2] == 'V' && head[3] == '7') {
            /* Header: entry bytes (3), entry count, then an unknown word. */
            uint32_t entry_bytes = big32(head + 12);
            uint32_t entries = big32(head + 16);
            uint32_t body = total_bytes - header_bytes;
            if (entry_bytes != 3 || entries == 0 || entries > 2000000u || entries * 3u > body) {
                xdj_wave_last_error = ERR_PWV7;
                return 0;
            }
            uint8_t *buffer = STOCK_ALLOC(entries * 3u);
            if (!buffer) {
                xdj_wave_last_error = ERR_ALLOC;
                return 0;
            }
            if (STOCK_SEEK(file, position + header_bytes, 0, STOCK_FILE_OPTIONS) != 0 ||
                STOCK_READ(buffer, entries * 3u, file, STOCK_FILE_OPTIONS) != (int)(entries * 3u)) {
                STOCK_FREE(buffer);
                xdj_wave_last_error = ERR_READ;
                return 0;
            }
            *bands = buffer;
            return entries;
        }
        position += total_bytes;
    }
    xdj_wave_last_error = ERR_NO_PWV7;
    return 0;
}

static void load_bands(void *record, const uint16_t *track_path, const uint32_t *out)
{
    uint32_t bytes = out[2];
    const uint8_t *pwv3 = (const uint8_t *)out[4];
    xdj_wave_last_getter_bytes = bytes;
    if (bytes == 0 || pwv3 == 0 || track_path == 0) {
        xdj_wave_last_error = ERR_NO_PATH;
        return;
    }
    uint32_t sum = checksum(pwv3, bytes);
    xdj_wave_last_getter_sum = sum;
    xdj_wave_last_getter_head = bytes >= 4 ? big32(pwv3) : 0;
    struct wave_slot *slot = 0;
    for (int i = 0; i < SLOTS; i++) {
        uint32_t state = slots[i].state;
        if ((state == SLOT_READY || state == SLOT_SHOWN) && slots[i].pwv3_bytes == bytes && slots[i].pwv3_sum == sum) {
            return; /* already here */
        }
    }
    for (int i = 0; i < SLOTS && !slot; i++) {
        if (slots[i].state == SLOT_FREE) {
            slot = &slots[i];
        }
    }
    if (!slot) {
        xdj_wave_last_error = ERR_NO_SLOT;
        return;
    }
    slot->state = SLOT_LOADING;
    uint16_t path[64];
    const uint8_t *r = (const uint8_t *)record;
    anlz_2ex_path(path, STOCK_PATH_HASH(track_path), *(const int16_t *)(r + 10) == 4);
    uint8_t *bands = 0;
    uint32_t columns = 0;
    void *file = STOCK_OPEN((const uint16_t *)(r + 8), 0, path, "r", 0);
    if (file) {
        columns = read_pwv7(file, &bands);
        STOCK_CLOSE(file);
    } else {
        xdj_wave_last_error = ERR_OPEN;
    }
    if (columns == 0) {
        xdj_wave_loads_missing++;
        slot->state = SLOT_FREE;
        return;
    }
    slot->pwv3_bytes = bytes;
    slot->pwv3_sum = sum;
    slot->columns = columns;
    slot->bands = bands;
    slot->sequence = ++load_sequence;
    xdj_wave_loads_ok++;
    xdj_wave_last_error = ERR_NONE;
    slot->state = SLOT_READY;
}

int xdj_wave_getter(void *record, const uint16_t *track_path, uint32_t *out)
{
    xdj_wave_getter_calls++;
    int result = STOCK_PWV3_GETTER(record, track_path, out);
    if (result == 0 && record && out) {
        load_bands(record, track_path, out);
    }
    return result;
}

static void release(struct wave_slot *slot)
{
    uint8_t *bands = slot->bands;
    slot->bands = 0;
    slot->columns = 0;
    if (bands) {
        STOCK_FREE(bands);
    }
    slot->state = SLOT_FREE;
}

/* After the stock unpacker: find the bands for the PWV3 now on screen. */
int xdj_wave_unpack(void)
{
    xdj_wave_unpack_calls++;
    int result = STOCK_PWV3_UNPACK();
    const uint8_t *shown = (const uint8_t *)SHOWN_RECORD;
    uint32_t bytes = *(const volatile uint32_t *)(shown + 0x10);
    const uint8_t *block = *(const uint8_t *const volatile *)(shown + 0xC);
    const uint8_t *pwv3 = block ? *(const uint8_t *const *)(block + 0x10) : 0;
    if (result != 1 || !pwv3 || bytes == 0) {
        return result;
    }
    xdj_wave_unpack_ok++;
    xdj_wave_last_unpack_bytes = bytes;
    xdj_wave_last_unpack_head = bytes >= 4 ? big32(pwv3) : 0;
    uint32_t quick = sample(pwv3, bytes);
    if (pwv3 == last_unpacked_data && bytes == last_unpacked_bytes && quick == last_unpacked_sample) {
        return result; /* the same track as last time */
    }
    last_unpacked_data = pwv3;
    last_unpacked_bytes = bytes;
    last_unpacked_sample = quick;
    /* The screen's size counts the client block's 0x14-byte header (seen on the player: always
     * getter bytes + 0x14); the data itself starts after it. */
    uint32_t data_bytes = bytes > 0x14u ? bytes - 0x14u : bytes;
    uint32_t sum = checksum(pwv3, data_bytes);
    xdj_wave_last_unpack_sum = sum;
    struct wave_slot *old = shown_slot;
    if (old && old->pwv3_bytes == data_bytes && old->pwv3_sum == sum) {
        return result;
    }
    struct wave_slot *found = 0;
    for (int i = 0; i < SLOTS; i++) {
        if (slots[i].state == SLOT_READY && slots[i].pwv3_bytes == data_bytes && slots[i].pwv3_sum == sum) {
            found = &slots[i];
            break;
        }
    }
    shown_slot = 0;
    if (old) {
        release(old);
    }
    /* Bands loaded for other tracks (a linked player's requests, an earlier load) are dropped. */
    for (int i = 0; i < SLOTS; i++) {
        if (&slots[i] != found && slots[i].state == SLOT_READY) {
            release(&slots[i]);
        }
    }
    if (found) {
        found->state = SLOT_SHOWN;
        shown_slot = found;
        xdj_wave_matches++;
    } else {
        xdj_wave_misses++;
    }
    return result;
}

/* ------------------------------------------------------------------------
 * Step 3: painting one column of the scrolling waveform.
 *
 * Called by the hook at the end of the stock column loop (0x08DC29A6), after the
 * stock code painted the column blue. With WAVE COLOR = 3BAND and bands for the
 * track, the column's 50 rows are painted again: background as the stock code
 * does it (from the entry's beat flags), then the bands with rekordbox's rule.
 * Scale as rekordbox's 3Band view: 158 = full height (127 reaches about 80%).
 */
#define RGB565(r, g, b) ((uint16_t)(((r) >> 3) << 11 | ((g) >> 2) << 5 | ((b) >> 3)))
#define COLOUR_LOW   RGB565(33, 82, 217)
#define COLOUR_MID   RGB565(242, 170, 60)
#define COLOUR_BOTH  RGB565(169, 106, 38)
#define COLOUR_HIGH  RGB565(243, 235, 217)
#define FULL_SCALE   158u

static uint32_t rows_for(uint32_t value, uint32_t rows)
{
    uint32_t r = (value * rows + FULL_SCALE / 2) / FULL_SCALE;
    return r > rows ? rows : r;
}

static void paint(int32_t column, int32_t entry, int32_t step, uint8_t *bottom, uint32_t wave_rows)
{
    struct wave_slot *slot = shown_slot;
    if (xdj_wave_color != 1 || !slot || column < 0 || column >= 504 || step <= 0 || !bottom) {
        return;
    }
    xdj_wave_paint_active++;
    const uint8_t *bands = slot->bands;
    uint32_t columns = slot->columns;
    if (!bands) {
        return;
    }
    /* The stock background for this entry. */
    const uint8_t *state = (const uint8_t *)DISPLAY_STATE;
    const uint16_t *flags_base = *(const uint16_t *const *)(state + 0x2EE10);
    uint16_t background = 0;
    if (flags_base && entry >= 0) {
        uint16_t flags = flags_base[(uint32_t)entry * 4u];
        if (flags & 8) {
            background = (uint16_t)STOCK_RGB565(-80, 97, 0);
        } else if (flags & 0x10) {
            background = (uint16_t)STOCK_RGB565(49, 48, 49);
        } else {
            background = (uint16_t)STOCK_RGB565(0, 0, 0);
        }
    }
    /* The bands: the largest of each over the entries this column covers (as the stock code). */
    uint32_t low = 0, mid = 0, high = 0;
    for (int32_t i = entry; i < entry + step; i++) {
        if (i >= 0 && (uint32_t)i < columns) {
            const uint8_t *b = bands + (uint32_t)i * 3u;
            if (b[0] > low) low = b[0];
            if (b[1] > mid) mid = b[1];
            if (b[2] > high) high = b[2];
        }
    }
    uint32_t reach = wave_rows; /* the screen mirrors the column: this is the centred look */
    uint32_t lo = rows_for(low, reach), mi = rows_for(mid, reach), hi = rows_for(high, reach);
    uint32_t inner = lo < mi ? lo : mi;
    uint32_t outer = lo < mi ? mi : lo;
    uint16_t outer_colour = lo > mi ? COLOUR_LOW : COLOUR_MID;
    uint8_t *pixel = bottom + (uint32_t)column * 2u;
    for (uint32_t row = 0; row < wave_rows; row++, pixel -= CANVAS_ROW_BYTES) {
        /* distance from the base line (the screen mirrors it: that is the centre line) */
        uint32_t d = row;
        uint16_t colour = background;
        if (d < hi) {
            colour = COLOUR_HIGH;
        } else if (d < inner) {
            colour = COLOUR_BOTH;
        } else if (d < outer) {
            colour = outer_colour;
        }
        *(volatile uint16_t *)pixel = colour;
    }
}

/* Drawer 1 (0x08DC2754): canvas 0x0C09408C, 50 rows up from row 65. */
void xdj_wave_paint_column(int32_t column, int32_t entry, int32_t step)
{
    xdj_wave_paint_calls++;
    paint(column, entry, step, (uint8_t *)CANVAS + CANVAS_BOTTOM_ROW * CANVAS_ROW_BYTES, WAVE_ROWS);
}

/* Drawer 2 (0x08DC39B6, the one the main screen uses): canvas from its frame, 55 rows up from row 70. */
void xdj_wave_paint_column2(int32_t column, int32_t entry, int32_t step, uint8_t *canvas)
{
    xdj_wave_paint2_calls++;
    paint(column, entry, step, canvas ? canvas + 70u * CANVAS_ROW_BYTES : 0, 55u);
}
