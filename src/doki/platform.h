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

/** \file platform.h
 *  \brief Functions and macros that may require platform specific stuff.
 *
 *  In this file some functions and macros have been defined.
 */

/** \def __PLATFORM_H
 *  \brief Indicates if platform.h has already been loaded.
 *
 *  If __PLATFORM_H is defined, platform.h file has already been included.
 */

/** \def MALLOC_TYPE(n,type)
 *  \brief A macro that calls malloc.
 *
 *  A macro that calls malloc for n items of specified type.
 */

/** \def CALLOC_TYPE(n,type)
 *  \brief A macro that calls calloc.
 *
 * A macro that calls calloc for n items of specified type.
 */

/** \def REALLOC_TYPE(p,n,type)
 *  \brief A macro that calls realloc.
 *
 * A macro that calls realloc with pointer p for n items of specified type.
 */

/** \def NATURAL_TYPE
 *  \brief Type used for natural numbers.
 *
 *  Type used for natural numbers in Doki. Currently unsigned long long int.
 */

/** \def NATURAL_MAX
 *  \brief Max possible natural number.
 *
 *  The maximum value for a natural number in Doki. Currently ULLONG_MAX.
 */

/** \def NATURAL_ONE
 *  \brief Literal for natural number one.
 *
 *  Number one literal for the specified NATURAL_TYPE. Currently 1ULL.
 */

/** \def COMPLEX_ONE
 *  \brief Literal for complex number one.
 *
 *  Number one literal for the specified COMPLEX_TYPE. Currently
 * complex_init(1, 0).
 */

/** \def COMPLEX_ZERO
 *  \brief Literal for complex number zero.
 *
 *  Number zero literal for the specified COMPLEX_TYPE. Currently
 * complex_init(0, 0).
 */

/** \def CHUNK_MAX
 *  \brief Max number of chunks in a list.
 *
 *  The maximum number of chunks in an ArrayList in Doki. Currently ULONG_MAX.
 */

/** \def REAL_TYPE
 *  \brief Real number type.
 *
 *  Real number type used in Doki. Currently double.
 */

/** \def COMPLEX_TYPE
 *  \brief Complex number type.
 *
 *  Complex number type used in Doki. May vary depending on the compiler.
 */

/** \def COMPLEX_INIT(real, imag);
 *  \brief Build a complex number with specified real and imaginary parts.
 *  \param real The real part as a double.
 *  \param imag The imaginary part as a double.
 *  \return The specified complex number.
 */

/** \def COMPLEX_ADD(a, b);
 *  \brief Calculate a + b, where a and b are complex numbers.
 *  \param a The first operand.
 *  \param b The second operand.
 *  \return The result of a + b.
 */

/** \def COMPLEX_SUB(a, b);
 *  \brief Calculate a - b, where a and b are complex numbers.
 *  \param a The first operand.
 *  \param b The second operand.
 *  \return The result of a - b.
 */

/** \def COMPLEX_MULT(a, b);
 *  \brief Calculate a * b, where a and b are complex numbers.
 *  \param a The first operand.
 *  \param b The second operand.
 *  \return The result of a * b.
 */

/** \def COMPLEX_MULT_R(a, r);
 *  \brief Calculate a * r, where a is a complex number and r is real.
 *  \param a The first operand (complex number).
 *  \param r The second operand (real number).
 *  \return The result of a * r.
 */

/** \def COMPLEX_DIV(res, a, b);
 *  \brief Calculate res <- a / b, where a and b are complex numbers.
 *  \param res The variable that will store the result.
 *  \param a The dividend.
 *  \param b The divisor.
 *  \return The result of a / b.
 */

/** \def COMPLEX_DIV_R(COMPLEX_TYPE a, REAL_TYPE r);
 *  \brief Calculate a / r, where a is a complex number and r is real.
 *  \param a The dividend (complex number).
 *  \param r The divisor (real number).
 *  \return The result of a / r.
 */

/** \fn COMPLEX_TYPE fix_value (COMPLEX_TYPE a);
 *  \brief Modifies a to keep its real and imaginary parts in [-1, 1].
 *  \param a The complex number.
 *  \return A complex number in said intervals.
 */

/** \fn unsigned int log2_64 (uint64_t value);
 *  \brief Calculates the logarithm base 2 of value.
 *  \param a The integer number to calculate its log2.
 *  \return The log2 of value.
 */

#pragma once
#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <complex.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define ALIGNED_(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define ALIGNED_(x) __attribute__((aligned(x)))
#endif
#endif

#define MALLOC_TYPE(n, type) ((type *)malloc((n) * sizeof(type)))
#define CALLOC_TYPE(n, type) ((type *)calloc((n), sizeof(type)))
#define REALLOC_TYPE(p, n, type) ((type *)realloc((p), (n) * sizeof(type)))

#define NATURAL_TYPE intmax_t
#ifdef _MSC_VER
#define NATURAL_STRING_FORMAT "%jd"
#else
#define NATURAL_STRING_FORMAT "%j"
#endif
#define NATURAL_ZERO 0LL
#define NATURAL_ONE 1LL
#define NATURAL_MAX INTMAX_MAX
static const unsigned int NATURAL_BITS = sizeof(NATURAL_TYPE) * 8 - 1;

#define DECIMAL_PLACES 5 // max: 17 (MinGWx64-gcc)
#define DECIMAL_PLACES_S "5" // same as before, but as a string
#define NOTATION                                                                    \
	"g" // f for normal behaviour, e for scientific notation, g for shortest (f \
		// or e)
#define PRECISION 2
#if PRECISION == 1
#define REAL_TYPE float
#ifndef _MSC_VER
#define COMPLEX_TYPE float _Complex
#else
#define COMPLEX_TYPE _Fcomplex
#endif
#define RE crealf
#define IM cimagf
#define ARG cargf
#define COS cosf
#define SIN sinf
#define REAL_STRING_FORMAT "%." DECIMAL_PLACES_S NOTATION
#elif PRECISION == 2
#define REAL_TYPE double
#ifndef _MSC_VER
#define COMPLEX_TYPE double _Complex
#else
#define COMPLEX_TYPE _Dcomplex
#endif
#define RE creal
#define IM cimag
#define ARG carg
#define COS cos
#define SIN sin
#define REAL_STRING_FORMAT "%." DECIMAL_PLACES_S "l" NOTATION
#elif PRECISION == 3
#define REAL_TYPE long double
#ifndef _MSC_VER
#define COMPLEX_TYPE long double _Complex
#else
#define COMPLEX_TYPE _Lcomplex
#endif
#define RE creall
#define IM cimagl
#define ARG cargl
#define COS cosl
#define SIN sinl
#define REAL_STRING_FORMAT "%." DECIMAL_PLACES_S "L" NOTATION
#endif

#ifndef _MSC_VER
static const COMPLEX_TYPE COMPLEX_ZERO = 0;
static const COMPLEX_TYPE COMPLEX_ONE = 1;
static const COMPLEX_TYPE COMPLEX_NAN = NAN;
#else
static const COMPLEX_TYPE COMPLEX_ZERO = { 0, 0 };
static const COMPLEX_TYPE COMPLEX_ONE = { 1, 0 };
static const COMPLEX_TYPE COMPLEX_NAN = { NAN, NAN };
#endif

#define COMPLEX_STRING_FORMAT REAL_STRING_FORMAT "+" REAL_STRING_FORMAT "i"
#define COMPLEX_STRING(c) RE(c), IM(c)
static const unsigned int REAL_BITS = sizeof(REAL_TYPE) * 8;
static const unsigned int COMPLEX_BITS = sizeof(COMPLEX_TYPE) * 8;
static const size_t COMPLEX_ARRAY_SIZE = ((size_t)-1) / sizeof(COMPLEX_TYPE);
static const size_t NATURAL_ARRAY_SIZE = ((size_t)-1) / sizeof(NATURAL_TYPE);
static const size_t COMPLEX_2DARRAY_SIZE =
	((size_t)-1) / sizeof(COMPLEX_TYPE *);
static const size_t NATURAL_2DARRAY_SIZE =
	((size_t)-1) / sizeof(NATURAL_TYPE *);

/*
#define _AUX1_MAX_NUM_QUBITS log2_64(COMPLEX_ARRAY_SIZE) +
log2_64(COMPLEX_2DARRAY_SIZE)
// Indexing with two natural numbers
#define _AUX2_MAX_NUM_QUBITS 2 * NATURAL_BITS
#define MAX_NUM_QUBITS _AUX1_MAX_NUM_QUBITS <= _AUX2_MAX_NUM_QUBITS ?
_AUX1_MAX_NUM_QUBITS : _AUX2_MAX_NUM_QUBITS
*/
#define MAX_NUM_QUBITS log2_64(NATURAL_MAX)

/**
 * Since VS compiler does not fully comply with C99 standard
 * we have to define a coulple of functions regarding complex
 * numbers.
 */
#ifndef _MSC_VER
#define COMPLEX_INIT(real, imag) (real) + I * (imag)
#else
#define COMPLEX_INIT(real, imag) \
	(COMPLEX_TYPE)           \
	{                        \
		real, imag       \
	}
#endif

#ifndef _MSC_VER
#define COMPLEX_ADD(a, b) (a) + (b)
#else
#define COMPLEX_ADD(a, b)                    \
	(COMPLEX_TYPE)                       \
	{                                    \
		RE(a) + RE(b), IM(a) + IM(b) \
	}
#endif

#ifndef _MSC_VER
#define COMPLEX_SUB(a, b) (a) - (b)
#else
#define COMPLEX_SUB(a, b)                    \
	(COMPLEX_TYPE)                       \
	{                                    \
		RE(a) - RE(b), IM(a) - IM(b) \
	}
#endif

#ifndef _MSC_VER
#define COMPLEX_MULT(a, b) (a) * (b)
#else
#define COMPLEX_MULT(a, b)                                                   \
	(COMPLEX_TYPE)                                                       \
	{                                                                    \
		RE(a) * RE(b) - IM(a) * IM(b), RE(a) * IM(b) + RE(b) * IM(a) \
	}
#endif

#ifndef _MSC_VER
#define COMPLEX_MULT_R(a, r) (a) * (r)
#else
#define COMPLEX_MULT_R(a, r)         \
	(COMPLEX_TYPE)               \
	{                            \
		RE(a) * (r), IM(a) * (r) \
	}
#endif

#ifndef _MSC_VER
#define COMPLEX_DIV(res, a, b) res = (a) / (b)
#else
#define COMPLEX_DIV(res, a, b)                                            \
	do {                                                              \
		REAL_TYPE ar = RE(a), ai = IM(a), br = RE(b), bi = IM(b); \
		REAL_TYPE divi = br * br + bi * bi;                       \
		res = (COMPLEX_TYPE){ (ar * br + ai * bi) / divi,         \
				      (ai * br - ar * bi) / divi };       \
	} while (0)
#endif

#ifndef _MSC_VER
#define COMPLEX_DIV_R(a, r) (a) / (r)
#else
#define COMPLEX_DIV_R(a, r)          \
	(COMPLEX_TYPE)               \
	{                            \
		RE(a) / (r), IM(a) / (r) \
	}
#endif

COMPLEX_TYPE
fix_value(COMPLEX_TYPE a, REAL_TYPE min_r, REAL_TYPE min_i, REAL_TYPE max_r,
	  REAL_TYPE max_i);

unsigned int log2_64(uint64_t value);

#endif /* PLATFORM_H_ */
