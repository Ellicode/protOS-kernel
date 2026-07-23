#include <proto/core.h>
#include <proto/graphics.h>

// fuck it i will vibecode ts T-T
bmp_t *bmp_load(const char *path) {
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

void bmp_draw(fb_info_t *fb, bmp_t *bmp, int x, int y) {
    if (fb == NULL || bmp == NULL || bmp->data == NULL) { return; }
    draw_img(fb, bmp->data, x, y, bmp->width, bmp->height);
}

void bmp_free(bmp_t *bmp) {
    if (!bmp) { return; }
    free(bmp->data);
    free(bmp);
}