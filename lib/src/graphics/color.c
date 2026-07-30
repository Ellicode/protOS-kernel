#include <proto/graphics.h>

static int32_t clamp_i32(int32_t x, int32_t lo, int32_t hi) {
    if (x < lo) { return lo; }
    if (x > hi) { return hi; }
    return x;
}

static int32_t abs_i32(int32_t x) { return (x < 0) ? -x : x; }

// (a * b) / FX_ONE with rounding, using 64-bit intermediate
static int32_t fx_mul(int32_t a, int32_t b) {
    int64_t t = (int64_t)a * (int64_t)b;
    t += (t >= 0) ? FX_HALF : -FX_HALF;
    return (int32_t)(t >> 16);
}

// (a / b) in Q16.16 with rounding
static int32_t fx_div(int32_t a, int32_t b) {
    if (b == 0) { return 0; }
    int64_t num = ((int64_t)a << 16);
    if ((num ^ b) >= 0) { num += (b / 2); }
    else                { num -= (b / 2); }
    return (int32_t)(num / b);
}

// Convert byte [0..255] to Q16.16 [0..1]
static int32_t byte_to_fx(uint32_t v) {
    // round(v / 255 * FX_ONE)
    return (int32_t)((v * (uint32_t)FX_ONE + 127U) / 255U);
}

// Convert Q16.16 [0..1] to byte [0..255]
static uint32_t fx_to_byte(int32_t x) {
    x = clamp_i32(x, 0, FX_ONE);
    // round(x * 255 / FX_ONE)
    return (uint32_t)(((int64_t)x * 255 + FX_HALF) >> 16);
}

hsl_t rgb2hsl(uint32_t rgb) {
    int32_t r = byte_to_fx((rgb >> 16) & 0xFFU);
    int32_t g = byte_to_fx((rgb >> 8)  & 0xFFU);
    int32_t b = byte_to_fx( rgb        & 0xFFU);

    int32_t max = r; if (g > max) max = g; if (b > max) max = b;
    int32_t min = r; if (g < min) min = g; if (b < min) min = b;

    int32_t delta = max - min;

    hsl_t out;
    out.l = (max + min) / 2;

    if (delta == 0) {
        out.h = 0;
        out.s = 0;
        return out;
    }

    // s = delta / (1 - |2l - 1|)
    int32_t denom = FX_ONE - abs_i32((out.l << 1) - FX_ONE);
    out.s = (denom == 0) ? 0 : fx_div(delta, denom);
    out.s = clamp_i32(out.s, 0, FX_ONE);

    // Hue in degrees (integer)
    // region formulas:
    // max=r: 60 * ((g-b)/delta mod 6)
    // max=g: 60 * ((b-r)/delta + 2)
    // max=b: 60 * ((r-g)/delta + 4)
    int32_t h;

    if (max == r) {
        int32_t num = (g - b) * 60;
        h = (delta == 0) ? 0 : (num / delta);
        while (h < 0) h += 360;
        while (h >= 360) h -= 360;
    } else if (max == g) {
        int32_t num = (b - r) * 60;
        h = (delta == 0) ? 0 : (num / delta) + 120;
        while (h < 0) h += 360;
        while (h >= 360) h -= 360;
    } else {
        int32_t num = (r - g) * 60;
        h = (delta == 0) ? 0 : (num / delta) + 240;
        while (h < 0) h += 360;
        while (h >= 360) h -= 360;
    }

    out.h = h;
    return out;
}

uint32_t hsl2rgb(hsl_t hsl) {
    int32_t h = hsl.h;
    int32_t s = clamp_i32(hsl.s, 0, FX_ONE);
    int32_t l = clamp_i32(hsl.l, 0, FX_ONE);

    while (h < 0) { 
        h += 360;
    }
    while (h >= 360) {
        h -= 360;
    }

    // c = (1 - |2l - 1|) * s
    int32_t c = fx_mul(FX_ONE - abs_i32((l << 1) - FX_ONE), s);

    // h sector and x
    // x = c * (1 - |(h/60 mod 2) - 1|)
    // Do this in rational/integer form to avoid float:
    int32_t h_mod120 = h % 120;           // [0..119]
    int32_t dist = abs_i32(h_mod120 - 60);// [0..60]
    // factor = 1 - dist/60 in Q16.16
    int32_t factor = FX_ONE - fx_div(dist * FX_ONE, 60 * FX_ONE);
    int32_t x = fx_mul(c, factor);

    int32_t r1 = 0;
    int32_t g1 = 0;
    int32_t b1 = 0;

    if (h < 60)       { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300) { r1 = x; g1 = 0; b1 = c; }
    else              { r1 = c; g1 = 0; b1 = x; }

    int32_t m = l - (c / 2);

    uint32_t r = fx_to_byte(r1 + m);
    uint32_t g = fx_to_byte(g1 + m);
    uint32_t b = fx_to_byte(b1 + m);

    return (r << 16) | (g << 8) | b;
}

uint32_t lighten(uint32_t rgb, int32_t amount) {
    amount = clamp_i32(amount, 0, FX_ONE);
    hsl_t hsl = rgb2hsl(rgb);

    // l = l + (1-l)*amount
    int32_t one_minus_l = FX_ONE - hsl.l;
    hsl.l = hsl.l + fx_mul(one_minus_l, amount);
    hsl.l = clamp_i32(hsl.l, 0, FX_ONE);

    return hsl2rgb(hsl);
}

uint32_t darken(uint32_t rgb, int32_t amount) {
    amount = clamp_i32(amount, 0, FX_ONE);
    hsl_t hsl = rgb2hsl(rgb);

    // l = l * (1-amount)
    hsl.l = fx_mul(hsl.l, FX_ONE - amount);
    hsl.l = clamp_i32(hsl.l, 0, FX_ONE);

    return hsl2rgb(hsl);
}