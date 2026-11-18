// accel.mjs -- Doom's accelerators for c4's libjs (its libjs/accel.js,
// docs/accel.md): the renderer's innermost loops, the patches' copy, the
// timedemo's checksum and m_fixed.c's FixedMul and FixedDiv, run by the
// host when the machine has them and the Options menu's ACCELERATED is on.
// c4/c4accel.h is the game's side, with each operation's number (DA_*);
// r_draw.c, v_video.c, dg-sum.c and m_fixed.c call them, and keep their own
// C for every other machine. Each does exactly what that C does: the same
// pixels in the same order, 32-bit arithmetic wrapping, shifts as c4's SHR
// and LSR take them.
export default function doom({ reader, writer, reader32, writer32, probe, span, args }) {
  // DA_COL (n, frac, step, mask, src, cmap, trans, dest, stride, d2): a
  // column, R_DrawColumn, R_DrawColumnLow, R_DrawTranslatedColumn and
  // R_DrawTranslatedColumnLow: *dest = cmap[src[(frac >> 16) & mask]],
  // through trans[] first when trans is not 0; dest += stride; frac +=
  // step. d2 not 0: the low detail's second column, dest + d2, the same.
  function col(m) {
    let [n, frac, step, mask, src, cmap, trans, dest, stride, d2] = args(m, 10);
    const rs = reader(m), rt = reader(m), rc = reader(m), wr = writer(m);
    for (; n > 0; n--, dest = (dest + stride) | 0, frac = (frac + step) | 0) {
      let t = rs((src + ((frac >> 16) & mask)) | 0);
      if (trans) t = rt((trans + t) | 0);
      const c = rc((cmap + t) | 0);
      wr(dest, c);
      if (d2) wr((dest + d2) | 0, c);
    }
    return 0;
  }

  // DA_FUZZ (n, dest, stride, cmap, fuzzofs, pfuzzpos, size, d2): the
  // shadow, R_DrawFuzzColumn and R_DrawFuzzColumnLow: *dest =
  // cmap[dest[fuzzofs[fuzzpos]]], fuzzpos round its table of size, left in
  // *pfuzzpos. The pixels it reads are a row above or below, which it may
  // have written: every pixel is worked out first, as the loop would have
  // it, and the pages it writes made sure of (probe) before the first is
  // written, so that a run again after a fault reads the screen as it was.
  function fuzz(m) {
    const [n, dest, stride, cmap, fofs, pfp, size, d2] = args(m, 8);
    if (n <= 0) return 0;
    const rd = reader(m), rc = reader(m), r32 = reader32(m);
    const pend = new Map(), wa = [], wv = [];
    const get = a => (pend.has(a) ? pend.get(a) : rd(a));
    const put = (a, v) => { pend.set(a, v); wa.push(a); wv.push(v); };
    let fp = r32(pfp), d = dest;
    for (let i = 0; i < n; i++, d = (d + stride) | 0) {
      const off = r32((fofs + 4 * fp) | 0);
      put(d, rc((cmap + get((d + off) | 0)) | 0));
      if (d2) { const e = (d + d2) | 0; put(e, rc((cmap + get((e + off) | 0)) | 0)); }
      if (++fp === size) fp = 0;
    }
    let page = -1;
    for (const a of wa) if ((a >>> 12) !== page) { probe(m, a, 1, true); page = a >>> 12; }
    probe(m, pfp, 4, true);
    const wr = writer(m);
    for (let i = 0; i < wa.length; i++) wr(wa[i], wv[i]);
    writer32(m)(pfp, fp);
    return 0;
  }

  // DA_SPAN (n, position, step, src, cmap, dest, dbl): a floor's or
  // ceiling's row, R_DrawSpan and R_DrawSpanLow, out of the 64x64 flat:
  // *dest++ = cmap[src[((position >> 4) & 0xfc0) | (position >> 26)]],
  // twice for dbl (low detail); position += step, unsigned
  function spanop(m) {
    let [n, pos, step, src, cmap, dest, dbl] = args(m, 7);
    const rs = reader(m), rc = reader(m), wr = writer(m);
    for (; n > 0; n--, pos = (pos + step) | 0) {
      const c = rc((cmap + rs((src + (((pos >>> 4) & 0xfc0) | (pos >>> 26))) | 0)) | 0);
      wr(dest, c); dest = (dest + 1) | 0;
      if (dbl) { wr(dest, c); dest = (dest + 1) | 0; }
    }
    return 0;
  }

  // DA_SUM (s, pal, pixels): dg-sum.c's checksum of a timedemo's frame, s
  // carried on: s = s * 31 + each of the palette's 256 words, then each of
  // the 320x200 frame's 16,000 words as they lie
  function sum(m) {
    let [s, pal, pix] = args(m, 3);
    const r32 = reader32(m);
    for (let i = 0; i < 256; i++) s = (Math.imul(s, 31) + r32((pal + 4 * i) | 0)) | 0;
    const N = 320 * 200;
    for (let i = 0; i < N; ) {
      // a page at a time where the frame is one run of the arena's bytes
      const at = (pix + i) | 0, k = Math.min(N - i, 4096 - (at & 4095));
      const v = span(m, at, k, false);
      if (v && (k & 3) === 0) {
        for (let j = 0; j < k; j += 4)
          s = (Math.imul(s, 31) + (v[j] | (v[j + 1] << 8) | (v[j + 2] << 16) | (v[j + 3] << 24))) | 0;
        i += k;
      } else {
        s = (Math.imul(s, 31) + r32(at)) | 0;
        i += 4;
      }
    }
    return s;
  }

  // DA_FIXMUL (a, b): FixedMul, ((int64_t)a * b) >> 16, its low word, as
  // m_fixed.c's 32-bit pieces make it
  function fixmul(m) {
    const [a, b] = args(m, 2);
    const ah = a >> 16, al = a & 0xffff, bh = b >> 16, bl = b & 0xffff;
    return ((Math.imul(ah, bh) << 16) + Math.imul(ah, bl) + Math.imul(al, bh) + ((al * bl) >>> 16)) | 0;
  }

  // DA_FIXDIV (a, b), b not 0: FixedDiv, INT_MAX or INT_MIN where
  // abs(a) >> 14 >= abs(b) (abs as c4's, -INT_MIN being INT_MIN), else
  // ((int64_t)a << 16) / b toward zero, its low word
  const abs32 = x => (x < 0 ? (-x) | 0 : x);
  function fixdiv(m) {
    const [a, b] = args(m, 2);
    if ((abs32(a) >> 14) >= abs32(b)) return (a ^ b) < 0 ? 0x80000000 | 0 : 0x7fffffff;
    return Number(BigInt.asIntN(32, (BigInt(a) << 16n) / BigInt(b))) | 0;
  }

  // DA_PATCH (patch, desttop, stride, w, flip): V_DrawPatch's and
  // V_DrawPatchFlipped's columns, a patch's posts copied down from desttop,
  // column by column (from the last for flip)
  function patch(m) {
    let [p, desttop, stride, w, flip] = args(m, 5);
    const rp = reader(m), rs = reader(m), r32 = reader32(m), wr = writer(m);
    for (let col = 0; col < w; col++, desttop = (desttop + 1) | 0) {
      let column = (p + r32((p + 8 + 4 * (flip ? w - 1 - col : col)) | 0)) | 0;
      let td;
      while ((td = rp(column)) !== 0xff) {
        const len = rp((column + 1) | 0);
        let src = (column + 3) | 0, dest = (desttop + Math.imul(td, stride)) | 0;
        for (let c = len; c > 0; c--, src = (src + 1) | 0, dest = (dest + stride) | 0) wr(dest, rs(src));
        column = (column + len + 4) | 0;
      }
    }
    return 0;
  }

  return [col, fuzz, spanop, sum, fixmul, fixdiv, patch];
}
