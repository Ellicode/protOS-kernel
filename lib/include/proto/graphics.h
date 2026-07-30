#ifndef PROTO_GRAPHICS_H
#define PROTO_GRAPHICS_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint64_t bpp;
} fb_info_t;

int fetch_framebuffer(fb_info_t *fb);

/*****************************************************************************
 * Basic VFX Operations
 *****************************************************************************/

void putpixel(fb_info_t *fb, int x, int y, uint32_t color);
void putpixel_a(fb_info_t *fb, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_rect(fb_info_t *fb, int x, int y, int w, int h, uint32_t color);
void draw_rect_o(fb_info_t *fb, int x, int y, int w, int h, uint32_t color);
void invert_rect_o(fb_info_t *fb, int x, int y, int w, int h);
void draw_img(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h);
void draw_img_a(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h);
void capture_rect(fb_info_t *fb, uint32_t *buf, int x, int y, int w, int h);
void draw_notex(fb_info_t *fb, int x, int y, int w, int h);
void draw_box(fb_info_t *fb, int x, int y, int w, int h, uint32_t color, int border, char fill);

/*****************************************************************************
 * BMP file support
 *****************************************************************************/

typedef struct bmp_t {
    int         width;
    int         height;
    uint32_t    *data;
} bmp_t;

bmp_t *bmp_load(const char *path);
void bmp_draw(fb_info_t *fb, bmp_t *bmp, int x, int y);
void bmp_free(bmp_t *bmp);

/*****************************************************************************
 * FMP file support
 *****************************************************************************/

typedef struct fmp_header_t {
    char        magic[4];
    uint16_t    width;
    uint16_t    height;
    uint32_t    glyph_count;
} fmp_header_t;

typedef struct font_t {
    int         width;
    int         height;
    int         glyph_count;
    uint32_t   *glyph_ids;
    char        **data;
} font_t;
 
font_t *font_load(char *path);
void font_putc(fb_info_t *fb, font_t *fnt, char c, int x, int y, uint32_t fg);
void font_print(fb_info_t *fb, font_t *fnt, char *str, int x, int y, uint32_t fg);

/**
 * Color utilities
 */

#define FX_ONE  ((int32_t)65536)
#define FX_HALF ((int32_t)32768)

typedef struct {
    int32_t h;
    int32_t s;
    int32_t l;
} hsl_t;

hsl_t rgb2hsl(uint32_t rgb);
uint32_t hsl2rgb(hsl_t hsl);

uint32_t lighten(uint32_t rgb, int32_t amount);
uint32_t darken(uint32_t rgb, int32_t amount);

#endif // PROTO_GRAPHICS_H