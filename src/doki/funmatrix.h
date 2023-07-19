#pragma once
#ifndef FUNMATRIX_H_
#define FUNMATRIX_H_

#include "platform.h"
#include <stdbool.h>
#include <complex.h>
#include <Python.h>

struct FMatrix
{
  /* Number of rows */
  NATURAL_TYPE r;
  /* Number of columns */
  NATURAL_TYPE c;
  /* Function that, given (i, j, nrows, ncolumns, *argv)
  returns the value of the element (i, j) of
  the matrix */
  COMPLEX_TYPE (*f)
  (NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, void *);
  /* Scalar number s that will be multiplied by the result of f(i, j) or
   * multiplied by A op B */
  COMPLEX_TYPE s;
  /* Pointer to matrix A in case an operation is going to be performed A op B
   */
  struct FMatrix *A;
  PyObject *A_capsule;
  /* Pointer to matrix B in case an operation is going to be performed A op B
   */
  struct FMatrix *B;
  PyObject *B_capsule;
  /* Operation to apply between the matrices.
      0 -> Matrix addition               A + B
      1 -> Matrix subtraction            A - B
      2 -> Matrix multiplication         A * B
      3 -> Entity-wise multiplication    A .* B
      4 -> Kronecker product             A ⊗ B
  */
  short op;
  /* Whether the matrix has to be transposed or not */
  bool transpose;
  /* Whether the matrix has to be complex conjugated or not */
  bool conjugate;
  /* Whether the matrix is simple or you have to perform an operation */
  bool simple;
  /* Extra arguments to pass to the function f */
  void *argv;
  /* Function that frees memory used by argv (if needed) */
  void (*argv_free) (void *);
  /* Function that clones argv (if needed) */
  void* (*argv_clone) (void *);
};

struct DMatrixForTrace
{
  /* Density Matrix */
  struct FMatrix *m;
  PyObject *m_capsule;
  /* Element to trace out */
  int e;
};

struct Matrix2D
{
  /* Matrix stored in an 2d array */
  COMPLEX_TYPE *matrix2d;
  /* Length of the array (#rows x #columns) */
  NATURAL_TYPE length;
  /* How many references are there to this object */
  NATURAL_TYPE refcount;
};

static void free_matrixelem(void *raw_me);

static void * clone_matrixelem(void *raw_me);

static struct Matrix2D * new_matrix2d(COMPLEX_TYPE *matrix2d, NATURAL_TYPE length);

static void free_matrix2d(void *raw_mat);

static void * clone_matrix2d(void *raw_mat);

/* Constructor */
struct FMatrix *
new_FunctionalMatrix (NATURAL_TYPE n_rows, NATURAL_TYPE n_columns,
                      COMPLEX_TYPE (*fun) (NATURAL_TYPE, NATURAL_TYPE,
                                           NATURAL_TYPE, NATURAL_TYPE, void *),
                      void *argv,
                      void (*argv_free) (void *), void* (*argv_clone) (void *));

/*
 * Get the element (i, j) from the matrix a, and return the result in
 * the address pointed by sol. If a 0 is returned, something went wrong.
 */
int getitem (struct FMatrix *a, NATURAL_TYPE i, NATURAL_TYPE j,
             COMPLEX_TYPE *sol);

/* Addition. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 2 -> Operands misalligned
 * 3 -> First operand is NULL
 * 4 -> Second operand is NULL
 */
struct FMatrix *madd (PyObject *raw_a, PyObject *raw_b);

/* Subtraction. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 2 -> Operands misalligned
 * 3 -> First operand is NULL
 * 4 -> Second operand is NULL
 */
struct FMatrix *msub (PyObject *raw_a, PyObject *raw_b);

/* Scalar product. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 3 -> Matrix operand is NULL
 */
struct FMatrix *mprod (COMPLEX_TYPE r, PyObject *raw_m);

/* Scalar division. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 3 -> Matrix operand is NULL
 */
struct FMatrix *mdiv (COMPLEX_TYPE r, PyObject *raw_m);

/* Matrix multiplication. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 2 -> Operands misalligned
 * 3 -> First operand is NULL
 * 4 -> Second operand is NULL
 */
struct FMatrix *matmul (PyObject *raw_a, PyObject *raw_b);

/* Entity-wise multiplication. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 2 -> Operands misalligned
 * 3 -> First operand is NULL
 * 4 -> Second operand is NULL
 */
struct FMatrix *ewmul (PyObject *raw_a, PyObject *raw_b);

/* Kronecker product. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 3 -> First operand is NULL
 * 4 -> Second operand is NULL
 */
struct FMatrix *kron (PyObject *raw_a, PyObject *raw_b);

/* Transpose. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 3 -> Matrix operand is NULL
 */
struct FMatrix *transpose (PyObject *raw_m);

/* Hermitian transpose. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 3 -> Matrix operand is NULL
 */
struct FMatrix *dagger (PyObject *raw_m);

NATURAL_TYPE
rows (struct FMatrix *m);

NATURAL_TYPE
columns (struct FMatrix *m);

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static NATURAL_TYPE
_GetElemIndex (int value, NATURAL_TYPE position, int bit);

#ifndef _MSC_VER
__attribute__ ((pure))
#endif
static COMPLEX_TYPE
_PartialTFunct (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                NATURAL_TYPE unused1 __attribute__ ((unused)),
                NATURAL_TYPE unused2 __attribute__ ((unused)),
#else
                NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
                void *items);

/* Partial trace. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 2 -> m is not a square matrix
 * 3 -> Matrix is NULL
 * 5 -> Could not allocate argv struct
 * 6 -> elem id has to be >= 0
 */
struct FMatrix *partial_trace (PyObject *raw_m, int elem);

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static COMPLEX_TYPE
_IdentityFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                   NATURAL_TYPE unused1 __attribute__ ((unused)),
                   NATURAL_TYPE unused2 __attribute__ ((unused)),
                   void *unused3 __attribute__ ((unused))
#else
                   NATURAL_TYPE unused1, NATURAL_TYPE unused2, void *unused3
#endif
                  );

struct FMatrix *Identity (int n);

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static COMPLEX_TYPE
_StateZeroFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                    NATURAL_TYPE unused1 __attribute__ ((unused)),
                    NATURAL_TYPE unused2 __attribute__ ((unused)),
                    void *unused3 __attribute__ ((unused))
#else
                    NATURAL_TYPE unused1, NATURAL_TYPE unused2, void *unused3
#endif
                   );

struct FMatrix *StateZero (int n);

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static COMPLEX_TYPE
_WalshFunction (NATURAL_TYPE i, NATURAL_TYPE j, NATURAL_TYPE size,
#ifndef _MSC_VER
                NATURAL_TYPE unused __attribute__ ((unused)),
#else
                NATURAL_TYPE unused,
#endif
                void *isHadamard);

void *clone_bool (void *raw_ptr);

struct FMatrix *Walsh (int n);

struct FMatrix *Hadamard (int n);

#ifndef _MSC_VER
__attribute__ ((pure))
#endif
static COMPLEX_TYPE
_CUFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
             NATURAL_TYPE unused1 __attribute__ ((unused)),
             NATURAL_TYPE unused2 __attribute__ ((unused)),
#else
             NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
             void *RawU);

void free_capsule(void *raw_capsule);

void *clone_capsule(void *raw_capsule);

struct FMatrix *CU (PyObject *raw_U);

#ifndef _MSC_VER
__attribute__ ((pure))
#endif
static COMPLEX_TYPE
_CustomMat (NATURAL_TYPE i, NATURAL_TYPE j, NATURAL_TYPE nrows,
#ifndef _MSC_VER
            NATURAL_TYPE unused __attribute__ ((unused)),
#else
            NATURAL_TYPE unused,
#endif
            void *matrix_2d);

struct FMatrix *CustomMat (COMPLEX_TYPE *matrix_2d, NATURAL_TYPE length,
                           NATURAL_TYPE nrows, NATURAL_TYPE ncols);

/*
 * Calculates the number of bytes added to a string
 * using the result of the sprintf function.
 */
#ifndef _MSC_VER
__attribute__ ((const))
#endif
static int
_bytes_added (int sprintfRe);

/* Gets the size in memory */
#ifndef _MSC_VER
__attribute__ ((pure))
#endif
size_t
getMemory (struct FMatrix *fm);

/* Print matrix */
#ifndef _MSC_VER
__attribute__ ((pure))
#endif
char *
FM_toString (struct FMatrix *a);

void FM_destroy (struct FMatrix *src);

#endif /* FUNMATRIX_H_ */
