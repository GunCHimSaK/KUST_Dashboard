/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: --bpp 1 --size 10 --font C:/Users/Han Gyujin/SquareLine/assets/JetBrainsMono-ExtraBold.ttf -o C:/Users/Han Gyujin/SquareLine/assets\ui_font_accunit.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
 ******************************************************************************/

#include "../ui.h"

#ifndef UI_FONT_ACCUNIT
#define UI_FONT_ACCUNIT 1
#endif

#if UI_FONT_ACCUNIT

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xc3,

    /* U+0022 "\"" */
    0xff, 0xf0,

    /* U+0023 "#" */
    0x4a, 0x7e, 0xa5, 0x7e, 0x52,

    /* U+0024 "$" */
    0x21, 0xbf, 0xfe, 0x38, 0xff, 0x79, 0x8,

    /* U+0025 "%" */
    0xe6, 0xab, 0xbc, 0x3d, 0xd5, 0x67,

    /* U+0026 "&" */
    0x72, 0x90, 0xce, 0xef, 0x4f,

    /* U+0027 "'" */
    0xfc,

    /* U+0028 "(" */
    0x37, 0xcc, 0xcc, 0xcc, 0x63,

    /* U+0029 ")" */
    0x8e, 0x73, 0x33, 0x33, 0xec,

    /* U+002A "*" */
    0x32, 0xdf, 0xcc, 0x68, 0x20,

    /* U+002B "+" */
    0x63, 0x3e, 0xc6, 0x0,

    /* U+002C "," */
    0x69, 0x0,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x8, 0x61, 0x84, 0x30, 0xc2, 0x18, 0x61, 0x0,

    /* U+0030 "0" */
    0x7b, 0x3c, 0xfb, 0xcf, 0x3c, 0xde,

    /* U+0031 "1" */
    0x77, 0xac, 0x63, 0x18, 0xdf,

    /* U+0032 "2" */
    0x74, 0xc6, 0x33, 0x33, 0x1f,

    /* U+0033 "3" */
    0x78, 0x61, 0xe, 0xc, 0x34, 0xde,

    /* U+0034 "4" */
    0x31, 0x98, 0xcd, 0xfc, 0x63,

    /* U+0035 "5" */
    0xfe, 0x31, 0xe1, 0x8f, 0x6e,

    /* U+0036 "6" */
    0x21, 0x19, 0xed, 0xef, 0x6e,

    /* U+0037 "7" */
    0xfe, 0x46, 0x23, 0x18, 0x8c,

    /* U+0038 "8" */
    0x76, 0xf7, 0xff, 0xef, 0x6e,

    /* U+0039 "9" */
    0x76, 0xf7, 0xb7, 0x98, 0x84,

    /* U+003A ":" */
    0xf0, 0xf0,

    /* U+003B ";" */
    0x6c, 0x0, 0xd2,

    /* U+003C "<" */
    0x9, 0xf9, 0xf, 0xc,

    /* U+003D "=" */
    0xf0, 0xf,

    /* U+003E ">" */
    0x6, 0x1c, 0x37, 0x60,

    /* U+003F "?" */
    0xf0, 0xc6, 0xe6, 0x0, 0xc,

    /* U+0040 "@" */
    0x74, 0x63, 0x7a, 0xd6, 0xf0, 0x83, 0x0,

    /* U+0041 "A" */
    0x30, 0xc7, 0x9e, 0x49, 0xe4, 0xb3,

    /* U+0042 "B" */
    0xf6, 0xf7, 0xed, 0xef, 0x7e,

    /* U+0043 "C" */
    0x76, 0x71, 0x8c, 0x63, 0x2e,

    /* U+0044 "D" */
    0xf6, 0xf7, 0xbd, 0xef, 0x7e,

    /* U+0045 "E" */
    0xfe, 0x31, 0xfc, 0x63, 0x1f,

    /* U+0046 "F" */
    0xfe, 0x31, 0x8f, 0xe3, 0x18,

    /* U+0047 "G" */
    0x76, 0xf1, 0x8f, 0xef, 0x6e,

    /* U+0048 "H" */
    0xde, 0xf7, 0xfd, 0xef, 0x7b,

    /* U+0049 "I" */
    0xf6, 0x66, 0x66, 0x6f,

    /* U+004A "J" */
    0x3c, 0x30, 0xc3, 0xc, 0x34, 0xde,

    /* U+004B "K" */
    0xcb, 0x6d, 0x3c, 0xd3, 0x6d, 0xb2,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0x1f,

    /* U+004D "M" */
    0xde, 0xf7, 0xff, 0xef, 0x7b,

    /* U+004E "N" */
    0xde, 0xff, 0xff, 0xff, 0x7b,

    /* U+004F "O" */
    0x76, 0xf7, 0xbd, 0xef, 0x6e,

    /* U+0050 "P" */
    0xf6, 0xf7, 0xbf, 0x63, 0x18,

    /* U+0051 "Q" */
    0x76, 0xf7, 0xbd, 0xef, 0x6e, 0x10, 0xc0,

    /* U+0052 "R" */
    0xf6, 0xf7, 0xbf, 0x7b, 0x5b,

    /* U+0053 "S" */
    0x76, 0xf1, 0xe7, 0x8f, 0x6e,

    /* U+0054 "T" */
    0xf9, 0x8c, 0x63, 0x18, 0xc6,

    /* U+0055 "U" */
    0xde, 0xf7, 0xbd, 0xef, 0x6e,

    /* U+0056 "V" */
    0xcd, 0x24, 0x9a, 0x79, 0xe3, 0xc,

    /* U+0057 "W" */
    0xdf, 0xf6, 0xdb, 0x6d, 0xb6, 0xda,

    /* U+0058 "X" */
    0x49, 0xe3, 0x8c, 0x31, 0xe5, 0xb3,

    /* U+0059 "Y" */
    0xcd, 0x37, 0x9e, 0x30, 0xc3, 0xc,

    /* U+005A "Z" */
    0xf3, 0x26, 0x64, 0xcf,

    /* U+005B "[" */
    0xfb, 0x6d, 0xb6, 0xdc,

    /* U+005C "\\" */
    0x43, 0x18, 0x43, 0x18, 0x43, 0x18, 0x40,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x66, 0xf9,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x60,

    /* U+0061 "a" */
    0x76, 0xdf, 0xbd, 0xfc,

    /* U+0062 "b" */
    0xc6, 0x3d, 0xbd, 0xef, 0x7e,

    /* U+0063 "c" */
    0x76, 0x71, 0x8c, 0xb8,

    /* U+0064 "d" */
    0x18, 0xdf, 0xbd, 0xef, 0x6f,

    /* U+0065 "e" */
    0x76, 0xff, 0x8d, 0xb8,

    /* U+0066 "f" */
    0x3b, 0x19, 0xf6, 0x31, 0x8c,

    /* U+0067 "g" */
    0x7e, 0xf7, 0xbd, 0xbc, 0x6e,

    /* U+0068 "h" */
    0xc6, 0x3d, 0xbd, 0xef, 0x7b,

    /* U+0069 "i" */
    0x3, 0x1, 0xc6, 0x31, 0x8c, 0xf8,

    /* U+006A "j" */
    0x0, 0xc0, 0xf1, 0x8c, 0x63, 0x18, 0xfc,

    /* U+006B "k" */
    0xc3, 0xc, 0xb6, 0xd3, 0xcd, 0xb2,

    /* U+006C "l" */
    0xf0, 0xc3, 0xc, 0x30, 0xc3, 0x7,

    /* U+006D "m" */
    0xff, 0xbe, 0xfb, 0xef, 0xb0,

    /* U+006E "n" */
    0xf6, 0xf7, 0xbd, 0xec,

    /* U+006F "o" */
    0x76, 0xf7, 0xbd, 0xb8,

    /* U+0070 "p" */
    0xf6, 0xf7, 0xbd, 0xfb, 0x18,

    /* U+0071 "q" */
    0x7e, 0xf7, 0xbd, 0xbc, 0x63,

    /* U+0072 "r" */
    0xf6, 0xf7, 0x8c, 0x60,

    /* U+0073 "s" */
    0x76, 0x7c, 0xf9, 0xf8,

    /* U+0074 "t" */
    0x63, 0x3e, 0xc6, 0x31, 0x87,

    /* U+0075 "u" */
    0xde, 0xf7, 0xbd, 0xb8,

    /* U+0076 "v" */
    0xcd, 0x24, 0x9e, 0x30, 0xc0,

    /* U+0077 "w" */
    0xb6, 0xdf, 0xde, 0x79, 0x20,

    /* U+0078 "x" */
    0x49, 0xe3, 0xc, 0x79, 0x20,

    /* U+0079 "y" */
    0xcd, 0x27, 0x8e, 0x30, 0xc3, 0x8,

    /* U+007A "z" */
    0xf3, 0x66, 0xcf,

    /* U+007B "{" */
    0x19, 0x8c, 0x6c, 0x18, 0xc6, 0x31, 0xc0,

    /* U+007C "|" */
    0xff, 0xff, 0xf0,

    /* U+007D "}" */
    0xe3, 0x18, 0xc3, 0xb1, 0x8c, 0x67, 0x0,

    /* U+007E "~" */
    0xe5, 0x4e
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 96, .box_w = 2, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 5, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 10, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 17, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 96, .box_w = 2, .box_h = 3, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 29, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 34, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 39, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 44, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 48, .adv_w = 96, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 50, .adv_w = 96, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 51, .adv_w = 96, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 60, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 96, .box_w = 2, .box_h = 6, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 96, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 117, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 121, .adv_w = 96, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 123, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 127, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 139, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 228, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 260, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 96, .box_w = 3, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 280, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 287, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 292, .adv_w = 96, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 294, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 295, .adv_w = 96, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 296, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 328, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 346, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 358, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 363, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 376, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 381, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 419, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 429, .adv_w = 96, .box_w = 2, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 432, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 439, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_accunit = {
#else
lv_font_t ui_font_accunit = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 11,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_ACCUNIT*/

