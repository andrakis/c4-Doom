//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Fixed point implementation.
//



#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#include "m_fixed.h"




// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul
( fixed_t	a,
  fixed_t	b )
{
#ifdef C4DOOM
    // c4doom: in 32-bit pieces, as Wolf3D's is (docs/c4ix-wolf3d.md, "How
    // fast"): c4lc does int64_t at 32 bits in helper calls, and this is
    // among the hottest calls in the game. With a = ah * 2^16 + al, ah =
    // a >> 16 (arithmetic) and al = a & 0xffff, and b likewise, the
    // product shifted is ah*bh*2^16 + ah*bl + al*bh + (al*bl >> 16)
    // exactly, and unsigned arithmetic keeps its low 32 bits, which is
    // what the cast below keeps of the 64-bit one.
    uint32_t ah = (uint32_t) (a >> FRACBITS), al = (uint32_t) a & 0xffff;
    uint32_t bh = (uint32_t) (b >> FRACBITS), bl = (uint32_t) b & 0xffff;
    return (fixed_t) (((ah * bh) << 16) + ah * bl + al * bh + ((al * bl) >> 16));
#else
    return ((int64_t) a * (int64_t) b) >> FRACBITS;
#endif
}



//
// FixedDiv, C version.
//

fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
    {
	return (a^b) < 0 ? INT_MIN : INT_MAX;
    }
#ifdef C4DOOM
    // c4doom: in 32-bit pieces too (FixedMul, above). Past the test above
    // the quotient fits a word: its whole part is |a| / |b| in the word's
    // own signed division, and its sixteen fraction bits a long division,
    // in one more division when |b| is under 2^15 and in sixteen steps
    // otherwise. Exactly what the 64-bit division gives, truncated towards
    // zero; INT_MIN, whose size a word cannot hold, takes that division.
    else if (a != INT_MIN && b != INT_MIN)
    {
	int ua, ub, q, r, i;
	ua = a < 0 ? -a : a;
	ub = b < 0 ? -b : b;
	q = ua / ub;
	r = ua % ub;
	if (ub <= 0x7fff)
	{
	    q = (q << 16) | ((r << 16) / ub);
	}
	else
	{
	    for (i = 0; i < 16; ++i)
	    {
		// r * 2 >= ub, without making r * 2
		if (r >= ub - r) { r = r - (ub - r); q = (q << 1) | 1; }
		else { r = r + r; q = q << 1; }
	    }
	}
	return (a ^ b) < 0 ? -q : q;
    }
#endif
    else
    {
	int64_t result;

	result = ((int64_t) a << 16) / b;

	return (fixed_t) result;
    }
}

