#include "platform.h"

COMPLEX_TYPE
complex_init(REAL_TYPE real, REAL_TYPE imag) {
    #ifndef _MSC_VER
    // When not using VS compiler we can use this
    return real + I * imag;
    #else
    // When using VS compiler we need an aux variable
    COMPLEX_TYPE aux = {real, imag};
    return aux;
    #endif
}

COMPLEX_TYPE
complex_sum(COMPLEX_TYPE a, COMPLEX_TYPE b) {
    #ifndef _MSC_VER
    // When not using VS compiler, addition is a native operation
    return a + b;
    #else
    // Since VS compiler does not fully comply with C99 standard
    // we have to define complex number addition
    COMPLEX_TYPE aux = {RE(a) + RE(b), IM(a) + IM(b)};
    return aux;
    #endif
}

COMPLEX_TYPE
complex_sub(COMPLEX_TYPE a, COMPLEX_TYPE b) {
    #ifndef _MSC_VER
    // When not using VS compiler, addition is a native operation
    return a - b;
    #else
    // Since VS compiler does not fully comply with C99 standard
    // we have to define complex number addition
    COMPLEX_TYPE aux = {RE(a) - RE(b), IM(a) - IM(b)};
    return aux;
    #endif
}

COMPLEX_TYPE
complex_mult(COMPLEX_TYPE a, COMPLEX_TYPE b) {
    #ifndef _MSC_VER
    // When not using VS compiler, product is a native operation
    return a * b;
    #else
    // Since VS compiler does not fully comply with C99 standard
    // we have to define complex number product
    COMPLEX_TYPE aux = {RE(a) * RE(b) - IM(a) * IM(b),
                        RE(a) * IM(b) + RE(b) * IM(a)};
    return aux;
    #endif
}

COMPLEX_TYPE
complex_mult_r(COMPLEX_TYPE a, REAL_TYPE r) {
    #ifndef _MSC_VER
    // When not using VS compiler, product is a native operation
    return a * r;
    #else
    // Since VS compiler does not fully comply with C99 standard
    // we have to define complex number product
    COMPLEX_TYPE aux = {RE(a) * r, IM(a) * r};
    return aux;
    #endif
}

COMPLEX_TYPE
complex_div(COMPLEX_TYPE a, COMPLEX_TYPE b) {
    #ifndef _MSC_VER
    // When not using VS compiler, product is a native operation
    return a / b;
    #else
    // Since VS compiler does not fully comply with C99 standard
    // we have to define complex number product
    REAL_TYPE ar, ai, br, bi;
    REAL_TYPE divi;

    ar = RE(a);
    ai = IM(a);
    br = RE(b);
    bi = IM(b);
    divi = br * br + bi * bi;
    COMPLEX_TYPE aux = {(ar * br + ai * bi) / divi, (ai * br - ar * bi) / divi};
    return aux;
    #endif
}

COMPLEX_TYPE
complex_div_r(COMPLEX_TYPE a, REAL_TYPE r) {
    #ifndef _MSC_VER
    // When not using VS compiler, product is a native operation
    return a / r;
    #else
    // Since VS compiler does not fully comply with C99 standard
    // we have to define complex number product
    COMPLEX_TYPE aux = {RE(a) / r, IM(a) / r};
    return aux;
    #endif
}

COMPLEX_TYPE
fix_value(COMPLEX_TYPE a, REAL_TYPE min_r, REAL_TYPE min_i, REAL_TYPE max_r, REAL_TYPE max_i) {
    double aux_r, aux_i;

    aux_r = RE(a);
    aux_i = IM(a);

    if (aux_r > max_r) {
        aux_r = max_r;
    }
    else if (aux_r < min_r) {
        aux_r = min_r;
    }

    if (aux_i > max_i) {
        aux_i = max_i;
    }
    else if (aux_i < min_i) {
        aux_i = min_i;
    }

    return complex_init(aux_r, aux_i);
}

/* log2 from stackoverflow
 * https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
 * written: https://stackoverflow.com/users/944687/desmond-hume
 * edited: https://stackoverflow.com/users/1308473/%c9%b9%c9%90%ca%8e%c9%af%c9%90%ca%9e
 * extra (allign): https://stackoverflow.com/users/267551/todd-lehman
 */
const int8_t ALIGNED_(64) tab64[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5 };

unsigned int
log2_64(uint64_t value) {
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return tab64[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
}
