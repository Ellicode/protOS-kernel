#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "fonts/niji-pixel-bold-16.h"
#include "debug/serial.h"
#include "string.h"
#include "globals.h"

#include "graphics/tty.h"

void cursor_set(uint32_t x, uint32_t y)
{
    if (g_vga_active_framebuffer == NULL)
    {
        return;
    }
    g_tty_cursor_x = x < 0 ? 0 : x;
    g_tty_cursor_y = y < 0 ? 0 : y;
}

static void scroll_up(void)
{
    if (g_vga_active_framebuffer == NULL)
    {
        return;
    }

    uint32_t pitch_px = g_vga_active_framebuffer->pitch / 4;
    uint32_t width = g_vga_active_framebuffer->width;
    uint32_t height = g_vga_active_framebuffer->height;
    volatile uint32_t *fb = g_vga_active_framebuffer->address;

    // Move every row up by FONT_HEIGHT rows
    for (uint32_t row = 0; row < height - FONT_HEIGHT; row++)
    {
        for (uint32_t col = 0; col < width; col++)
        {
            fb[row * pitch_px + col] = fb[(row + FONT_HEIGHT) * pitch_px + col];
        }
    }

    // Clear the bottom FONT_HEIGHT rows
    for (uint32_t row = height - FONT_HEIGHT; row < height; row++)
    {
        for (uint32_t col = 0; col < width; col++)
        {
            fb[row * pitch_px + col] = 0x000000;
        }
    }
}

void cursor_newline(void)
{
    if (g_vga_active_framebuffer == NULL)
    {
        return;
    }
    g_tty_cursor_x = 0;
    g_tty_cursor_y += FONT_HEIGHT;
    if (g_tty_cursor_y + FONT_HEIGHT > g_vga_active_framebuffer->height)
    {
        scroll_up();
        g_tty_cursor_y -= FONT_HEIGHT;
    }
}

static void cursor_advance(void)
{
    if (g_vga_active_framebuffer == NULL)
    {
        return;
    }
    g_tty_cursor_x += FONT_WIDTH + FONT_KERNING;
    if (g_tty_cursor_x + FONT_WIDTH > g_vga_active_framebuffer->width)
    {
        cursor_newline();
    }
}

void draw_char(uint32_t x, uint32_t y, const char c, color_t color)
{
    if (g_vga_active_framebuffer == NULL)
    {
        return;
    }
    for (int row = 0; row < FONT_HEIGHT; row++)
    {
        for (int col = 0; col < FONT_WIDTH; col++)
        {
            if (font[c - 32][FONT_HEIGHT - 1 - row] & (1 << (7 - col)))
            {
                putpixel(x + col, y + row, color);
            }
        }
    }
}

void draw_text(uint32_t x, uint32_t y, const char *text, color_t color)
{
    if (g_vga_active_framebuffer == NULL)
    {
        return;
    }

    cursor_set(x, y);

    for (size_t i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '\n')
        {
            cursor_newline();
        }
        else
        {
            draw_char(g_tty_cursor_x, g_tty_cursor_y, text[i], color);
            cursor_advance();
        }
    }
}

void print(const char *text)
{
    if (g_vga_active_framebuffer != NULL)
    {
        draw_text(g_tty_cursor_x, g_tty_cursor_y, text, g_tty_current_color);
    }
    

    const char *ansi_code = ANSI_RESET;

    if (g_tty_current_color == PROTO_RED)         { ansi_code = ANSI_RED; }
    else if (g_tty_current_color == PROTO_YELLOW) { ansi_code = ANSI_YELLOW; }
    else if (g_tty_current_color == PROTO_GREEN)  { ansi_code = ANSI_GREEN; }
    else if (g_tty_current_color == PROTO_CYAN)   { ansi_code = ANSI_CYAN; }
    else if (g_tty_current_color == PROTO_BLUE)   { ansi_code = ANSI_BLUE; }
    else if (g_tty_current_color == PROTO_MAGENTA){ ansi_code = ANSI_MAGENTA; }
    else if (g_tty_current_color == PROTO_WHITE)  { ansi_code = ANSI_WHITE; };

    // First print the color ansi code
    for (size_t c = 0; ansi_code[c] != '\0'; c++) {
        serial_write(ansi_code[c]);
    }

    // Then the actual text for the print
    for (size_t c = 0; text[c] != '\0'; c++){
        serial_write(text[c]);
    }

    // Finally reset the terminal colors
    for (size_t c = 0; ANSI_RESET[c] != '\0'; c++) {
        serial_write(ANSI_RESET[c]);
    }
}

void set_color(color_t color)
{
    g_tty_current_color = color;
}

void print_f(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[1024];
    buffer[0] = '\0';
    int buffer_index = 0;

    for (size_t i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%')
        {
            i++;
            if (format[i] == 'd')
            {
                int value = va_arg(args, int);
                char *str = int_to_string(value);
                str_cat(buffer, str);
                buffer_index = str_len(buffer);
            }
            else if (format[i] == 'x')
            {
                unsigned long value = va_arg(args, unsigned long);
                char *str = hex_to_string((uint64_t)value);
                str_cat(buffer, str);
                buffer_index = str_len(buffer);
            }
            else if (format[i] == 's')
            {
                char *str = va_arg(args, char *);
                str_cat(buffer, str);
                buffer_index = str_len(buffer);
            }
        }
        else
        {
            buffer[buffer_index++] = format[i];
            buffer[buffer_index] = '\0';
        }
    }
    va_end(args);

    print(buffer);
}