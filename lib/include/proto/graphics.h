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
 * Helpers
 *****************************************************************************/

static inline int min_i(int a, int b)
{
    return a < b ? a : b;
}

static inline int max_i(int a, int b)
{
    return a > b ? a : b;
}

static inline int clip_rect(
    fb_info_t *fb,
    int x, int y, int w, int h,
    int cx, int cy, int cw, int ch,
    int *out_x, int *out_y,
    int *out_w, int *out_h
) {
    if (!fb || !fb->address || w <= 0 || h <= 0 ||
        cw <= 0 || ch <= 0) {
        return 0;
    }

    int left   = max_i(x, cx);
    int top    = max_i(y, cy);
    int right  = min_i(x + w, cx + cw);
    int bottom = min_i(y + h, cy + ch);

    // Framebuffer bounds.
    left   = max_i(left, 0);
    top    = max_i(top, 0);
    right  = min_i(right, (int)fb->width);
    bottom = min_i(bottom, (int)fb->height);

    if (left >= right || top >= bottom)
        return 0;

    *out_x = left;
    *out_y = top;
    *out_w = right - left;
    *out_h = bottom - top;

    return 1;
}

static inline uint32_t *fb_pixel_ptr(fb_info_t *fb, int x, int y) {
    return ((uint32_t *)fb->address) + (uint32_t)y * (fb->pitch / 4) + (uint32_t)x;
}

static inline uint32_t *fb_row_ptr(fb_info_t *fb, int y) {
    return ((uint32_t *)fb->address) + (uint32_t)y * (fb->pitch / 4);
}

/*****************************************************************************
 * Basic VFX Operations
 *****************************************************************************/

void putpixel(fb_info_t *fb, int x, int y, uint32_t color);
void putpixel_a(fb_info_t *fb, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void draw_rect(fb_info_t *fb, int x, int y, int w, int h, uint32_t color);
void draw_rect_clip(
    fb_info_t *fb,
    int x, int y, int w, int h,
    uint32_t color,
    int cx, int cy, int cw, int ch
);
void draw_rect_c(fb_info_t *fb, int x, int y, int w, int h, uint32_t color1, uint32_t color2);
void draw_rect_c_clip(
    fb_info_t *fb,
    int x, int y, int w, int h,
    uint32_t color1,
    uint32_t color2,
    int cx, int cy, int cw, int ch
);
void draw_rect_o(fb_info_t *fb, int x, int y, int w, int h, uint32_t color);
void draw_rect_o_clip(
    fb_info_t *fb,
    int x, int y, int w, int h,
    uint32_t color,
    int cx, int cy, int cw, int ch
);
void invert_rect_o(fb_info_t *fb, int x, int y, int w, int h);

void draw_img(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h);
void draw_img_clip(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h, int cx, int cy, int cw, int ch);
void draw_img_a(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h);
void draw_img_a_clip(
    fb_info_t *fb,
    const uint32_t *img,
    int x, int y, int w, int h,
    int cx, int cy, int cw, int ch
);

void capture_rect(fb_info_t *fb, uint32_t *buf, int x, int y, int w, int h);
void draw_notex(fb_info_t *fb, int x, int y, int w, int h);

void draw_box(fb_info_t *fb, int x, int y, int w, int h, uint32_t color, int border, char fill);
void draw_box_clip(
    fb_info_t *fb,
    int x, int y, int w, int h,
    uint32_t color,
    int border,
    char fill,
    int cx, int cy, int cw, int ch
);

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
void draw_char(fb_info_t *fb, font_t *fnt, char c, int x, int y, uint32_t fg);
void draw_char_clip(
    fb_info_t *fb,
    font_t *fnt,
    char c,
    int x,
    int y,
    uint32_t fg,
    int cx,
    int cy,
    int cw,
    int ch
);

void draw_text(fb_info_t *fb, font_t *fnt, char *str, int x, int y, uint32_t fg);
void draw_text_clip(
    fb_info_t *fb,
    font_t *fnt,
    char *str,
    int x,
    int y,
    uint32_t fg,
    int cx,
    int cy,
    int cw,
    int chh
);

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
