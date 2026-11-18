/* c4accel.h -- c4's port: Doom's accelerators, c4/accel.mjs, from the
   game's side (c4's libjs/guest/accel.h, docs/accel.md). On a machine that
   has them, with the Options menu's ACCELERATED on, the host runs these in
   the game's own JavaScript, one instruction each; elsewhere, and in gcc's
   build, the C runs as before. doom_accel is the first of Doom's numbers
   (0 for none, -1 until asked: r_draw.c); DOOM_ACCEL() asks once. The
   numbers are the module's operations in its order. */
#ifndef C4ACCEL_H
#define C4ACCEL_H

enum { DA_COL, DA_FUZZ, DA_SPAN, DA_SUM, DA_FIXMUL, DA_FIXDIV, DA_PATCH };

extern int doom_accel;
int doom_accel_base(void);
int doom_accel_avail(void);      /* the machine has them */
void doom_accel_set(int on);     /* the menu's switch */
extern int doom_accel_on;        /* ACCELERATED, c4_accelerated in the config */
#define DOOM_ACCEL() (doom_accel >= 0 ? doom_accel : doom_accel_base())

#ifdef __c4_inttypes__
#include "accel.h"
#define DA_OP(op) (doom_accel + (op))
/* a column: *dest = cmap[src[(frac >> 16) & mask]], through trans unless
   0, and at dest + d2 too unless d2 is 0 (low detail) */
#define doom_col(n, frac, step, mask, src, cmap, trans, dest, stride, d2) \
	__c4_opcode((n), (frac), (step), (mask), (src), (cmap), (trans), (dest), (stride), (d2), DA_OP(DA_COL))
/* the shadow: *dest = cmap[dest[fuzzofs[*pfuzzpos]]], *pfuzzpos round size */
#define doom_fuzz(n, dest, stride, cmap, fuzzofs, pfuzzpos, size, d2) \
	__c4_opcode((n), (dest), (stride), (cmap), (fuzzofs), (pfuzzpos), (size), (d2), DA_OP(DA_FUZZ))
/* a flat's row, each pixel twice for dbl */
#define doom_span(n, position, step, src, cmap, dest, dbl) \
	__c4_opcode((n), (position), (step), (src), (cmap), (dest), (dbl), DA_OP(DA_SPAN))
/* dg-sum.c's checksum of a frame and its palette, from s */
#define doom_sum(s, pal, pixels) __c4_opcode((s), (pal), (pixels), DA_OP(DA_SUM))
/* m_fixed.c's FixedMul, and FixedDiv for b not 0 */
#define doom_fixmul(a, b) __c4_opcode((a), (b), DA_OP(DA_FIXMUL))
#define doom_fixdiv(a, b) __c4_opcode((a), (b), DA_OP(DA_FIXDIV))
/* v_video.c's V_DrawPatch and V_DrawPatchFlipped, the patch's columns */
#define doom_patch(patch, desttop, stride, w, flip) \
	__c4_opcode((patch), (desttop), (stride), (w), (flip), DA_OP(DA_PATCH))
#else
#define accel_find(name) 0
#define doom_col(n, frac, step, mask, src, cmap, trans, dest, stride, d2) 0
#define doom_fuzz(n, dest, stride, cmap, fuzzofs, pfuzzpos, size, d2) 0
#define doom_span(n, position, step, src, cmap, dest, dbl) 0
#define doom_sum(s, pal, pixels) 0
#define doom_fixmul(a, b) 0
#define doom_fixdiv(a, b) 0
#define doom_patch(patch, desttop, stride, w, flip) 0
#endif

#endif
