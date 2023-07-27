#ifdef __MINGW32__
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include "funmatrix.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  void *matrix2d;
  /* Associated FMatrix (if any) */
  PyObject *fmat;
  /* Length of the array (#rows x #columns) */
  NATURAL_TYPE length;
  /* How many references are there to this object */
  NATURAL_TYPE refcount;
};

struct Projection
{
  /* FMatrix capsule */
  PyObject *fmat_capsule;
  /* FMatrix over which we'll apply the projection */
  struct FMatrix *fmat;
  /* Index of the qubit */
  NATURAL_TYPE qubitId;
  /* How many references are there to this object */
  NATURAL_TYPE refcount;
  /* Value of the qubit */
  bool value;
};

static void free_matrixelem (void *raw_me);

static void *clone_matrixelem (void *raw_me);

static struct Matrix2D *new_matrix2d (void *matrix2d, NATURAL_TYPE length);

static void free_matrix2d (void *raw_mat);

static void *clone_matrix2d (void *raw_mat);

static struct Projection *new_projection (PyObject *parent_capsule,
                                          NATURAL_TYPE qubitId, bool value);

static void free_projection (void *raw_proj);

static void *clone_projection (void *raw_proj);

static struct FMatrix *_WalshHadamard (int n, bool isHadamard);

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

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static COMPLEX_TYPE
_DensityZeroFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                      NATURAL_TYPE unused1 __attribute__ ((unused)),
                      NATURAL_TYPE unused2 __attribute__ ((unused)),
                      void *unused3 __attribute__ ((unused))
#else
                      NATURAL_TYPE unused1, NATURAL_TYPE unused2, void *unused3
#endif
);

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

/*
 * Calculates the number of bytes added to a string
 * using the result of the sprintf function.
 */
#ifndef _MSC_VER
__attribute__ ((const))
#endif
static int
_bytes_added (int sprintfRe);

/* Constructor */
struct FMatrix *
new_FunctionalMatrix (NATURAL_TYPE n_rows, NATURAL_TYPE n_columns,
                      COMPLEX_TYPE (*fun) (NATURAL_TYPE, NATURAL_TYPE,
                                           NATURAL_TYPE, NATURAL_TYPE, void *),
                      void *argv, void (*argv_free) (void *),
                      void *(*argv_clone) (void *))
{
  struct FMatrix *pFM = MALLOC_TYPE (1, struct FMatrix);

  if (pFM != NULL)
    {
      pFM->r = n_rows;
      pFM->c = n_columns;
      pFM->f = fun;
      pFM->A = NULL;
      pFM->A_capsule = NULL;
      pFM->B = NULL;
      pFM->B_capsule = NULL;
      pFM->s = COMPLEX_ONE;
      pFM->op = -1;
      pFM->transpose = false;
      pFM->conjugate = false;
      pFM->simple = true;
      pFM->argv = argv;
      pFM->argv_free = argv_free;
      pFM->argv_clone = argv_clone;
    }

  return pFM;
}

/* Get the element (i, j) from the matrix a */
int
getitem (struct FMatrix *a, NATURAL_TYPE i, NATURAL_TYPE j, COMPLEX_TYPE *sol)
{
  unsigned int k;
  NATURAL_TYPE aux;
  int result = 0;
  COMPLEX_TYPE aux1 = COMPLEX_ZERO, aux2 = COMPLEX_ZERO;

  *sol = COMPLEX_NAN;
  if (i < a->r && j < a->c)
    {
      if (a->transpose)
        {
          aux = i;
          i = j;
          j = aux;
        }

      if (a->simple)
        {
          *sol = a->f (i, j, a->r, a->c, a->argv);
          if (isnan (RE (*sol)) || isnan (IM (*sol)))
            {
              result = 8;
            }
        }
      else
        {
          int res1, res2;
          switch (a->op)
            {
            case 0: /* Matrix addition */
              res1 = getitem (a->A, i, j, &aux1);
              res2 = getitem (a->B, i, j, &aux2);
              if (res1 == 0 && res2 == 0)
                {
                  *sol = COMPLEX_ADD (aux1, aux2);
                }
              else
                {
                  result = 1;
                }
              break;
            case 1: /* Matrix subtraction */
              res1 = getitem (a->A, i, j, &aux1);
              res2 = getitem (a->B, i, j, &aux2);
              if (res1 == 0 && res2 == 0)
                {
                  *sol = COMPLEX_SUB (aux1, aux2);
                }
              else
                {
                  result = 2;
                }
              break;
            case 2: /* Matrix multiplication    */
              *sol = COMPLEX_ZERO;
              for (k = 0; k < a->A->c; k++)
                {
                  res1 = getitem (a->A, i, k, &aux1);
                  res2 = getitem (a->B, k, j, &aux2);
                  if (res1 == 0 && res2 == 0)
                    {
                      *sol = COMPLEX_ADD (*sol, COMPLEX_MULT (aux1, aux2));
                    }
                  else
                    {
                      result = 3;
                      break;
                    }
                }
              break;
            case 3: /* Entity-wise multiplication */
              res1 = getitem (a->A, i, j, &aux1);
              res2 = getitem (a->B, i, j, &aux2);
              if (res1 == 0 && res2 == 0)
                {
                  *sol = COMPLEX_MULT (aux1, aux2);
                }
              else
                {
                  result = 4;
                }
              break;

            case 4: /* Kronecker product */
              res1 = getitem (a->A, i / a->B->r, j / a->B->c, &aux1);
              res2 = getitem (a->B, i % a->B->r, j % a->B->c, &aux2);
              if (res1 == 0 && res2 == 0)
                {
                  *sol = COMPLEX_MULT (aux1, aux2);
                }
              else
                {
                  result = 5;
                }
              break;

            default:
              printf ("Unknown option: %d\n", a->op);
              result = 6;
            }
        }

      if (result == 0 && a->conjugate)
        {
          *sol = conj (*sol);
        }
    }
  else
    {
      printf (
          "(" NATURAL_STRING_FORMAT ", " NATURAL_STRING_FORMAT
          ") is out of bounds!\n Matrix dimensions: (" NATURAL_STRING_FORMAT
          ", " NATURAL_STRING_FORMAT ")\n",
          i, j, a->r, a->c);
      result = 7;
    }

  if (result == 0)
    {
      *sol = COMPLEX_MULT (*sol, a->s);
    }

  return result;
}

/* Addition */
struct FMatrix *
madd (PyObject *raw_a, PyObject *raw_b)
{
  struct FMatrix *a, *b, *pFM = NULL;

  a = PyCapsule_GetPointer (raw_a, "qsimov.doki.funmatrix");
  b = PyCapsule_GetPointer (raw_b, "qsimov.doki.funmatrix");
  if (a == NULL)
    {
      errno = 3;
      return NULL;
    }
  if (b == NULL)
    {
      errno = 4;
      return NULL;
    }

  /* if the dimensions allign (nxm .* nxm)*/
  if (a->r == b->r && a->c == b->c)
    {
      pFM = MALLOC_TYPE (1, struct FMatrix);
      if (pFM != NULL)
        {
          pFM->r = a->r;
          pFM->c = a->c;
          pFM->f = NULL;
          pFM->A = a;
          Py_INCREF (raw_a);
          pFM->A_capsule = raw_a;
          pFM->B = b;
          Py_INCREF (raw_b);
          pFM->B_capsule = raw_b;
          pFM->s = COMPLEX_ONE;
          pFM->op = 0;
          pFM->transpose = false;
          pFM->conjugate = false;
          pFM->simple = false;
          pFM->argv = NULL;
          pFM->argv_free = NULL;
          pFM->argv_clone = NULL;
        }
      else
        {
          errno = 1;
        }
    }
  else
    {
      errno = 2;
    }

  return pFM;
}

/* Subtraction */
struct FMatrix *
msub (PyObject *raw_a, PyObject *raw_b)
{
  struct FMatrix *a, *b, *pFM = NULL;

  a = PyCapsule_GetPointer (raw_a, "qsimov.doki.funmatrix");
  b = PyCapsule_GetPointer (raw_b, "qsimov.doki.funmatrix");
  if (a == NULL)
    {
      errno = 3;
      return NULL;
    }
  if (b == NULL)
    {
      errno = 4;
      return NULL;
    }

  /* if the dimensions allign (nxm .* nxm)*/
  if (a->r == b->r && a->c == b->c)
    {
      pFM = MALLOC_TYPE (1, struct FMatrix);
      if (pFM != NULL)
        {
          pFM->r = a->r;
          pFM->c = a->c;
          pFM->f = NULL;
          pFM->A = a;
          Py_INCREF (raw_a);
          pFM->A_capsule = raw_a;
          pFM->B = b;
          Py_INCREF (raw_b);
          pFM->B_capsule = raw_b;
          pFM->s = COMPLEX_ONE;
          pFM->op = 1;
          pFM->transpose = false;
          pFM->conjugate = false;
          pFM->simple = false;
          pFM->argv = NULL;
          pFM->argv_free = NULL;
          pFM->argv_clone = NULL;
        }
      else
        {
          errno = 1;
        }
    }
  else
    {
      errno = 2;
    }

  return pFM;
}

/* Scalar product */
struct FMatrix *
mprod (COMPLEX_TYPE r, PyObject *raw_m)
{
  struct FMatrix *m, *pFM = NULL;

  m = PyCapsule_GetPointer (raw_m, "qsimov.doki.funmatrix");
  if (m == NULL)
    {
      errno = 3;
      return NULL;
    }

  pFM = MALLOC_TYPE (1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = m->r;
      pFM->c = m->c;
      pFM->f = m->f;
      pFM->A = m->A;
      Py_XINCREF (m->A_capsule);
      pFM->A_capsule = m->A_capsule;
      pFM->B = m->B;
      Py_XINCREF (m->B_capsule);
      pFM->B_capsule = m->B_capsule;
      pFM->s = COMPLEX_MULT (m->s, r);
      pFM->op = m->op;
      pFM->transpose = m->transpose;
      pFM->conjugate = m->conjugate;
      pFM->simple = m->simple;
      if (m->argv_clone != NULL)
        {
          pFM->argv = m->argv_clone (m->argv);
        }
      else
        {
          pFM->argv = m->argv;
        }
      pFM->argv_free = m->argv_free;
      pFM->argv_clone = m->argv_clone;
    }
  else
    {
      errno = 1;
    }

  return pFM;
}

/* Scalar division */
struct FMatrix *
mdiv (COMPLEX_TYPE r, PyObject *raw_m)
{
  struct FMatrix *m, *pFM = NULL;

  m = PyCapsule_GetPointer (raw_m, "qsimov.doki.funmatrix");
  if (m == NULL)
    {
      errno = 3;
      return NULL;
    }

  pFM = MALLOC_TYPE (1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = m->r;
      pFM->c = m->c;
      pFM->f = m->f;
      pFM->A = m->A;
      Py_XINCREF (m->A_capsule);
      pFM->A_capsule = m->A_capsule;
      pFM->B = m->B;
      Py_XINCREF (m->B_capsule);
      pFM->B_capsule = m->B_capsule;
      COMPLEX_DIV (pFM->s, m->s, r);
      pFM->op = m->op;
      pFM->transpose = m->transpose;
      pFM->conjugate = m->conjugate;
      pFM->simple = m->simple;
      if (m->argv_clone != NULL)
        {
          pFM->argv = m->argv_clone (m->argv);
        }
      else
        {
          pFM->argv = m->argv;
        }
      pFM->argv_free = m->argv_free;
      pFM->argv_clone = m->argv_clone;
    }
  else
    {
      errno = 1;
    }

  return pFM;
}

/* Matrix multiplication */
struct FMatrix *
matmul (PyObject *raw_a, PyObject *raw_b)
{
  struct FMatrix *a, *b, *pFM = NULL;

  a = PyCapsule_GetPointer (raw_a, "qsimov.doki.funmatrix");
  b = PyCapsule_GetPointer (raw_b, "qsimov.doki.funmatrix");
  if (a == NULL)
    {
      errno = 3;
      return NULL;
    }
  if (b == NULL)
    {
      errno = 4;
      return NULL;
    }
  /* if the dimensions allign (uxv * vxw) */
  if (a->c == b->r)
    {
      pFM = MALLOC_TYPE (1, struct FMatrix);
      if (pFM != NULL)
        {
          pFM->r = a->r;
          pFM->c = b->c;
          pFM->f = NULL;
          pFM->A = a;
          Py_INCREF (raw_a);
          pFM->A_capsule = raw_a;
          pFM->B = b;
          Py_INCREF (raw_b);
          pFM->B_capsule = raw_b;
          pFM->s = COMPLEX_ONE;
          pFM->op = 2;
          pFM->transpose = false;
          pFM->conjugate = false;
          pFM->simple = false;
          pFM->argv = NULL;
          pFM->argv_free = NULL;
          pFM->argv_clone = NULL;
        }
      else
        {
          errno = 1;
        }
    }
  else
    {
      // printf("[DEBUG] Shape a (%lld, %lld)\n", a->r, a->c);
      // printf("[DEBUG] Shape b (%lld, %lld)\n", b->r, b->c);
      errno = 2;
    }

  return pFM;
}

/* Entity-wise multiplication */
struct FMatrix *
ewmul (PyObject *raw_a, PyObject *raw_b)
{
  struct FMatrix *a, *b, *pFM = NULL;

  a = PyCapsule_GetPointer (raw_a, "qsimov.doki.funmatrix");
  b = PyCapsule_GetPointer (raw_b, "qsimov.doki.funmatrix");
  if (a == NULL)
    {
      errno = 3;
      return NULL;
    }
  if (b == NULL)
    {
      errno = 4;
      return NULL;
    }
  /* if the dimensions allign (nxm .* nxm)*/
  if (a->r == b->r && a->c == b->c)
    {
      pFM = MALLOC_TYPE (1, struct FMatrix);
      if (pFM != NULL)
        {
          pFM->r = a->r;
          pFM->c = a->c;
          pFM->f = NULL;
          pFM->A = a;
          Py_INCREF (raw_a);
          pFM->A_capsule = raw_a;
          pFM->B = b;
          Py_INCREF (raw_b);
          pFM->B_capsule = raw_b;
          pFM->s = COMPLEX_ONE;
          pFM->op = 3;
          pFM->transpose = false;
          pFM->conjugate = false;
          pFM->simple = false;
          pFM->argv = NULL;
          pFM->argv_free = NULL;
          pFM->argv_clone = NULL;
        }
      else
        {
          errno = 1;
        }
    }
  else if (a->r == 1 && b->c == 1)
    { /* row .* column */
      pFM = matmul (raw_b, raw_a);
    }
  else if (b->r == 1 && a->c == 1)
    { /* column .* row */
      pFM = matmul (raw_a, raw_b);
    }
  else
    {
      errno = 2;
    }

  return pFM;
}

/* Kronecker product */
struct FMatrix *
kron (PyObject *raw_a, PyObject *raw_b)
{
  struct FMatrix *a, *b, *pFM = NULL;

  a = PyCapsule_GetPointer (raw_a, "qsimov.doki.funmatrix");
  b = PyCapsule_GetPointer (raw_b, "qsimov.doki.funmatrix");
  if (a == NULL)
    {
      errno = 3;
      return NULL;
    }
  if (b == NULL)
    {
      errno = 4;
      return NULL;
    }

  pFM = MALLOC_TYPE (1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = a->r * b->r;
      pFM->c = a->c * b->c;
      pFM->f = NULL;
      pFM->A = a;
      Py_INCREF (raw_a);
      pFM->A_capsule = raw_a;
      pFM->B = b;
      Py_INCREF (raw_b);
      pFM->B_capsule = raw_b;
      pFM->s = COMPLEX_ONE;
      pFM->op = 4;
      pFM->transpose = false;
      pFM->conjugate = false;
      pFM->simple = false;
      pFM->argv = NULL;
      pFM->argv_free = NULL;
      pFM->argv_clone = NULL;
    }
  else
    {
      errno = 1;
    }

  return pFM;
}

static COMPLEX_TYPE
_eyeKronFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                  NATURAL_TYPE unused1 __attribute__ ((unused)),
                  NATURAL_TYPE unused2 __attribute__ ((unused)),
#else
                  NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
                  void *matrix_2d)
{
  struct Matrix2D *kron_data;
  struct FMatrix *U;
  NATURAL_TYPE *data;
  COMPLEX_TYPE val;
  int result;

  kron_data = (struct Matrix2D *)matrix_2d;
  U = PyCapsule_GetPointer (kron_data->fmat, "qsimov.doki.funmatrix");
  if (U == NULL)
    {
      return COMPLEX_NAN;
    }
  data = (NATURAL_TYPE *)kron_data->matrix2d;

  if (i % data[1] != j % data[1])
    {
      return COMPLEX_ZERO;
    }
  i /= data[1];
  j /= data[1];
  if (i / U->r != j / U->c)
    {
      return COMPLEX_ZERO;
    }
  i = i % U->r;
  j = j % U->c;

  result = getitem (U, i, j, &val) == 0;
  if (!result)
    printf ("Error getting element (" NATURAL_STRING_FORMAT
            ", " NATURAL_STRING_FORMAT ") from U gate on eyeKron\n",
            i, j);

  return val;
}

struct FMatrix *
eyeKron (PyObject *raw_m, NATURAL_TYPE leftQ, NATURAL_TYPE rightQ)
{
  struct FMatrix *m, *pFM;
  struct Matrix2D *data;
  NATURAL_TYPE *raw_data;

  m = PyCapsule_GetPointer (raw_m, "qsimov.doki.funmatrix");
  if (m == NULL)
    {
      errno = 3;
      return NULL;
    }

  raw_data = MALLOC_TYPE (2, NATURAL_TYPE);
  if (raw_data == NULL)
    {
      errno = 5;
      return NULL;
    }
  raw_data[0] = NATURAL_ONE << leftQ;
  raw_data[1] = NATURAL_ONE << rightQ;
  data = new_matrix2d ((void *)raw_data, 2);
  if (data == NULL)
    {
      errno = 6;
      free (raw_data);
      return NULL;
    }

  pFM = new_FunctionalMatrix (
      raw_data[0] * m->r * raw_data[1], raw_data[0] * m->c * raw_data[1],
      &_eyeKronFunction, data, free_matrix2d, clone_matrix2d);

  if (pFM == NULL)
    {
      errno = 1;
      free (raw_data);
      free (data);
    }
  else
    {
      Py_INCREF (raw_m);
      data->fmat = raw_m;
    }

  return pFM;
}

/* Transpose */
struct FMatrix *
transpose (PyObject *raw_m)
{
  struct FMatrix *m, *pFM = NULL;

  m = PyCapsule_GetPointer (raw_m, "qsimov.doki.funmatrix");
  if (m == NULL)
    {
      errno = 3;
      return NULL;
    }

  pFM = MALLOC_TYPE (1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = m->r;
      pFM->c = m->c;
      pFM->f = m->f;
      pFM->A = m->A;
      Py_XINCREF (m->A_capsule);
      pFM->A_capsule = m->A_capsule;
      pFM->B = m->B;
      Py_XINCREF (m->B_capsule);
      pFM->B_capsule = m->B_capsule;
      pFM->s = m->s;
      pFM->op = m->op;
      pFM->transpose = !(m->transpose);
      pFM->conjugate = m->conjugate;
      pFM->simple = m->simple;
      if (m->argv_clone != NULL)
        {
          pFM->argv = m->argv_clone (m->argv);
        }
      else
        {
          pFM->argv = m->argv;
        }
      pFM->argv_free = m->argv_free;
      pFM->argv_clone = m->argv_clone;
    }
  else
    {
      errno = 1;
    }

  return pFM;
}

/* Hermitian transpose */
struct FMatrix *
dagger (PyObject *raw_m)
{
  struct FMatrix *m, *pFM = NULL;

  m = PyCapsule_GetPointer (raw_m, "qsimov.doki.funmatrix");
  if (m == NULL)
    {
      errno = 3;
      return NULL;
    }

  pFM = MALLOC_TYPE (1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = m->r;
      pFM->c = m->c;
      pFM->f = m->f;
      pFM->A = m->A;
      Py_XINCREF (m->A_capsule);
      pFM->A_capsule = m->A_capsule;
      pFM->B = m->B;
      Py_XINCREF (m->B_capsule);
      pFM->B_capsule = m->B_capsule;
      pFM->s = m->s;
      pFM->op = m->op;
      pFM->transpose = !(m->transpose);
      pFM->conjugate = !(m->conjugate);
      pFM->simple = m->simple;
      if (m->argv_clone != NULL)
        {
          pFM->argv = m->argv_clone (m->argv);
        }
      else
        {
          pFM->argv = m->argv;
        }
      pFM->argv_free = m->argv_free;
      pFM->argv_clone = m->argv_clone;
    }
  else
    {
      errno = 1;
    }

  return pFM;
}

static COMPLEX_TYPE
_projectionFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                     NATURAL_TYPE unused1 __attribute__ ((unused)),
                     NATURAL_TYPE unused2 __attribute__ ((unused)),
#else
                     NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
                     void *raw_proj)
{
  struct Projection *proj;
  COMPLEX_TYPE val;
  NATURAL_TYPE qbmask, masked;
  int result;

  proj = (struct Projection *)raw_proj;
  if (proj->fmat == NULL)
    {
      return COMPLEX_NAN;
    }

  qbmask = NATURAL_ONE << proj->qubitId;
  masked = i & qbmask;

  if ((masked && !proj->value) || (!masked && proj->value))
    {
      return COMPLEX_ZERO;
    }

  result = getitem (proj->fmat, i, j, &val) == 0;
  if (!result)
    printf ("Error getting element (" NATURAL_STRING_FORMAT
            ", " NATURAL_STRING_FORMAT ") from U gate on projection\n",
            i, j);

  return val;
}

struct FMatrix *
projection (PyObject *raw_m, NATURAL_TYPE qubitId, bool value)
{
  struct FMatrix *m, *pFM;
  struct Projection *proj;

  m = PyCapsule_GetPointer (raw_m, "qsimov.doki.funmatrix");
  if (m == NULL)
    {
      errno = 3;
      return NULL;
    }

  proj = new_projection (raw_m, qubitId, value);
  if (proj == NULL)
    {
      errno = 5;
      return NULL;
    }

  pFM = new_FunctionalMatrix (m->r, m->c, &_projectionFunction, proj,
                              free_projection, clone_projection);

  if (pFM == NULL)
    {
      errno = 1;
      free_projection (proj);
    }

  return pFM;
}

static NATURAL_TYPE
_GetElemIndex (int value, NATURAL_TYPE position, int bit)
{
  NATURAL_TYPE index = 0, aux = 1;

  if ((value == 0 || value == 1) && bit >= 0)
    {
      if (bit != 0)
        {
          aux = NATURAL_ONE << bit;
        }
      index = position % aux + (position / aux) * (aux << 1) + value * aux;
    }

  return index;
}

static COMPLEX_TYPE
_PartialTFunct (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                NATURAL_TYPE unused1 __attribute__ ((unused)),
                NATURAL_TYPE unused2 __attribute__ ((unused)),
#else
                NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
                void *items)
{
  COMPLEX_TYPE sol = COMPLEX_ZERO, aux = COMPLEX_ZERO;
  struct DMatrixForTrace *me;

  if (items != NULL)
    {
      me = (struct DMatrixForTrace *)items;

      if (getitem (me->m, _GetElemIndex (0, i, me->e),
                   _GetElemIndex (0, j, me->e), &sol)
              == 0
          && getitem (me->m, _GetElemIndex (1, i, me->e),
                      _GetElemIndex (1, j, me->e), &aux)
                 == 0)
        {
          sol = COMPLEX_ADD (sol, aux);
        }
    }

  return sol;
}

static void
free_matrixelem (void *raw_me)
{
  struct DMatrixForTrace *me = (struct DMatrixForTrace *)raw_me;

  if (me == NULL)
    {
      return;
    }
  Py_DECREF (me->m_capsule);
  me->m = NULL;
  me->m_capsule = NULL;
  me->e = -1;
  free (me);
}

static void *
clone_matrixelem (void *raw_me)
{
  struct DMatrixForTrace *new_me, *me = (struct DMatrixForTrace *)raw_me;

  if (me == NULL)
    {
      return NULL;
    }

  new_me = MALLOC_TYPE (1, struct DMatrixForTrace);
  if (new_me == NULL)
    {
      printf ("Error while cloning extra partial trace data. Could not "
              "allocate memory. Things might get weird.\n");
      return NULL;
    }

  new_me->m = me->m;
  Py_INCREF (me->m_capsule);
  new_me->m_capsule = me->m_capsule;
  new_me->e = me->e;

  return new_me;
}

/* Partial trace */
struct FMatrix *
partial_trace (PyObject *raw_m, int elem)
{
  struct FMatrix *m, *pt = NULL;
  struct DMatrixForTrace *me = NULL;

  m = PyCapsule_GetPointer (raw_m, "qsimov.doki.funmatrix");
  if (m == NULL)
    {
      errno = 3;
      return NULL;
    }
  if (m->r != m->c)
    {
      errno = 2;
      return NULL;
    }
  if (elem < 0)
    {
      errno = 6;
      return NULL;
    }

  me = MALLOC_TYPE (1, struct DMatrixForTrace);
  if (me != NULL)
    {
      me->m = m;
      Py_INCREF (raw_m);
      me->m_capsule = raw_m;
      me->e = elem;
      pt = new_FunctionalMatrix (m->r >> 1, m->c >> 1, _PartialTFunct, me,
                                 free_matrixelem, clone_matrixelem);
      if (pt == NULL)
        {
          Py_DECREF (raw_m);
          free_matrixelem (me);
          errno = 1;
        }
    }
  else
    {
      errno = 5;
    }

  return pt;
}

NATURAL_TYPE
rows (struct FMatrix *m) { return m->r; }

NATURAL_TYPE
columns (struct FMatrix *m) { return m->c; }

static COMPLEX_TYPE
_IdentityFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                   NATURAL_TYPE unused1 __attribute__ ((unused)),
                   NATURAL_TYPE unused2 __attribute__ ((unused)),
                   void *unused3 __attribute__ ((unused))
#else
                   NATURAL_TYPE unused1, NATURAL_TYPE unused2, void *unused3
#endif
)
{
  return COMPLEX_INIT (i == j, 0);
}

struct FMatrix *
Identity (int n)
{
  struct FMatrix *pFM;
  NATURAL_TYPE size;

  size = NATURAL_ONE << n; // 2^n
  pFM = new_FunctionalMatrix (size, size, &_IdentityFunction, NULL, NULL,
                              NULL);

  return pFM;
}

static COMPLEX_TYPE
_StateZeroFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                    NATURAL_TYPE unused1 __attribute__ ((unused)),
                    NATURAL_TYPE unused2 __attribute__ ((unused)),
                    void *unused3 __attribute__ ((unused))
#else
                    NATURAL_TYPE unused1, NATURAL_TYPE unused2, void *unused3
#endif
)
{
  return COMPLEX_INIT (i == 0, 0);
}

struct FMatrix *
StateZero (int n)
{
  struct FMatrix *pFM;
  NATURAL_TYPE size;

  size = NATURAL_ONE << n; // 2^n
  pFM = new_FunctionalMatrix (size, 1, &_StateZeroFunction, NULL, NULL, NULL);

  return pFM;
}

static COMPLEX_TYPE
_DensityZeroFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
                      NATURAL_TYPE unused1 __attribute__ ((unused)),
                      NATURAL_TYPE unused2 __attribute__ ((unused)),
                      void *unused3 __attribute__ ((unused))
#else
                      NATURAL_TYPE unused1, NATURAL_TYPE unused2, void *unused3
#endif
)
{
  return COMPLEX_INIT (i == 0 && j == 0, 0);
}

struct FMatrix *
DensityZero (int n)
{
  struct FMatrix *pFM;
  NATURAL_TYPE size;

  size = NATURAL_ONE << n; // 2^n
  pFM = new_FunctionalMatrix (size, size, &_DensityZeroFunction, NULL, NULL,
                              NULL);

  return pFM;
}

static COMPLEX_TYPE
_WalshFunction (NATURAL_TYPE i, NATURAL_TYPE j, NATURAL_TYPE size,
#ifndef _MSC_VER
                NATURAL_TYPE unused __attribute__ ((unused)),
#else
                NATURAL_TYPE unused,
#endif
                void *isHadamard)
{
  REAL_TYPE number;
  NATURAL_TYPE mid;

  mid = size / 2;
  number = 1;
  if (size == 2)
    {
      if (i == 1 && j == 1)
        number = -1;
    }
  else
    {
      if (i >= mid && j >= mid)
        {
          number = -RE (_WalshFunction (i - mid, j - mid, mid, 0, false));
        }
      else
        {
          if (i >= mid)
            i = i - mid;
          if (j >= mid)
            j = j - mid;
          number = RE (_WalshFunction (i, j, mid, 0, false));
        }
    }

  if ((bool)isHadamard)
    {
      number /= sqrt (size);
    }

  return COMPLEX_INIT (number, 0);
}

static struct FMatrix *
_WalshHadamard (int n, bool isHadamard)
{
  struct FMatrix *pFM;
  NATURAL_TYPE size;

  size = NATURAL_ONE << n; // 2^n
  pFM = new_FunctionalMatrix (size, size, &_WalshFunction, (void *)isHadamard,
                              NULL, NULL);

  return pFM;
}

struct FMatrix *
Walsh (int n)
{
  return _WalshHadamard (n, false);
}

struct FMatrix *
Hadamard (int n)
{
  return _WalshHadamard (n, true);
}

static COMPLEX_TYPE
_CUFunction (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
             NATURAL_TYPE unused1 __attribute__ ((unused)),
             NATURAL_TYPE unused2 __attribute__ ((unused)),
#else
             NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
             void *RawU)
{
  COMPLEX_TYPE val;
  int result = 1;
  struct FMatrix *U
      = (struct FMatrix *)PyCapsule_GetPointer (RawU, "qsimov.doki.funmatrix");

  if (i < rows (U) || j < columns (U))
    val = COMPLEX_INIT (i == j, 0);
  else
    result = getitem (U, i - rows (U), j - columns (U), &val) == 0;

  if (!result)
    printf ("Error getting element (" NATURAL_STRING_FORMAT
            ", " NATURAL_STRING_FORMAT ") from U gate\n",
            i - rows (U), j - columns (U));

  return val;
}

void
free_capsule (void *raw_capsule)
{
  PyObject *capsule = (PyObject *)raw_capsule;

  if (capsule == NULL)
    {
      return;
    }

  Py_DECREF (capsule);
}

void *
clone_capsule (void *raw_capsule)
{
  PyObject *capsule = (PyObject *)raw_capsule;

  if (capsule == NULL)
    {
      return NULL;
    }

  Py_INCREF (capsule);

  return raw_capsule;
}

struct FMatrix *
CU (PyObject *raw_U)
{
  struct FMatrix *U = (struct FMatrix *)PyCapsule_GetPointer (
      raw_U, "qsimov.doki.funmatrix");

  if (U == NULL)
    {
      return NULL;
    }

  U = new_FunctionalMatrix (rows (U) * 2, columns (U) * 2, &_CUFunction, raw_U,
                            free_capsule, clone_capsule);
  if (U != NULL)
    {
      Py_INCREF (raw_U);
    }

  return U;
}

static struct Matrix2D *
new_matrix2d (void *matrix2d, NATURAL_TYPE length)
{
  struct Matrix2D *mat = MALLOC_TYPE (1, struct Matrix2D);

  if (mat != NULL)
    {
      mat->matrix2d = matrix2d;
      mat->fmat = NULL;
      mat->length = length;
      mat->refcount = 1;
    }

  return mat;
}

static void
free_matrix2d (void *raw_mat)
{
  struct Matrix2D *mat = (struct Matrix2D *)raw_mat;

  if (mat == NULL)
    {
      return;
    }

  mat->refcount--;
  if (mat->refcount == 0)
    {
      free (mat->matrix2d);
      mat->matrix2d = NULL;
      Py_XDECREF (mat->fmat);
      mat->length = 0;
      free (mat);
    }
}

static void *
clone_matrix2d (void *raw_mat)
{
  struct Matrix2D *mat = (struct Matrix2D *)raw_mat;

  if (mat == NULL)
    {
      return NULL;
    }

  mat->refcount++;
  return raw_mat;
}

static struct Projection *
new_projection (PyObject *parent_capsule, NATURAL_TYPE qubitId, bool value)
{
  struct Projection *proj;
  struct FMatrix *m;

  m = PyCapsule_GetPointer (parent_capsule, "qsimov.doki.funmatrix");
  if (m == NULL)
    {
      errno = 3;
      return NULL;
    }
  proj = MALLOC_TYPE (1, struct Projection);

  if (proj != NULL)
    {
      Py_INCREF (parent_capsule);
      proj->fmat_capsule = parent_capsule;
      proj->fmat = m;
      proj->qubitId = qubitId;
      proj->value = value;
      proj->refcount = 1;
    }

  return proj;
}

static void
free_projection (void *raw_proj)
{
  struct Projection *proj = (struct Projection *)raw_proj;

  if (proj == NULL)
    {
      return;
    }

  proj->refcount--;
  if (proj->refcount == 0)
    {
      Py_DECREF (proj->fmat_capsule);
      proj->fmat_capsule = NULL;
      proj->fmat = NULL;
      free (proj);
    }
}

static void *
clone_projection (void *raw_proj)
{
  struct Projection *proj = (struct Projection *)raw_proj;

  if (proj == NULL)
    {
      return NULL;
    }

  proj->refcount++;
  return raw_proj;
}

static COMPLEX_TYPE
_CustomMat (NATURAL_TYPE i, NATURAL_TYPE j, NATURAL_TYPE nrows,
#ifndef _MSC_VER
            NATURAL_TYPE unused __attribute__ ((unused)),
#else
            NATURAL_TYPE unused,
#endif
            void *matrix_2d)
{
  struct Matrix2D *custom_matrix;
  custom_matrix = (struct Matrix2D *)matrix_2d;
  return ((COMPLEX_TYPE *)custom_matrix->matrix2d)[i * nrows + j];
}

struct FMatrix *
CustomMat (COMPLEX_TYPE *matrix_2d, NATURAL_TYPE length, NATURAL_TYPE nrows,
           NATURAL_TYPE ncols)
{
  return new_FunctionalMatrix (nrows, ncols, &_CustomMat,
                               new_matrix2d ((void *)matrix_2d, length),
                               free_matrix2d, clone_matrix2d);
}

static int
_bytes_added (int sprintfRe)
{
  return (sprintfRe > 0) ? sprintfRe : 0;
}

/* Gets the size in memory */
size_t
getMemory (struct FMatrix *m)
{
  size_t total;

  total = sizeof (*m);
  if (!m->simple)
    {
      total += getMemory (m->A);
      total += getMemory (m->B);
    }

  return total;
}

/* Print matrix */
char *
FM_toString (struct FMatrix *a)
{
  char *text;
  COMPLEX_TYPE it;
  NATURAL_TYPE i, j;
  int length = 0;
  const NATURAL_TYPE MAX_BUF
      = a->r * a->c * (2 * (DECIMAL_PLACES + 7) + 2) + 2;
  // numero de elementos (r * c) multiplicado por numero de cifras
  // significativas establecidas para cada numero por 2 (son complejos) mas 7
  // (1 del signo, otro del . y 5 del exponente e-001) mas 2, uno de la i y
  // otro del espacio/;/] que hay despues de cada numero. Al final se suman 2,
  // uno para el corchete inicial y otro para \0.

  text = (char *)malloc (MAX_BUF);

  it = COMPLEX_ZERO;
  if (text != NULL)
    {
      length += _bytes_added (snprintf (text + length, MAX_BUF - length, "["));
      for (i = 0; i < a->r; i++)
        {
          for (j = 0; j < a->c; j++)
            {
              if (getitem (a, i, j, &it) == 0 && !isnan (RE (it))
                  && !isnan (IM (it)))
                {
                  if (cimag (it) >= 0)
                    {
                      length += _bytes_added (snprintf (
                          text + length, MAX_BUF - length,
                          REAL_STRING_FORMAT "+" REAL_STRING_FORMAT "i",
                          RE (it), IM (it)));
                    }
                  else
                    {
                      length += _bytes_added (snprintf (
                          text + length, MAX_BUF - length,
                          REAL_STRING_FORMAT "-" REAL_STRING_FORMAT "i",
                          RE (it), IM (it)));
                    }
                }
              else
                {
                  length += _bytes_added (
                      snprintf (text + length, MAX_BUF - length, "ERR"));
                }
              if (j < a->c - 1)
                {
                  length += _bytes_added (
                      snprintf (text + length, MAX_BUF - length, " "));
                }
            }

          if (i < a->r - 1)
            {
              length += _bytes_added (
                  snprintf (text + length, MAX_BUF - length, ";"));
            }
        }
      length += _bytes_added (snprintf (text + length, MAX_BUF - length, "]"));
      *(text + length) = '\0';
    }

  return text;
}

void
FM_destroy (struct FMatrix *src)
{
  if (src->A_capsule != NULL)
    {
      Py_DECREF (src->A_capsule);
    }
  if (src->B_capsule != NULL)
    {
      Py_DECREF (src->B_capsule);
    }
  if (src->argv_free != NULL)
    {
      src->argv_free (src->argv);
    }
  src->r = 0;
  src->c = 0;
  src->f = NULL;
  src->A = NULL;
  src->A_capsule = NULL;
  src->B = NULL;
  src->B_capsule = NULL;
  src->s = COMPLEX_NAN;
  src->op = -1;
  src->transpose = false;
  src->conjugate = false;
  src->simple = true;
  src->argv = NULL;
  src->argv_free = NULL;
  src->argv_clone = NULL;
  free (src);
}
