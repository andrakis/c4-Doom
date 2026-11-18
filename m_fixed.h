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
//	Fixed point arithemtics, implementation.
//


#ifndef __M_FIXED__
#define __M_FIXED__




//
// Fixed point, 32bit as 16.16.
//
#define FRACBITS		16
#define FRACUNIT		(1<<FRACBITS)

typedef int fixed_t;

fixed_t FixedMul	(fixed_t a, fixed_t b);
fixed_t FixedDiv	(fixed_t a, fixed_t b);

#if defined(C4DOOM) && defined(__c4_inttypes__)
// c4doom: FixedMul is some 2,000 calls a frame, each about fifty
// instructions with the call; where the machine has Doom's accelerators
// (c4/c4accel.h) it is one, chosen where it stands. m_fixed.c's function,
// c4_FixedMul, is there for every other machine and until the game asks.
#include "c4/c4accel.h"
fixed_t c4_FixedMul(fixed_t a, fixed_t b);
#define FixedMul(a, b) (DOOM_ACCEL() ? doom_fixmul((a), (b)) : c4_FixedMul((a), (b)))
#endif



#endif
