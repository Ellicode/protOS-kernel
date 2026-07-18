#include "graphics.h"

void putpixel(fb_info_t *fb, uint32_t x, uint32_t y, uint32_t color) {
    if (fb == NULL) { return; }

    if (x < 0 || y < 0) { return; }
    if (x >= fb->width || y >= fb->height) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    fb_ptr[y * (fb->pitch / 4) + x] = color;
}

void draw_rect(fb_info_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (fb == NULL) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    
    // Pitch is in bytes, so divide by 4 to use with a uint32_t pointer
    uint32_t fb_pitch = fb->pitch / 4;

    for (uint32_t row = y; row < y + h; row++) {
        for (uint32_t col = x; col < x + w; col++) {
            if (row > fb->height || col > fb->width) { continue; }
            fb_ptr[row * fb_pitch + col] = color;
        }
    }
}

void draw_box(fb_info_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {

}

static inline uint32_t min_u32(uint32_t a, uint32_t b) { return (a < b) ? a : b; }

void putpixel_a(fb_info_t *fb, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (fb == NULL) { return; }

    if (x < 0 || y < 0) { return; }
    if (x >= fb->width || y >= fb->height) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    uint32_t idx = (uint32_t)y * (fb->pitch / 4) + (uint32_t)x;

    if (a == 255) {
        fb_ptr[idx] = (r << 16) | (g << 8) | b;
        return;
    }
    if (a == 0) { return; }

    uint32_t bg_pixel = fb_ptr[idx];
    uint8_t bg_r = (bg_pixel >> 16) & 0xFF;
    uint8_t bg_g = (bg_pixel >> 8) & 0xFF;
    uint8_t bg_b = bg_pixel & 0xFF;

    uint8_t out_r = ((r * a) + (bg_r * (255 - a))) / 255;
    uint8_t out_g = ((g * a) + (bg_g * (255 - a))) / 255;
    uint8_t out_b = ((b * a) + (bg_b * (255 - a))) / 255;

    fb_ptr[idx] = (out_r << 16) | (out_g << 8) | out_b;
}

bmp_t *load_bitmap(const char *path) {
    if (path == NULL) { return NULL; }

    int fd = open(path, "r");
    if (fd < 0) { return NULL; }

    uint8_t file_hdr[14];
    if (read(fd, 14, file_hdr) < 0) { close(fd); return NULL; }
    if (file_hdr[0] != 'B' || file_hdr[1] != 'M') { close(fd); return NULL; }

    uint32_t pixel_offset =
        (uint32_t)file_hdr[10] |
        ((uint32_t)file_hdr[11] << 8) |
        ((uint32_t)file_hdr[12] << 16) |
        ((uint32_t)file_hdr[13] << 24);

    uint8_t dib_size_buf[4];
    if (read(fd, 4, dib_size_buf)  < 0) { close(fd); return NULL; }
    uint32_t dib_size =
        (uint32_t)dib_size_buf[0] |
        ((uint32_t)dib_size_buf[1] << 8) |
        ((uint32_t)dib_size_buf[2] << 16) |
        ((uint32_t)dib_size_buf[3] << 24);

    if (dib_size < 40) { close(fd); return NULL; }

    uint8_t dib_rest[36];
    if (read(fd, 36, dib_rest) < 0) { close(fd); return NULL; }

    int32_t width =
        (int32_t)((uint32_t)dib_rest[0] |
        ((uint32_t)dib_rest[1] << 8) |
        ((uint32_t)dib_rest[2] << 16) |
        ((uint32_t)dib_rest[3] << 24));

    int32_t height_signed =
        (int32_t)((uint32_t)dib_rest[4] |
        ((uint32_t)dib_rest[5] << 8) |
        ((uint32_t)dib_rest[6] << 16) |
        ((uint32_t)dib_rest[7] << 24));

    uint16_t planes = (uint16_t)dib_rest[8] | ((uint16_t)dib_rest[9] << 8);
    uint16_t bpp    = (uint16_t)dib_rest[10] | ((uint16_t)dib_rest[11] << 8);
    uint32_t compression =
        (uint32_t)dib_rest[12] |
        ((uint32_t)dib_rest[13] << 8) |
        ((uint32_t)dib_rest[14] << 16) |
        ((uint32_t)dib_rest[15] << 24);

    if (planes != 1 || (bpp != 24 && bpp != 32) || width <= 0 || height_signed == 0) {
        close(fd);
        return NULL;
    }

    if (!(compression == 0 || compression == 3)) { 
        close(fd);
        return NULL;
    }
    uint32_t height = (height_signed < 0) ? (uint32_t)(-height_signed) : (uint32_t)height_signed;
    int top_down = (height_signed < 0);

    // skip to pixel array
    uint32_t consumed = 14 + 40;
    if (pixel_offset < consumed) { close(fd); return NULL; }

    uint32_t to_skip = pixel_offset - consumed;
    uint8_t skipbuf[128];
    while (to_skip) {
        uint32_t chunk = (to_skip < sizeof(skipbuf)) ? to_skip : (uint32_t)sizeof(skipbuf);
        int n = read(fd, chunk, skipbuf);
        if (n != (int)chunk) { close(fd); return NULL; }
        to_skip -= chunk;
    }

    bmp_t *bmp = malloc(sizeof(bmp_t));
    if (!bmp) { close(fd); return NULL; }

    bmp->width = (uint32_t)width;
    bmp->height = height;
    bmp->data = malloc((size_t)bmp->width * bmp->height * sizeof(uint32_t));
    if (!bmp->data) { free(bmp); close(fd); return NULL; }

    uint32_t row_raw = (bpp == 24) ? (bmp->width * 3) : (bmp->width * 4);
    uint32_t row_padded = (row_raw + 3) & ~3U; // BMP rows are 4-byte aligned
    uint8_t *row = malloc(row_padded);
    if (!row) { free(bmp->data); free(bmp); close(fd); return NULL; }

    for (uint32_t r = 0; r < bmp->height; r++) {
        if (read(fd, row_padded, row) != (int)row_padded) {
            free(row); free(bmp->data); free(bmp); close(fd); return NULL;
        }

        uint32_t dst_r = top_down ? r : (bmp->height - 1 - r);

        for (uint32_t c = 0; c < bmp->width; c++) {
            uint8_t b = row[c * (bpp / 8) + 0];
            uint8_t g = row[c * (bpp / 8) + 1];
            uint8_t rr = row[c * (bpp / 8) + 2];
            uint8_t a = (bpp == 32) ? row[c * 4 + 3] : 255;

            bmp->data[dst_r * bmp->width + c] =
                ((uint32_t)a << 24) | ((uint32_t)rr << 16) | ((uint32_t)g << 8) | b;
        }
    }

    free(row);
    close(fd);
    return bmp;
}

void draw_bitmap(fb_info_t *fb, bmp_t *bmp, int x, int y) {
    if (!fb || !bmp || !bmp->data) { return; }

    for (uint32_t row = 0; row < bmp->height; row++) {
        for (uint32_t col = 0; col < bmp->width; col++) {
            uint32_t pixel = bmp->data[row * bmp->width + col];

            uint8_t a = (pixel >> 24) & 0xFF;
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            putpixel_a(fb, x + (int)col, y + (int)row, r, g, b, a);
        }
    }
}

void free_bitmap(bmp_t *bmp) {
    if (!bmp) { return; }
    free(bmp->data);
    free(bmp);
}