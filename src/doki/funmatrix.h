#pragma once
#ifndef FUNMATRIX_H_
#define FUNMATRIX_H_

#include "platform.h"
#include <Python.h>
#include <complex.h>
#include <stdbool.h>

struct FMatrix
{
  /* Scalar number s that will be multiplied by the result of f(i, j) or
   * multiplied by A op B */
  COMPLEX_TYPE s;
  /* Number of rows */
  NATURAL_TYPE r;
  /* Number of columns */
  NATURAL_TYPE c;
  /* Function that, given (i, j, nrows, ncolumns, *argv)
  returns the value of the element (i, j) of
  the matrix */
  COMPLEX_TYPE (*f)
  (NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, void *);
  /* Pointer to matrix A in case an operation is going to be performed A op B
   */
  struct FMatrix *A;
  PyObject *A_capsule;
  /* Pointer to matrix B in case an operation is going to be performed A op B
   */
  struct FMatrix *B;
  PyObject *B_capsule;
  /* Extra arguments to pass to the function f */
  void *argv;
  /* Function that frees memory used by argv (if needed) */
  void (*argv_free) (void *);
  /* Function that clones argv (if needed) */
  void *(*argv_clone) (void *);
  /* Whether the matrix has to be transposed or not */
  bool transpose;
  /* Whether the matrix has to be complex conjugated or not */
  bool conjugate;
  /* Whether the matrix is simple or you have to perform an operation */
  bool simple;
  /* Operation to apply between the matrices.
      0 -> Matrix addition               A + B
      1 -> Matrix subtraction            A - B
      2 -> Matrix multiplication         A * B
      3 -> Entity-wise multiplication    A .* B
      4 -> Kronecker product             A ⊗ B
  */
  short op;
};

void free_capsule (void *raw_capsule);

void *clone_capsule (void *raw_capsule);

/* Constructor */
struct FMatrix *
new_FunctionalMatrix (NATURAL_TYPE n_rows, NATURAL_TYPE n_columns,
                      COMPLEX_TYPE (*fun) (NATURAL_TYPE, NATURAL_TYPE,
                                           NATURAL_TYPE, NATURAL_TYPE, void *),
                      void *argv, void (*argv_free) (void *),
                      void *(*argv_clone) (void *));

/*
 * Get the element (i, j) from the matrix a, and return the result in
 * the address pointed by sol.
 * Return values:
 * 0 -> OK
 * 1 -> Error adding
 * 2 -> Error substracting
 * 3 -> Error multiplicating matrices
 * 4 -> Error multiplicating matrices entity-wise
 * 5 -> Error calculating Kronecker product
 * 6 -> Unknown operation
 * 7 -> Out of bounds
 * 8 -> f returned NAN
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

/* I(2^left) kron A kron I(2^right). Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 3 -> Matrix is NULL
 * 5 -> Could not allocate data array
 * 6 -> Could not allocate data struct
 */
struct FMatrix *eyeKron (PyObject *raw_m, NATURAL_TYPE leftQ,
                         NATURAL_TYPE rightQ);

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

/* Applies a projection matrix to a state vector, leaving only the amplitudes
 * for which qubitId is in the state value.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 3 -> Matrix operand is NULL
 * 5 -> Could not allocate argv struct
 */
struct FMatrix *projection (PyObject *raw_m, NATURAL_TYPE qubitId, bool value);

NATURAL_TYPE
rows (struct FMatrix *m);

NATURAL_TYPE
columns (struct FMatrix *m);

/* Partial trace. Returns NULL on error.
 * errno values:
 * 1 -> Could not allocate result matrix
 * 2 -> m is not a square matrix
 * 3 -> Matrix is NULL
 * 5 -> Could not allocate argv struct
 * 6 -> elem id has to be >= 0
 */
struct FMatrix *partial_trace (PyObject *raw_m, int elem);

struct FMatrix *Identity (int n);

struct FMatrix *StateZero (int n);

struct FMatrix *DensityZero (int n);

struct FMatrix *Walsh (int n);

struct FMatrix *Hadamard (int n);

struct FMatrix *CU (PyObject *raw_U);

struct FMatrix *CustomMat (COMPLEX_TYPE *matrix_2d, NATURAL_TYPE length,
                           NATURAL_TYPE nrows, NATURAL_TYPE ncols);

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
