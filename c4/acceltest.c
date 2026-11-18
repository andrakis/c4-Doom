// acceltest.c -- c4's port: Doom's accelerators (c4/accel.mjs) against the
// loops they stand for, each as r_draw.c, v_video.c, dg-sum.c and m_fixed.c
// have them: the same pixels, on made-up textures, colormaps, flats,
// patches and steps, the same checksums, and the same FixedMul and
// FixedDiv as the 32-bit pieces and the 64-bit arithmetic give. test-doom
// runs it on c4's libjs.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "c4accel.h"

int doom_accel = -1;
int doom_accel_on = 1;
int doom_accel_base(void) { return doom_accel = accel_find("doom"); }

unsigned seed = 12345;
int rnd(int n) { seed = seed * 1103515245 + 12345; return (int)((seed >> 8) % (unsigned)n); }

unsigned char *tex, *cmap, *trans, *bufc, *bufa, *base;
int fails, cases;
enum { TEXN = 65536 * 2, W = 320, H = 200, BUFN = W * H, FUZZTABLE = 50 };

void fill(unsigned char *p, int n) { int i; for (i = 0; i < n; i++) p[i] = rnd(256); }
void check(char *what, int k) {
    cases++;
    if (memcmp(bufc, bufa, BUFN) != 0) {
        fails++;
        if (fails < 10) printf("accel: %s, case %d, differs\n", what, k);
    }
}
// the screen as it was, made up once: the cases start from it, a word of
// it changed
void clear() { base[rnd(BUFN)] = rnd(256); memcpy(bufc, base, BUFN); memcpy(bufa, base, BUFN); }

// R_DrawColumn and R_DrawColumnLow (mask 127), R_DrawTranslatedColumn and
// its low one (trans, no mask), the source's middle the middle of tex
void colc(int count, int frac, int step, int mask, unsigned char *src, unsigned char *cm,
          unsigned char *tr, unsigned char *dest, unsigned char *dest2) {
    do {
        if (tr) *dest = cm[tr[src[frac >> 16]]];
        else *dest = cm[src[(frac >> 16) & 127]];
        if (dest2) *dest2 = *dest;
        dest += W; if (dest2) dest2 += W;
        frac += step;
    } while (count--);
}
void test_col() {
    int k, count, frac, step, x, y, low, tr;
    unsigned char *src;
    for (k = 0; k < 800; k++) {
        clear();
        low = k & 1; tr = (k >> 1) & 1;
        y = rnd(H); count = rnd(H - y); x = rnd(W - 1);
        src = tex + 65536;
        step = rnd(1 << 18);
        frac = tr ? rnd(1 << 26) - (1 << 25) - step * count / 2 : (int)((unsigned)rnd(1 << 30) * 4 + rnd(4));
        if (tr && (frac + step * count) >> 16 >= 32768) frac = 0;
        colc(count, frac, step, 127, src, cmap, tr ? trans : 0, bufc + x + y * W, low ? bufc + x + 1 + y * W : 0);
        doom_col(count + 1, frac, step, tr ? -1 : 127, src, cmap, tr ? trans : 0, bufa + x + y * W, W, low);
        check(tr ? (low ? "translated column, low" : "translated column") : (low ? "column, low" : "column"), k);
    }
}

// R_DrawFuzzColumn and its low one: rows 1 to H - 2, so that a row above
// and below are the screen's
int fuzzoffset[FUZZTABLE] = {
    W,-W,W,-W,W,W,-W, W,W,-W,W,W,W,-W, W,W,W,-W,-W,-W,-W, W,-W,-W,W,W,W,W,-W,
    W,-W,W,W,-W,-W,W, W,-W,-W,-W,-W,W,W, W,W,-W,W,W,-W,W };
int fuzzc, fuzza;
void fuzzcc(int count, unsigned char *dest, unsigned char *dest2) {
    do {
        *dest = cmap[dest[fuzzoffset[fuzzc]]];
        if (dest2) *dest2 = cmap[dest2[fuzzoffset[fuzzc]]];
        if (++fuzzc == FUZZTABLE) fuzzc = 0;
        dest += W; if (dest2) dest2 += W;
    } while (count--);
}
void test_fuzz() {
    int k, count, x, y, low;
    for (k = 0; k < 400; k++) {
        clear();
        low = k & 1;
        y = 1 + rnd(H - 2); count = rnd(H - 1 - y); x = rnd(W - 1);
        fuzzc = fuzza = rnd(FUZZTABLE);
        fuzzcc(count, bufc + x + y * W, low ? bufc + x + 1 + y * W : 0);
        doom_fuzz(count + 1, bufa + x + y * W, W, cmap, fuzzoffset, &fuzza, FUZZTABLE, low);
        check(low ? "fuzz, low" : "fuzz", k);
        cases++;
        if (fuzzc != fuzza) { fails++; if (fails < 10) printf("accel: fuzz, case %d, fuzzpos %d, not %d\n", k, fuzza, fuzzc); }
    }
}

// R_DrawSpan and R_DrawSpanLow
void spanc(int count, unsigned position, unsigned step, unsigned char *src, unsigned char *dest, int dbl) {
    unsigned xtemp, ytemp;
    int spot;
    do {
        ytemp = (position >> 4) & 0x0fc0;
        xtemp = (position >> 26);
        spot = xtemp | ytemp;
        *dest++ = cmap[src[spot]];
        if (dbl) *dest++ = cmap[src[spot]];
        position += step;
    } while (count--);
}
void test_span() {
    int k, count, x, y, dbl;
    unsigned pos, step;
    for (k = 0; k < 800; k++) {
        clear();
        dbl = k & 1;
        y = rnd(H);
        if (dbl) { x = rnd(W / 2); count = rnd(W / 2 - x); x *= 2; }
        else { x = rnd(W); count = rnd(W - x); }
        pos = (unsigned)rnd(1 << 30) * 4 + rnd(4);
        step = (unsigned)rnd(1 << 30) * 4 + rnd(4);
        if (k % 3 == 0) step = ((unsigned)(rnd(1 << 14) << 16)) | rnd(1 << 14);
        spanc(count, pos, step, tex + 1000, bufc + x + y * W, dbl);
        doom_span(count + 1, pos, step, tex + 1000, cmap, bufa + x + y * W, dbl);
        check(dbl ? "span, low" : "span", k);
    }
}

// dg-sum.c's checksum
int pal[256];
void test_sum() {
    int k, i;
    uint32_t s, c, a;
    for (k = 0; k < 40; k++) {
        fill(bufc, BUFN);
        for (i = 0; i < 256; i++) pal[i] = rnd(1 << 24);
        s = (uint32_t)rnd(1 << 30) * 4;
        c = s;
        for (i = 0; i < 256; i++) c = c * 31 + (uint32_t)pal[i];
        for (i = 0; i < 320 * 200; i += 4)
            c = c * 31 + (uint32_t)(bufc[i] | (bufc[i + 1] << 8) | (bufc[i + 2] << 16) | ((uint32_t)bufc[i + 3] << 24));
        a = (uint32_t)doom_sum(s, pal, bufc);
        cases++;
        if (a != c) { fails++; if (fails < 10) printf("accel: sum, case %d, %08x, not %08x\n", k, (unsigned)a, (unsigned)c); }
    }
}

// v_video.c's V_DrawPatch and V_DrawPatchFlipped, on a made-up patch:
// width w, columnofs[w], each column's posts in order down
unsigned char *patch;
void patchc(unsigned char *p, unsigned char *desttop, int w, int flip) {
    int col, count, ofs;
    unsigned char *column, *source, *dest;
    for (col = 0; col < w; col++, desttop++) {
        ofs = p[8 + 4 * (flip ? w - 1 - col : col)] | (p[9 + 4 * (flip ? w - 1 - col : col)] << 8);
        column = p + ofs;
        while (column[0] != 0xff) {
            source = column + 3;
            dest = desttop + column[0] * W;
            count = column[1];
            while (count--) { *dest = *source++; dest += W; }
            column = column + column[1] + 4;
        }
    }
}
int makepatch(int w, int h) {
    int col, at, top, len, *ofs;
    at = 8 + 4 * w;
    ofs = (int *)(patch + 8);
    for (col = 0; col < w; col++) {
        ofs[col] = at;
        top = rnd(4);
        while (top < h) {
            len = 1 + rnd(h - top);
            patch[at] = top; patch[at + 1] = len; patch[at + 2] = rnd(256);
            fill(patch + at + 3, len);
            patch[at + 3 + len] = rnd(256);
            at += len + 4;
            top += len + rnd(6);
        }
        patch[at++] = 0xff;
    }
    return at;
}
void test_patch() {
    int k, w, h, x, y, flip;
    for (k = 0; k < 300; k++) {
        clear();
        flip = k & 1;
        w = 1 + rnd(64); h = 1 + rnd(64);
        makepatch(w, h);
        x = rnd(W - w + 1); y = rnd(H - h + 1);
        patchc(patch, bufc + x + y * W, w, flip);
        doom_patch(patch, bufa + x + y * W, W, w, flip);
        check(flip ? "patch, flipped" : "patch", k);
    }
}

// m_fixed.c's FixedMul and FixedDiv, as the port has them, and as the
// 64-bit arithmetic does
int fixmulc(int a, int b) {
    uint32_t ah = (uint32_t) (a >> 16), al = (uint32_t) a & 0xffff;
    uint32_t bh = (uint32_t) (b >> 16), bl = (uint32_t) b & 0xffff;
    return (int) (((ah * bh) << 16) + ah * bl + al * bh + ((al * bl) >> 16));
}
int fixdivc(int a, int b) {
    if ((abs(a) >> 14) >= abs(b)) return (a ^ b) < 0 ? INT_MIN : INT_MAX;
    if (a != INT_MIN && b != INT_MIN) {
        int ua, ub, q, r, i;
        ua = a < 0 ? -a : a;
        ub = b < 0 ? -b : b;
        q = ua / ub;
        r = ua % ub;
        if (ub <= 0x7fff) q = (q << 16) | ((r << 16) / ub);
        else for (i = 0; i < 16; ++i) {
            if (r >= ub - r) { r = r - (ub - r); q = (q << 1) | 1; }
            else { r = r + r; q = q << 1; }
        }
        return (a ^ b) < 0 ? -q : q;
    }
    return (int) (((int64_t) a << 16) / b);
}
int pick(int k) {
    switch (rnd(7)) {
    case 0: return rnd(65536) - 32768;
    case 1: return (int)((unsigned)rnd(1 << 30) * 4 + rnd(4));
    case 2: return k & 1 ? INT_MAX : INT_MIN;
    case 3: return rnd(1 << 24) - (1 << 23);
    case 4: return rnd(3) - 1;
    case 5: return (rnd(1 << 16) - (1 << 15)) << 16;
    }
    return rnd(1 << 30) - (1 << 29);
}
void bad(char *what, int a, int b, int r, int w) {
    fails++;
    if (fails < 10) printf("accel: %s %d %d: %d, not %d\n", what, a, b, r, w);
}
void test_math() {
    int k, a, b, r, w;
    for (k = 0; k < 4000; k++) {
        a = pick(k); b = pick(k + 1);
        r = doom_fixmul(a, b); w = fixmulc(a, b);
        cases++; if (r != w) bad("fixmul", a, b, r, w);
        w = (int)((((int64_t)a * (int64_t)b) >> 16) & 0xffffffff);
        cases++; if (r != w) bad("fixmul (64-bit)", a, b, r, w);
        if (b == 0) continue;
        r = doom_fixdiv(a, b); w = fixdivc(a, b);
        cases++; if (r != w) bad("fixdiv", a, b, r, w);
    }
}

int main() {
    if (DOOM_ACCEL() <= 0) { printf("accel: Doom's are not here\n"); return 0; }
    tex = malloc(TEXN); cmap = malloc(256); trans = malloc(256);
    bufc = malloc(BUFN); bufa = malloc(BUFN); base = malloc(BUFN); patch = malloc(65536);
    fill(base, BUFN); fill(tex, TEXN); fill(cmap, 256); fill(trans, 256);
    test_col(); test_fuzz(); test_span(); test_sum(); test_patch(); test_math();
    if (fails) printf("accel: %d of %d cases differ\n", fails, cases);
    else printf("accel: Doom's, %d cases, each as its loop has it\n", cases);
    return fails != 0;
}
