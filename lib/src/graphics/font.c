#include <proto/core.h>
#include <proto/graphics.h>

static int read_exact(int fd, void *dst, size_t n) {
    size_t got = 0;
    while (got < n) {
        int r = read(fd, n - got, (char *)dst + got);
        if (r <= 0) { return -1; }
        got += (size_t)r;
    }
    return 0;
}

font_t *font_load(char *path) {
    int fd = open(path, "r");   // adapt flags for your OS
    if (fd < PROTO_OK) { return NULL; }

    font_t *fnt = malloc(sizeof(font_t));
    if (!fnt) {
        close(fd); 
        return NULL;
    }

    fmp_header_t hdr;
    if (read_exact(fd, &hdr, sizeof(hdr)) < 0) { goto fail; }

    if (hdr.magic[0] != 'F' || hdr.magic[1] != 'M' ||
        hdr.magic[2] != 'P' || hdr.magic[3] != '\0') {
        goto fail;
    }

    fnt->width = (int)hdr.width;
    fnt->height = (int)hdr.height;
    fnt->glyph_count = (int)hdr.glyph_count;

    if (fnt->width <= 0 || fnt->width > 32 || fnt->height <= 0 || fnt->glyph_count < 0) {
        goto fail;
    }

    fnt->glyph_ids = malloc((size_t)fnt->glyph_count * sizeof(uint32_t));
    fnt->data = malloc((size_t)fnt->glyph_count * sizeof(char *));
    if (!fnt->glyph_ids || !fnt->data) { goto fail; }

    for (int g = 0; g < fnt->glyph_count; g++) {
        uint32_t char_id = 0;
        if (read_exact(fd, &char_id, sizeof(char_id)) < 0) { goto fail; }
        fnt->glyph_ids[g] = char_id;

        size_t pixels = (size_t)fnt->width * (size_t)fnt->height;
        fnt->data[g] = malloc(pixels);
        if (!fnt->data[g]) { goto fail; }

        for (int r = 0; r < fnt->height; r++) {
            uint32_t row_bits = 0;
            if (read_exact(fd, &row_bits, sizeof(row_bits)) < 0) { goto fail; }

            for (int c = 0; c < fnt->width; c++) {
                int bit = (row_bits >> (fnt->width - 1 - c)) & 1;
                fnt->data[g][(size_t)r * (size_t)fnt->width + (size_t)c] = (char)bit;
            }
        }
    }

    close(fd);
    return fnt;

fail:
    if (fnt) {
        if (fnt->data) {
            for (int i = 0; i < fnt->glyph_count; i++) { 
                free(fnt->data[i]);
        }
        }
        free(fnt->data);
        free(fnt->glyph_ids);
        free(fnt);
    }
    close(fd);
    return NULL;
}

static int font_find_glyph_index(const font_t *fnt, uint32_t cp) {
    for (int i = 0; i < fnt->glyph_count; i++) {
        if (fnt->glyph_ids[i] == cp) return i;
    }
    return -1;
}

void font_putc(fb_info_t *fb, font_t *fnt, char ch, int x, int y, uint32_t fg) {
    uint32_t cp = (uint8_t)ch;   // avoid negative char values
    int gi = font_find_glyph_index(fnt, cp);
    if (gi < 0) return;          // or fallback to '?' / .notdef

    char *bmp = fnt->data[gi];   // width*height bytes, each byte is 0 or 1
    for (int row = 0; row < fnt->height; row++) {
        for (int col = 0; col < fnt->width; col++) {
            if (bmp[(size_t)row * (size_t)fnt->width + (size_t)col]) {
                putpixel(fb, x + col, y + row, fg);
            }
        }
    }
}

void font_print(fb_info_t *fb, font_t *fnt, char *str, int x, int y, uint32_t fg) {
    int pen_x = x;
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            pen_x = x;
            y += fnt->height + 1;
            continue;
        }
        font_putc(fb, fnt, str[i], pen_x, y, fg);
        pen_x += fnt->width + 1;
    }
}