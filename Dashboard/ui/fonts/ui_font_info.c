/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --bpp 1 --size 12 --font C:/Users/Han Gyujin/SquareLine/assets/JetBrainsMono-ExtraBold.ttf -o C:/Users/Han Gyujin/SquareLine/assets\ui_font_info.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
 ******************************************************************************/

#include "../ui.h"

#ifndef UI_FONT_INFO
#define UI_FONT_INFO 1
#endif

#if UI_FONT_INFO

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf0, 0xc0,

    /* U+0022 "\"" */
    0xde, 0xf7, 0xb0,

    /* U+0023 "#" */
    0x24, 0x49, 0xf9, 0x42, 0x85, 0x3f, 0x24, 0x48,

    /* U+0024 "$" */
    0x10, 0x47, 0xf7, 0xd3, 0x43, 0x87, 0xdf, 0xf3,
    0x84, 0x10,

    /* U+0025 "%" */
    0xe3, 0x2a, 0x77, 0x41, 0x7, 0xdc, 0xa9, 0x9e,

    /* U+0026 "&" */
    0x39, 0xa6, 0x18, 0xf3, 0x5d, 0xf2, 0x7c,

    /* U+0027 "'" */
    0xff,

    /* U+0028 "(" */
    0x3, 0x7c, 0xcc, 0xcc, 0xce, 0x73, 0x0,

    /* U+0029 ")" */
    0xc, 0xe3, 0x33, 0x33, 0x37, 0xec, 0x0,

    /* U+002A "*" */
    0x31, 0x6b, 0xf1, 0x87, 0x89, 0x0,

    /* U+002B "+" */
    0x30, 0xcf, 0xcc, 0x30, 0xc0,

    /* U+002C "," */
    0x7b, 0x40,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x8, 0x61, 0x84, 0x30, 0xc6, 0x18, 0x63, 0xc,
    0x30,

    /* U+0030 "0" */
    0x7b, 0x3c, 0xf3, 0xff, 0x3c, 0xf3, 0x78,

    /* U+0031 "1" */
    0x73, 0xcb, 0xc, 0x30, 0xc3, 0xc, 0xfc,

    /* U+0032 "2" */
    0x7b, 0x30, 0xc3, 0x1c, 0xe7, 0x38, 0xfc,

    /* U+0033 "3" */
    0x7c, 0x18, 0x60, 0xe0, 0x60, 0xc1, 0xb3, 0x3c,

    /* U+0034 "4" */
    0x18, 0xe3, 0x18, 0xef, 0x3f, 0xc3, 0xc,

    /* U+0035 "5" */
    0xff, 0xc, 0x3e, 0xcc, 0x30, 0xf3, 0x78,

    /* U+0036 "6" */
    0x30, 0xc6, 0x1e, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0037 "7" */
    0xff, 0x30, 0x86, 0x18, 0x43, 0xc, 0x60,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xff, 0x7b, 0xfc, 0xf3, 0x78,

    /* U+0039 "9" */
    0x7b, 0x3c, 0xf3, 0x7c, 0x61, 0x8c, 0x30,

    /* U+003A ":" */
    0xc0, 0xc,

    /* U+003B ";" */
    0x60, 0x0, 0xda, 0xc0,

    /* U+003C "<" */
    0x4, 0x77, 0xb0, 0xe1, 0xe0, 0xc0,

    /* U+003D "=" */
    0xf8, 0x0, 0xf, 0x80,

    /* U+003E ">" */
    0x3, 0x7, 0x87, 0x1f, 0xcc, 0x0,

    /* U+003F "?" */
    0xf0, 0xc6, 0x26, 0x0, 0x0, 0x60,

    /* U+0040 "@" */
    0x3c, 0xcf, 0x1f, 0xff, 0x7e, 0xfd, 0xff, 0xc0,
    0xc0, 0xc0,

    /* U+0041 "A" */
    0x38, 0x70, 0xa1, 0x66, 0xcd, 0x9f, 0x23, 0xc6,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xf3, 0xf3, 0x3c, 0xf3, 0xf8,

    /* U+0043 "C" */
    0x7b, 0x3c, 0x30, 0xc3, 0xc, 0x33, 0x78,

    /* U+0044 "D" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xf8,

    /* U+0045 "E" */
    0xff, 0xc, 0x30, 0xfb, 0xc, 0x30, 0xfc,

    /* U+0046 "F" */
    0xff, 0xc, 0x30, 0xff, 0xc, 0x30, 0xc0,

    /* U+0047 "G" */
    0x7b, 0x3c, 0x30, 0xdf, 0x3c, 0xf3, 0x78,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xf3, 0xff, 0x3c, 0xf3, 0xcc,

    /* U+0049 "I" */
    0xf9, 0x8c, 0x63, 0x18, 0xc6, 0xf8,

    /* U+004A "J" */
    0x3c, 0x30, 0xc3, 0xc, 0x3c, 0xf3, 0x78,

    /* U+004B "K" */
    0xcf, 0x3d, 0xb6, 0xf3, 0x6d, 0xb3, 0xcc,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xfc,

    /* U+004D "M" */
    0xcf, 0xff, 0xff, 0xff, 0x3c, 0xf3, 0xcc,

    /* U+004E "N" */
    0xef, 0xbe, 0xfb, 0xff, 0x7d, 0xf7, 0xdc,

    /* U+004F "O" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xf3, 0xfb, 0xc, 0x30, 0xc0,

    /* U+0051 "Q" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78, 0x60,
    0xc0,

    /* U+0052 "R" */
    0xfb, 0x3c, 0xf3, 0xfb, 0x6d, 0xb2, 0xcc,

    /* U+0053 "S" */
    0x7b, 0x3c, 0x3c, 0x7c, 0xf0, 0xf3, 0x78,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0056 "V" */
    0xc6, 0x8d, 0x93, 0x66, 0xc5, 0x8e, 0x1c, 0x38,

    /* U+0057 "W" */
    0xd7, 0xaf, 0xdf, 0x7e, 0xed, 0xdb, 0xb6, 0x6c,

    /* U+0058 "X" */
    0x66, 0xd8, 0xf1, 0xc1, 0x87, 0x1b, 0x36, 0xc6,

    /* U+0059 "Y" */
    0x66, 0x66, 0x24, 0x3c, 0x3c, 0x18, 0x18, 0x18,
    0x18,

    /* U+005A "Z" */
    0xf8, 0xcc, 0x62, 0x31, 0x98, 0xf8,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xf0,

    /* U+005C "\\" */
    0xc3, 0xc, 0x18, 0x61, 0x83, 0xc, 0x10, 0x61,
    0x82,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0xf0,

    /* U+005E "^" */
    0x33, 0x95, 0xbc, 0x80,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0xc0,

    /* U+0061 "a" */
    0x7b, 0x30, 0xdf, 0xcf, 0x37, 0xc0,

    /* U+0062 "b" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xf8,

    /* U+0063 "c" */
    0x7b, 0x3c, 0x30, 0xc3, 0x37, 0x80,

    /* U+0064 "d" */
    0xc, 0x37, 0xf3, 0xcf, 0x3c, 0xf3, 0x7c,

    /* U+0065 "e" */
    0x7b, 0x3c, 0xff, 0xc3, 0x37, 0x80,

    /* U+0066 "f" */
    0x1c, 0xc3, 0x3f, 0x30, 0xc3, 0xc, 0x30,

    /* U+0067 "g" */
    0x7f, 0x3c, 0xf3, 0xcd, 0xf0, 0xc3, 0x78,

    /* U+0068 "h" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xcc,

    /* U+0069 "i" */
    0x30, 0xf, 0xc, 0x30, 0xc3, 0xc, 0xfc,

    /* U+006A "j" */
    0x18, 0x1, 0xf1, 0x8c, 0x63, 0x18, 0xc7, 0xe0,

    /* U+006B "k" */
    0xc3, 0xc, 0xf3, 0xdb, 0xcd, 0xb3, 0xcc,

    /* U+006C "l" */
    0xf0, 0x60, 0xc1, 0x83, 0x6, 0xc, 0x18, 0x1e,

    /* U+006D "m" */
    0xff, 0x7d, 0xf7, 0xdf, 0x7d, 0xc0,

    /* U+006E "n" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xc0,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3f, 0xb0, 0xc0,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0xc,

    /* U+0072 "r" */
    0xfb, 0x3c, 0xf0, 0xc3, 0xc, 0x0,

    /* U+0073 "s" */
    0x7c, 0xcd, 0xc3, 0xf0, 0xe8, 0xdf, 0x0,

    /* U+0074 "t" */
    0x30, 0xcf, 0xcc, 0x30, 0xc3, 0xc, 0x1c,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0076 "v" */
    0xc6, 0xc9, 0xb3, 0x62, 0xc7, 0xe, 0x0,

    /* U+0077 "w" */
    0xd3, 0xaf, 0xbb, 0x76, 0xcd, 0x9b, 0x0,

    /* U+0078 "x" */
    0x6c, 0xd8, 0xe1, 0xc3, 0x8d, 0x99, 0x80,

    /* U+0079 "y" */
    0xc6, 0xc9, 0xb1, 0x63, 0x87, 0x6, 0x18, 0x30,

    /* U+007A "z" */
    0xf8, 0xcc, 0xe6, 0x63, 0xe0,

    /* U+007B "{" */
    0xc, 0x61, 0x86, 0x1b, 0x81, 0x86, 0x18, 0x61,
    0xc0,

    /* U+007C "|" */
    0xff, 0xff, 0xff,

    /* U+007D "}" */
    0xe0, 0xc3, 0xc, 0x30, 0x33, 0xc, 0x30, 0xce,
    0x0,

    /* U+007E "~" */
    0xe7, 0xaf, 0x38
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 115, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 115, .box_w = 2, .box_h = 9, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 115, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 7, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 115, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 25, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 115, .box_w = 2, .box_h = 4, .ofs_x = 3, .ofs_y = 5},
    {.bitmap_index = 41, .adv_w = 115, .box_w = 4, .box_h = 13, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 48, .adv_w = 115, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 55, .adv_w = 115, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 61, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 66, .adv_w = 115, .box_w = 3, .box_h = 4, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 68, .adv_w = 115, .box_w = 4, .box_h = 1, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 69, .adv_w = 115, .box_w = 2, .box_h = 1, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 115, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 79, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 115, .box_w = 2, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 115, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 156, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 162, .adv_w = 115, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 166, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 172, .adv_w = 115, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 115, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 188, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 224, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 115, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 115, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 309, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 330, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 337, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 115, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 115, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 115, .box_w = 4, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 382, .adv_w = 115, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 391, .adv_w = 115, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 397, .adv_w = 115, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 401, .adv_w = 115, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 402, .adv_w = 115, .box_w = 3, .box_h = 1, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 403, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 442, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 449, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 463, .adv_w = 115, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 471, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 492, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 504, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 511, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 518, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 524, .adv_w = 115, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 531, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 538, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 544, .adv_w = 115, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 551, .adv_w = 115, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 115, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 565, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 573, .adv_w = 115, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 578, .adv_w = 115, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 587, .adv_w = 115, .box_w = 2, .box_h = 12, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 590, .adv_w = 115, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 599, .adv_w = 115, .box_w = 7, .box_h = 3, .ofs_x = 1, .ofs_y = 3}
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
const lv_font_t ui_font_info = {
#else
lv_font_t ui_font_info = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
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



#endif /*#if UI_FONT_INFO*/

