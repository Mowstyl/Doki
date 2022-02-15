#ifndef __FUNMATRIX_H
#define __FUNMATRIX_H

#include <complex.h>
#include "platform.h"

struct FMatrix
{
    /* Number of rows */
    NATURAL_TYPE     r;
    /* Number of columns */
    NATURAL_TYPE     c;
    /* Function that, given (i, j, nrows, ncolumns, *argv)
    returns the value of the element (i, j) of
    the matrix */
    COMPLEX_TYPE   (*f)(NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, void*);
    /* Scalar number s that will be multiplied by the result of f(i, j) or multiplied by A op B */
    COMPLEX_TYPE     s;
    /* Pointer to matrix A in case an operation is going to be performed A op B */
    struct FMatrix  *A;
    /* Pointer to matrix B in case an operation is going to be performed A op B */
    struct FMatrix  *B;
    /* Operation to apply between the matrices.
        0 -> Matrix addition               A + B
        1 -> Matrix subtraction            A - B
        2 -> Matrix multiplication         A * B
        3 -> Entity-wise multiplication    A.* B
        4 -> Kronecker product             A⊗ B
    */
    short            op;
    /* Whether the matrix has to be transposed or not */
    short            transpose;
    /* Whether the matrix has to be complex conjugated or not */
    short            conjugate;
    /* Whether the matrix is simple or you have to perform an operation */
    short            simple;
    /* Extra arguments to pass to the function f */
    void            *argv;
};

typedef struct FMatrix FunctionalMatrix;

struct DMatrixForTrace
{
    /* Density Matrix */
    FunctionalMatrix *m;
    /* Element to trace out */
    int               e;
};

typedef struct DMatrixForTrace _MatrixElem;

/* Constructor */
FunctionalMatrix*
new_FunctionalMatrix(NATURAL_TYPE n_rows, NATURAL_TYPE n_columns,
                     COMPLEX_TYPE (*fun)(NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, void*),
                     void *argv);

/*
 * Get the element (i, j) from the matrix a, and return the result in
 * the address pointed by sol. If a 0 is returned, something went wrong.
 */
int
getitem(FunctionalMatrix *a, NATURAL_TYPE i, NATURAL_TYPE j, COMPLEX_TYPE *sol);

/* Addition */
FunctionalMatrix*
madd(FunctionalMatrix *a, FunctionalMatrix *b);

/* Subtraction */
FunctionalMatrix*
msub(FunctionalMatrix *a, FunctionalMatrix *b);

/* Scalar product */
FunctionalMatrix*
mprod(COMPLEX_TYPE r, FunctionalMatrix *a);

/* Scalar division */
FunctionalMatrix*
mdiv(COMPLEX_TYPE r, FunctionalMatrix *a);

/* Matrix multiplication */
FunctionalMatrix*
matmul(FunctionalMatrix *a, FunctionalMatrix *b);

/* Entity-wise multiplication */
FunctionalMatrix*
ewmul(FunctionalMatrix *a, FunctionalMatrix *b);

/* Kronecker product */
FunctionalMatrix*
kron(FunctionalMatrix *a, FunctionalMatrix *b);

/* Transpose */
FunctionalMatrix*
transpose(FunctionalMatrix *m);

/* Hermitian transpose */
FunctionalMatrix*
dagger(FunctionalMatrix *m);

NATURAL_TYPE
rows(FunctionalMatrix *m);

NATURAL_TYPE
columns(FunctionalMatrix *m);

#ifndef _MSC_VER
__attribute__ ((const))
#endif
NATURAL_TYPE
_GetElemIndex(int value, NATURAL_TYPE position, int bit);

#ifndef _MSC_VER
__attribute__ ((pure))
#endif
COMPLEX_TYPE
_PartialTFunct(NATURAL_TYPE i, NATURAL_TYPE j,
               #ifndef _MSC_VER
               NATURAL_TYPE unused1 __attribute__((unused)), NATURAL_TYPE unused2 __attribute__((unused)),
               #else
               NATURAL_TYPE unused1, NATURAL_TYPE unused2,
               #endif
               void *items);

/* Partial trace */
FunctionalMatrix*
partial_trace(FunctionalMatrix *m, int elem);

/*
 * Calculates the number of bytes added to a string
 * using the result of the sprintf function.
 */
#ifndef _MSC_VER
__attribute__ ((const))
#endif
int
_bytes_added(int sprintfRe);

/* Gets the size in memory */
#ifndef _MSC_VER
__attribute__ ((pure))
#endif
size_t
getMemory(FunctionalMatrix *fm);

/* Print matrix */
#ifndef _MSC_VER
__attribute__ ((pure))
#endif
char*
FM_toString(FunctionalMatrix *a);

#endif
