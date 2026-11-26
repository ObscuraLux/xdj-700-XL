/* 3-band waveform test build: entry wrappers for routines Pioneer's code calls.
 *
 * Not part of XDJ-700 PLUS. The stock compiler keeps MACH and MACL across a
 * call (its functions restore them in their epilogues); GCC does not. Each
 * wrapper saves them and PR, calls the C function with the same r4..r7, and
 * restores them. r0 (the result) passes through.
 */

    .section .text.xdj_wave_glue,"ax"
    .align 2

    .macro XDJ_WRAP name, target
    .global \name
    .type \name,@function
\name:
    sts.l   pr,@-r15
    sts.l   mach,@-r15
    sts.l   macl,@-r15
    mov.l   1f,r0
    jsr     @r0
     nop
    lds.l   @r15+,macl
    lds.l   @r15+,mach
    lds.l   @r15+,pr
    rts
     nop
    .align 2
1:  .long   \target
    .size \name,.-\name
    .endm

    XDJ_WRAP _xdj_wave_fill_color_entry, _xdj_wave_fill_color
    XDJ_WRAP _xdj_wave_commit_color_entry, _xdj_wave_commit_color

#ifdef XDJ_WAVE_DIAG
    XDJ_WRAP _xdj_wave_fill_info1_entry, _xdj_wave_fill_info1
    XDJ_WRAP _xdj_wave_fill_info2_entry, _xdj_wave_fill_info2
    XDJ_WRAP _xdj_wave_fill_info3_entry, _xdj_wave_fill_info3
    XDJ_WRAP _xdj_wave_fill_info4_entry, _xdj_wave_fill_info4
    XDJ_WRAP _xdj_wave_commit_none_entry, _xdj_wave_commit_none
#endif

    XDJ_WRAP _xdj_wave_getter_entry, _xdj_wave_getter
    XDJ_WRAP _xdj_wave_unpack_entry, _xdj_wave_unpack

/* The column hook. The stock column loop of the scrolling waveform's drawer
 * (0x08DC2754) ends at 0x08DC29A6; the build replaces the five instructions there
 * (10 bytes) with
 *     mov.l @(1,pc),r0 / jmp @r0 / nop / .long _xdj_wave_column_hook
 * We paint the column (or not), run the five displaced instructions, and go on
 * at 0x08DC29B0 with every other register as it was. The drawer's frame:
 * @(16,r15) entries per column, @(28,r15) the column, @(36,r15) its first entry,
 * @(8,r15) the value r13 gets back. r0 is free there (written before it is read).
 */
    .global _xdj_wave_column_hook
    .type _xdj_wave_column_hook,@function
_xdj_wave_column_hook:
    mov.l   r1,@-r15
    mov.l   r2,@-r15
    mov.l   r3,@-r15
    mov.l   r4,@-r15
    mov.l   r5,@-r15
    mov.l   r6,@-r15
    mov.l   r7,@-r15
    sts.l   pr,@-r15
    sts.l   mach,@-r15
    sts.l   macl,@-r15          /* 10 words: the drawer's frame is now at r15+40 */
    mov     #40+28,r0
    mov.l   @(r0,r15),r4        /* column */
    mov     #40+36,r0
    mov.l   @(r0,r15),r5        /* first entry */
    mov     #40+16,r0
    mov.l   @(r0,r15),r6        /* entries per column */
    mov.l   .Lpaint,r0
    jsr     @r0
     nop
    lds.l   @r15+,macl
    lds.l   @r15+,mach
    lds.l   @r15+,pr
    mov.l   @r15+,r7
    mov.l   @r15+,r6
    mov.l   @r15+,r5
    mov.l   @r15+,r4
    mov.l   @r15+,r3
    mov.l   @r15+,r2
    mov.l   @r15+,r1
    /* the five stock instructions this hook displaced */
    mov.l   .Lstock_r1,r1
    mov     #0,r12
    mov.l   .Lstock_r5,r5
    mov.l   @(8,r15),r13
    mov.l   @(28,r15),r10
    mov.l   .Lresume,r0
    jmp     @r0
     nop
    .align 2
.Lpaint:    .long _xdj_wave_paint_column
.Lstock_r1: .long 0x0E736124
.Lstock_r5: .long 0x0BA3C050
.Lresume:   .long 0x08DC29B0
    .size _xdj_wave_column_hook,.-_xdj_wave_column_hook

/* The same for drawer 2 (0x08DC39B6, the main screen's): its column loop ends at 0x08DC3BFE
 * Frame: @(16) entries per column, @(28) column, @(36) first entry,
 * @(12) the canvas, @(8) what r11 gets back. Resume at 0x08DC3C08. */
    .global _xdj_wave_column_hook2
    .type _xdj_wave_column_hook2,@function
_xdj_wave_column_hook2:
    mov.l   r1,@-r15
    mov.l   r2,@-r15
    mov.l   r3,@-r15
    mov.l   r4,@-r15
    mov.l   r5,@-r15
    mov.l   r6,@-r15
    mov.l   r7,@-r15
    sts.l   pr,@-r15
    sts.l   mach,@-r15
    sts.l   macl,@-r15          /* 10 words: the drawer's frame is now at r15+40 */
    mov     #40+28,r0
    mov.l   @(r0,r15),r4        /* column */
    mov     #40+36,r0
    mov.l   @(r0,r15),r5        /* first entry */
    mov     #40+16,r0
    mov.l   @(r0,r15),r6        /* entries per column */
    mov     #40+12,r0
    mov.l   @(r0,r15),r7        /* canvas */
    mov.l   .Lpaint2,r0
    jsr     @r0
     nop
    lds.l   @r15+,macl
    lds.l   @r15+,mach
    lds.l   @r15+,pr
    mov.l   @r15+,r7
    mov.l   @r15+,r6
    mov.l   @r15+,r5
    mov.l   @r15+,r4
    mov.l   @r15+,r3
    mov.l   @r15+,r2
    mov.l   @r15+,r1
    /* the five stock instructions this hook displaced */
    mov.l   .Lstock2_r1,r1
    mov     #0,r13
    mov.l   .Lstock2_r5,r5
    mov.l   @(8,r15),r11
    mov.l   @(28,r15),r10
    mov.l   .Lresume2,r0
    jmp     @r0
     nop
    .align 2
.Lpaint2:    .long _xdj_wave_paint_column2
.Lstock2_r1: .long 0x0E736270
.Lstock2_r5: .long 0x0BA3C050
.Lresume2:   .long 0x08DC3C08
    .size _xdj_wave_column_hook2,.-_xdj_wave_column_hook2
