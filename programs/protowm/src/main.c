#include <proto.h>

void putpixel(fb_info_t *fb, uint32_t x, uint32_t y, uint32_t color) {
    if (fb == NULL) { return; }
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


int pmain(char argv[16][64], int argc) {
    fb_info_t *front = malloc(sizeof(fb_info_t));
    fetch_framebuffer(front);
    if (front == NULL) {
        fprintf(STDERR, "No fb\n");
        return 1;
    }

    size_t fb_size = (front->width * front->height);
    void* back_data = malloc(fb_size);
    fb_info_t *back = malloc(sizeof(fb_info_t));
    back->address = (uint64_t)back_data;
    back->bpp       = front->bpp;
    back->width     = front->width;
    back->height    = front->height;
    back->pitch     = front->pitch;


    // draw_rect(front, 0, 0, front->width, front->height, 0xFF0000);

    // swap buffers
    // memcpy((void *)front->address, (void *)back->address, fb_size);

    printf("resolution=%dx%d; bpp=%d\n", front->width, front->height, front->bpp);
    printf("pid=%d\n", getpid());
    
    ipc_meta_t *meta = malloc(sizeof(ipc_meta_t));
    char *data = malloc(1);

    int res = ipc_recieve(meta, data);
    printf("res=%d\n", res);
    if (meta == NULL) {
        printf("buf is null\n");
    } else {
        printf("IPC recieved! name=%s, sender=%d, size=%d \n", meta->name, meta->sender, meta->size);
        printf("char=%c\n", *data);
    }

    free(back_data);
    free(back);
    free(front);

    return 0;
}