/*
 * Doki: Quantum Computer simulator, using state vectors. QSimov core.
 * Copyright (C) 2021  Hernán Indíbil de la Cruz Calvo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "platform.h"

COMPLEX_TYPE fix_value(COMPLEX_TYPE a, REAL_TYPE min_r, REAL_TYPE min_i,
		       REAL_TYPE max_r, REAL_TYPE max_i)
{
	double aux_r, aux_i;

	aux_r = RE(a);
	aux_i = IM(a);

	if (aux_r > max_r) {
		aux_r = max_r;
	} else if (aux_r < min_r) {
		aux_r = min_r;
	}

	if (aux_i > max_i) {
		aux_i = max_i;
	} else if (aux_i < min_i) {
		aux_i = min_i;
	}

	return COMPLEX_INIT(aux_r, aux_i);
}

/* log2 from stackoverflow
 * https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
 * written: https://stackoverflow.com/users/944687/desmond-hume
 * edited:
 * https://stackoverflow.com/users/1308473/%c9%b9%c9%90%ca%8e%c9%af%c9%90%ca%9e
 * extra (allign): https://stackoverflow.com/users/267551/todd-lehman
 */
const int8_t ALIGNED_(64) tab64[64] = {
	63, 0,	58, 1,	59, 47, 53, 2,	60, 39, 48, 27, 54, 33, 42, 3,
	61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4,
	62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21,
	56, 45, 25, 31, 35, 16, 9,  12, 44, 24, 15, 8,	23, 7,	6,  5
};

unsigned int log2_64(uint64_t value)
{
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;
	return tab64[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >>
		     58];
}
