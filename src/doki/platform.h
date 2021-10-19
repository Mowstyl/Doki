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
 *  Number one literal for the specified COMPLEX_TYPE. Currently complex_init(1, 0).
 */

/** \def COMPLEX_ZERO
 *  \brief Literal for complex number zero.
 *
 *  Number zero literal for the specified COMPLEX_TYPE. Currently complex_init(0, 0).
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

/** \fn COMPLEX_TYPE complex_init(double real, double imag);
 *  \brief Build a complex number with specified real and imaginary parts.
 *  \param real The real part as a double.
 *  \param imag The imaginary part as a double.
 *  \return The specified complex number.
 */

/** \fn COMPLEX_TYPE complex_sum(COMPLEX_TYPE a, COMPLEX_TYPE b);
 *  \brief Calculate a + b, where a and b are complex numbers.
 *  \param a The first operand.
 *  \param b The second operand.
 *  \return The result of a + b.
 */

/** \fn COMPLEX_TYPE complex_sub(COMPLEX_TYPE a, COMPLEX_TYPE b);
 *  \brief Calculate a - b, where a and b are complex numbers.
 *  \param a The first operand.
 *  \param b The second operand.
 *  \return The result of a - b.
 */

/** \fn COMPLEX_TYPE complex_mult(COMPLEX_TYPE a, COMPLEX_TYPE b);
 *  \brief Calculate a * b, where a and b are complex numbers.
 *  \param a The first operand.
 *  \param b The second operand.
 *  \return The result of a * b.
 */

/** \fn COMPLEX_TYPE complex_mult_r(COMPLEX_TYPE a, REAL_TYPE r);
 *  \brief Calculate a * r, where a is a complex number and r is real.
 *  \param a The first operand (complex number).
 *  \param b The second operand (real number).
 *  \return The result of a * r.
 */

/** \fn COMPLEX_TYPE complex_div_r(COMPLEX_TYPE a, COMPLEX_TYPE b);
 *  \brief Calculate a / b, where a and b are complex numbers.
 *  \param a The dividend.
 *  \param r The divisor.
 *  \return The result of a / b.
 */

/** \fn COMPLEX_TYPE complex_div_r(COMPLEX_TYPE a, REAL_TYPE r);
 *  \brief Calculate a / r, where a is a complex number and r is real.
 *  \param a The dividend.
 *  \param r The divisor.
 *  \return The result of a / r.
 */

/** \fn COMPLEX_TYPE fix_value (COMPLEX_TYPE a);
 *  \brief Modifies a to keep its real and imaginary parts in [-1, 1].
 *  \param a The complex number.
 *  \return A complex number in said intervals.
 */

#pragma once
#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <stdlib.h>
#include <stdint.h>
#include <complex.h>
#include <limits.h>

#if defined(_MSC_VER)
	#define ALIGNED_(x) __declspec(align(x))
#else
	#if defined(__GNUC__)
		#define ALIGNED_(x) __attribute__ ((aligned(x)))
	#endif
#endif

#define MALLOC_TYPE(n,type) ((type *) malloc((n) * sizeof(type)))
#define CALLOC_TYPE(n,type) ((type *) calloc((n), sizeof(type)))
#define REALLOC_TYPE(p,n,type) ((type *) realloc((p), (n) * sizeof(type)))

#define NATURAL_TYPE long long int
#define NATURAL_STRING_FORMAT "%lld"
static const NATURAL_TYPE NATURAL_ZERO = 0;
static const NATURAL_TYPE NATURAL_ONE = 1;
static const NATURAL_TYPE NATURAL_MAX = LLONG_MAX;
static const unsigned int NATURAL_BITS = sizeof(NATURAL_TYPE) * 8 - 1;

#define DECIMAL_PLACES 5 // max: 17 (MinGWx64-gcc)
#define DECIMAL_PLACES_S "5" // same as before, but as a string
#define NOTATION "g" // f for normal behaviour, e for scientific notation, g for shortest (f or e)
#define PRECISION 2
#if PRECISION==1
	#define REAL_TYPE float
	#ifndef _WIN32
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
#elif PRECISION==2
	#define REAL_TYPE double
	#ifndef _WIN32
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
#elif PRECISION==3
	#define REAL_TYPE long double
	#ifndef _WIN32
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

#ifndef _WIN32
	static const COMPLEX_TYPE COMPLEX_ZERO = 0;
	static const COMPLEX_TYPE COMPLEX_ONE = 1;
#else
	static const COMPLEX_TYPE COMPLEX_ZERO = { 0, 0 };
	static const COMPLEX_TYPE COMPLEX_ONE = { 1, 0 };
#endif

#define COMPLEX_STRING_FORMAT REAL_STRING_FORMAT "+" REAL_STRING_FORMAT "i"
#define COMPLEX_STRING(c) RE(c), IM(c)
static const unsigned int REAL_BITS = sizeof(REAL_TYPE) * 8;
static const unsigned int COMPLEX_BITS = sizeof(COMPLEX_TYPE) * 8;
static const size_t COMPLEX_ARRAY_SIZE = ((size_t)-1) / sizeof(COMPLEX_TYPE);
static const size_t NATURAL_ARRAY_SIZE = ((size_t)-1) / sizeof(NATURAL_TYPE);
static const size_t COMPLEX_2DARRAY_SIZE = ((size_t)-1) / sizeof(COMPLEX_TYPE*);
static const size_t NATURAL_2DARRAY_SIZE = ((size_t)-1) / sizeof(NATURAL_TYPE*);

/*
#define _AUX1_MAX_NUM_QUBITS log2_64(COMPLEX_ARRAY_SIZE) + log2_64(COMPLEX_2DARRAY_SIZE)
// Indexing with two natural numbers
#define _AUX2_MAX_NUM_QUBITS 2 * NATURAL_BITS
#define MAX_NUM_QUBITS _AUX1_MAX_NUM_QUBITS <= _AUX2_MAX_NUM_QUBITS ? _AUX1_MAX_NUM_QUBITS : _AUX2_MAX_NUM_QUBITS
*/
#define MAX_NUM_QUBITS log2_64(NATURAL_MAX)


/**
 * complex_init
 */
COMPLEX_TYPE
complex_init(REAL_TYPE real, REAL_TYPE imag);

COMPLEX_TYPE
complex_sum(COMPLEX_TYPE a, COMPLEX_TYPE b);

COMPLEX_TYPE
complex_sub(COMPLEX_TYPE a, COMPLEX_TYPE b);

COMPLEX_TYPE
complex_mult(COMPLEX_TYPE a, COMPLEX_TYPE b);

COMPLEX_TYPE
complex_mult_r(COMPLEX_TYPE a, REAL_TYPE r);

COMPLEX_TYPE
complex_div(COMPLEX_TYPE a, COMPLEX_TYPE b);

COMPLEX_TYPE
complex_div_r(COMPLEX_TYPE a, REAL_TYPE r);

COMPLEX_TYPE
fix_value(COMPLEX_TYPE a, REAL_TYPE min_r, REAL_TYPE min_i, REAL_TYPE max_r, REAL_TYPE max_i);

unsigned int
log2_64(uint64_t value);

#endif
