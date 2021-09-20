#include <stdio.h>
#include "platform.h"

COMPLEX_TYPE
complex_init(double real, double imag) {
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
    COMPLEX_TYPE aux = {creal(a) + creal(b), cimag(a) + cimag(b)};
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
    COMPLEX_TYPE aux = {creal(a) - creal(b), cimag(a) - cimag(b)};
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
    COMPLEX_TYPE aux = {creal(a) * creal(b) - cimag(a) * cimag(b),
                        creal(a) * cimag(b) + creal(b) * cimag(a)};
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
    COMPLEX_TYPE aux = {creal(a) * r, cimag(a) * r};
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

    ar = creal(a);
    ai = cimag(a);
    br = creal(b);
    bi = cimag(b);
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
    COMPLEX_TYPE aux = {creal(a) / r, cimag(a) / r};
    return aux;
    #endif
}

COMPLEX_TYPE
fix_value (COMPLEX_TYPE a) {
    double aux_r, aux_i;

    aux_r = creal(a);
    aux_i = cimag(a);

    if(aux_r > 1.0) {
        aux_r = 1.0;
    }
    else if (aux_r < -1.0) {
        aux_r = -1.0;
    }

    if(aux_i > 1.0) {
        aux_i = 1.0;
    }
    else if (aux_i < -1.0) {
        aux_i = -1.0;
    }

    return complex_init(aux_r, aux_i);
}
