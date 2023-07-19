#ifdef __MINGW32__
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "funmatrix.h"

/* Constructor */
struct FMatrix *
new_FunctionalMatrix (NATURAL_TYPE n_rows, NATURAL_TYPE n_columns,
                      COMPLEX_TYPE (*fun) (NATURAL_TYPE, NATURAL_TYPE,
                                           NATURAL_TYPE, NATURAL_TYPE, void *),
                      void *argv,
                      void (*argv_free) (void *), void* (*argv_clone) (void *))
{
  struct FMatrix *pFM = MALLOC_TYPE(1, struct FMatrix);

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
getitem (struct FMatrix *a, NATURAL_TYPE i, NATURAL_TYPE j,
         COMPLEX_TYPE *sol)
{
  unsigned int k;
  NATURAL_TYPE aux;
  int result = 1;
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
        }
      else
        {
          intmax_t auxilio;
          switch (a->op)
            {
            case 0: /* Matrix addition */
              if (getitem (a->A, i, j, &aux1) && getitem (a->B, i, j, &aux2))
                {
                  *sol = COMPLEX_ADD (aux1, aux2);
                }
              else
                {
                  printf ("Error while operating!\n");
                  result = 0;
                }
              break;
            case 1: /* Matrix subtraction */
              if (getitem (a->A, i, j, &aux1) && getitem (a->B, i, j, &aux2))
                {
                  *sol = COMPLEX_SUB (aux1, aux2);
                }
              else
                {
                  printf ("Error while operating!\n");
                  result = 0;
                }
              break;
            case 2: /* Matrix multiplication    */
              *sol = COMPLEX_ZERO;
              for (k = 0; k < a->A->c; k++)
                {
                  if (getitem (a->A, i, k, &aux1)
                      && getitem (a->B, k, j, &aux2))
                    {
                      *sol = COMPLEX_ADD (*sol, COMPLEX_MULT (aux1, aux2));
                    }
                  else
                    {
                      printf ("Error while operating!\n");
                      result = 0;
                      break;
                    }
                }
              break;
            case 3: /* Entity-wise multiplication */
              if (getitem (a->A, i, j, &aux1) && getitem (a->B, i, j, &aux2))
                {
                  *sol = COMPLEX_MULT (aux1, aux2);
                }
              else
                {
                  printf ("Error while operating!\n");
                  result = 0;
                }
              break;

            case 4: /* Kronecker product */
              printf("Omae ");
              auxilio = a->B->c;
              printf("%lld ", auxilio);
              auxilio = a->B->r;
              printf("%lld ", auxilio);
              printf("wa ");
              printf("%p ", a->A);
              printf("%p ", a->B);
              printf("mou ");
              printf("%p ", &aux1);
              printf("%p ", &aux2);
              fflush(stdout);
              if (getitem (a->A, i / a->B->r, j / a->B->c, &aux1)
                  && getitem (a->B, i % a->B->r, j % a->B->c, &aux2))
                {
                  printf("Shindeiru - ");
                  fflush(stdout);
                  *sol = COMPLEX_MULT (aux1, aux2);
                }
              else
                {
                  printf ("Error while operating!\n");
                  result = 0;
                }
              printf("NANI???\n");
              fflush(stdout);
              break;

            default:
              printf ("Unknown option: %d\n", a->op);
              result = 0;
            }
        }

      if (result && a->conjugate)
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
      result = 0;
    }

  if (result)
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
      pFM = MALLOC_TYPE(1, struct FMatrix);
      if (pFM != NULL)
        {
          pFM->r = a->r;
          pFM->c = a->c;
          pFM->f = NULL;
          pFM->A = a;
          Py_INCREF(raw_a);
          pFM->A_capsule = raw_a;
          pFM->B = b;
          Py_INCREF(raw_b);
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
      pFM = MALLOC_TYPE(1, struct FMatrix);
      if (pFM != NULL)
        {
          pFM->r = a->r;
          pFM->c = a->c;
          pFM->f = NULL;
          pFM->A = a;
          Py_INCREF(raw_a);
          pFM->A_capsule = raw_a;
          pFM->B = b;
          Py_INCREF(raw_b);
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

  pFM = MALLOC_TYPE(1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = m->r;
      pFM->c = m->c;
      pFM->f = m->f;
      pFM->A = m->A;
      Py_INCREF(m->A_capsule);
      pFM->A_capsule = m->A_capsule;
      pFM->B = m->B;
      Py_INCREF(m->B_capsule);
      pFM->B_capsule = m->B_capsule;
      pFM->s = COMPLEX_MULT (m->s, r);
      pFM->op = m->op;
      pFM->transpose = m->transpose;
      pFM->conjugate = m->conjugate;
      pFM->simple = m->simple;
      if (m->argv_clone != NULL)
        {
          pFM->argv = m->argv_clone(m->argv);
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

  pFM = MALLOC_TYPE(1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = m->r;
      pFM->c = m->c;
      pFM->f = m->f;
      pFM->A = m->A;
      Py_INCREF(m->A_capsule);
      pFM->A_capsule = m->A_capsule;
      pFM->B = m->B;
      Py_INCREF(m->B_capsule);
      pFM->B_capsule = m->B_capsule;
      COMPLEX_DIV (pFM->s, m->s, r);
      pFM->op = m->op;
      pFM->transpose = m->transpose;
      pFM->conjugate = m->conjugate;
      pFM->simple = m->simple;
      if (m->argv_clone != NULL)
        {
          pFM->argv = m->argv_clone(m->argv);
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
  if (a->c != b->r)
    {
      pFM = MALLOC_TYPE(1, struct FMatrix);
      if (pFM != NULL)
        {
          pFM->r = a->r;
          pFM->c = b->c;
          pFM->f = NULL;
          pFM->A = a;
          Py_INCREF(raw_a);
          pFM->A_capsule = raw_a;
          pFM->B = b;
          Py_INCREF(raw_b);
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
      pFM = MALLOC_TYPE(1, struct FMatrix);
      if (pFM != NULL)
        {
          pFM->r = a->r;
          pFM->c = a->c;
          pFM->f = NULL;
          pFM->A = a;
          Py_INCREF(raw_a);
          pFM->A_capsule = raw_a;
          pFM->B = b;
          Py_INCREF(raw_b);
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

  pFM = MALLOC_TYPE(1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = a->r * b->r;
      pFM->c = a->c * b->c;
      pFM->f = NULL;
      pFM->A = a;
      Py_INCREF(raw_a);
      pFM->A_capsule = raw_a;
      pFM->B = b;
      Py_INCREF(raw_b);
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
  
  pFM = MALLOC_TYPE(1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = m->r;
      pFM->c = m->c;
      pFM->f = m->f;
      pFM->A = m->A;
      Py_INCREF(m->A_capsule);
      pFM->A_capsule = m->A_capsule;
      pFM->B = m->B;
      Py_INCREF(m->B_capsule);
      pFM->B_capsule = m->B_capsule;
      pFM->s = m->s;
      pFM->op = m->op;
      pFM->transpose = !(m->transpose);
      pFM->conjugate = m->conjugate;
      pFM->simple = m->simple;
      if (m->argv_clone != NULL)
        {
          pFM->argv = m->argv_clone(m->argv);
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
  
  pFM = MALLOC_TYPE(1, struct FMatrix);
  if (pFM != NULL)
    {
      pFM->r = m->r;
      pFM->c = m->c;
      pFM->f = m->f;
      pFM->A = m->A;
      Py_INCREF(m->A_capsule);
      pFM->A_capsule = m->A_capsule;
      pFM->B = m->B;
      Py_INCREF(m->B_capsule);
      pFM->B_capsule = m->B_capsule;
      pFM->s = m->s;
      pFM->op = m->op;
      pFM->transpose = !(m->transpose);
      pFM->conjugate = !(m->conjugate);
      pFM->simple = m->simple;
      if (m->argv_clone != NULL)
        {
          pFM->argv = m->argv_clone(m->argv);
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

static NATURAL_TYPE
_GetElemIndex (int value, NATURAL_TYPE position, int bit)
{
  NATURAL_TYPE index = 0, aux = 1;

  if ((value == 0 || value == 1) && bit >= 0)
    {
      if (bit != 0)
        {
          aux = (NATURAL_TYPE)(2 << (bit - 1));
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
          && getitem (me->m, _GetElemIndex (1, i, me->e),
                      _GetElemIndex (1, j, me->e), &aux))
        {
          sol = COMPLEX_ADD (sol, aux);
        }
    }

  return sol;
}

static void free_matrixelem(void *raw_me)
{
  struct DMatrixForTrace *me = (struct DMatrixForTrace *) raw_me;

  if (me == NULL)
    {
      return;
    }
  Py_DECREF(me->m_capsule);
  me->m = NULL;
  me->m_capsule = NULL;
  me->e = -1;
  free(me);
}

static void * clone_matrixelem(void *raw_me)
{
  struct DMatrixForTrace *new_me,
                         *me = (struct DMatrixForTrace *) raw_me;

  if (me == NULL)
    {
      return NULL;
    }

  new_me = MALLOC_TYPE(1, struct DMatrixForTrace);
  if (new_me == NULL)
    {
      printf("Error while cloning extra partial trace data. Could not allocate memory. Things might get weird.\n");
      return NULL;
    }

  new_me->m = me->m;
  Py_INCREF(me->m_capsule);
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

  me = MALLOC_TYPE(1, struct DMatrixForTrace);
  if (me != NULL)
    {
      me->m = m;
      Py_INCREF(raw_m);
      me->m_capsule = raw_m;
      me->e = elem;
      pt = new_FunctionalMatrix (m->r >> 1, m->c >> 1, _PartialTFunct, me, free_matrixelem, clone_matrixelem);
      if (pt == NULL)
        {
          Py_DECREF(raw_m);
          free(me);
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

  size = 2 << (n - 1); // 2^n
  pFM = new_FunctionalMatrix (size, size, &_IdentityFunction, NULL, NULL, NULL);

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
  return COMPLEX_INIT (i == 0 && j == 0, 0);
}

struct FMatrix *
StateZero (int n)
{
  struct FMatrix *pFM;
  NATURAL_TYPE size;

  size = 2 << (n - 1); // 2^n
  pFM = new_FunctionalMatrix (size, size, &_StateZeroFunction, NULL, NULL, NULL);

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
          number = -RE (_WalshFunction (i - mid, j - mid, mid, 0, 0));
        }
      else
        {
          if (i >= mid)
            i = i - mid;
          if (j >= mid)
            j = j - mid;
          number = RE (_WalshFunction (i, j, mid, 0, 0));
        }
    }

  if (*((int*)isHadamard))
    {
      number /= sqrt (size);
    }

  return COMPLEX_INIT (number, 0);
}

void *clone_bool (void *raw_ptr)
{
  bool *new_ptr,
       *bool_ptr = (bool *) raw_ptr;

  if (bool_ptr == NULL)
    {
      return NULL;
    }

  new_ptr = MALLOC_TYPE(1, bool);
  if (new_ptr == NULL)
    {
      printf("Error while cloning extra Walsh-Hadamard data. Could not allocate memory. Things might get weird.\n");
      return NULL;
    }

  *new_ptr = *bool_ptr;

  return new_ptr;
}

struct FMatrix *
Walsh (int n)
{
  struct FMatrix *pFM;
  NATURAL_TYPE size;
  int *isHadamard = MALLOC_TYPE(1, int);

  *isHadamard = 1;
  size = 2 << (n - 1); // 2^n
  pFM = new_FunctionalMatrix (size, size, &_WalshFunction, isHadamard, free, clone_bool);

  return pFM;
}

struct FMatrix *
Hadamard (int n)
{
  struct FMatrix *pFM;
  NATURAL_TYPE size;
  int *isHadamard = MALLOC_TYPE(1, int);

  *isHadamard = 1;
  size = 2 << (n - 1); // 2^n
  pFM = new_FunctionalMatrix (size, size, &_WalshFunction, isHadamard, free, clone_bool);

  return pFM;
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
  struct FMatrix *U = (struct FMatrix *)RawU;

  if (i < rows (U) || j < columns (U))
    val = COMPLEX_INIT (i == j, 0);
  else
    result = getitem (U, i - rows (U), j - columns (U), &val);

  if (!result)
    printf ("Error getting element (" NATURAL_STRING_FORMAT
            ", " NATURAL_STRING_FORMAT ") from U gate\n",
            i - rows (U), j - columns (U));

  return val;
}

void free_capsule(void *raw_capsule)
{
  PyObject *capsule = (PyObject *) raw_capsule;

  if (capsule == NULL)
    {
      return;
    }

  Py_DECREF(capsule);
}

void *clone_capsule(void *raw_capsule)
{
  PyObject *capsule = (PyObject *) raw_capsule;

  if (capsule == NULL)
    {
      return NULL;
    }

  Py_INCREF(capsule);

  return raw_capsule;
}

struct FMatrix *
CU (PyObject *raw_U)
{
  struct FMatrix *U = (struct FMatrix *) PyCapsule_GetPointer (raw_U, "qsimov.doki.funmatrix");

  if (U == NULL)
    {
      return NULL;
    }

  return new_FunctionalMatrix (rows (U) * 2, columns (U) * 2, &_CUFunction, raw_U, free_capsule, clone_capsule);
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
  COMPLEX_TYPE *custom_matrix;
  custom_matrix = (COMPLEX_TYPE *)matrix_2d;
  return custom_matrix[i * nrows + j];
}

static struct Matrix2D * new_matrix2d(COMPLEX_TYPE *matrix2d, NATURAL_TYPE length)
{
  struct Matrix2D *mat = MALLOC_TYPE(1, struct Matrix2D);

  if (mat != NULL)
    {
      mat->matrix2d = matrix2d;
      mat->length = length;
      mat->refcount = 1;
    }

  return mat;
}

static void free_matrix2d(void *raw_mat)
{
  struct Matrix2D *mat = (struct Matrix2D *) raw_mat;

  if (mat == NULL)
    {
      return;
    }

  mat->refcount--;
  if (mat->refcount == 0)
    {
      free(mat->matrix2d);
      mat->matrix2d = NULL;
      mat->length = 0;
      free(mat);
    }
}

static void *clone_matrix2d(void *raw_mat)
{
  struct Matrix2D *mat = (struct Matrix2D *) raw_mat;

  if (mat == NULL)
    {
      return NULL;
    }

  mat->refcount++;
  return mat;
}

struct FMatrix *
CustomMat (COMPLEX_TYPE *matrix_2d, NATURAL_TYPE length, NATURAL_TYPE nrows, NATURAL_TYPE ncols)
{
  return new_FunctionalMatrix (nrows, ncols, &_CustomMat, new_matrix2d(matrix_2d, length), free_matrix2d, clone_matrix2d);
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
              if (getitem (a, i, j, &it) && !isnan (creal (it))
                  && !isnan (cimag (it)))
                {
                  if (cimag (it) >= 0)
                    {
                      length += _bytes_added (snprintf (
                          text + length, MAX_BUF - length,
                          REAL_STRING_FORMAT "+" REAL_STRING_FORMAT "i",
                          creal (it), cimag (it)));
                    }
                  else
                    {
                      length += _bytes_added (snprintf (
                          text + length, MAX_BUF - length,
                          REAL_STRING_FORMAT "-" REAL_STRING_FORMAT "i",
                          creal (it), cimag (it)));
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

void FM_destroy (struct FMatrix *src) {
  if (src->A_capsule != NULL)
    {
      Py_DECREF(src->A_capsule);
    }
  if (src->B_capsule != NULL)
    {
      Py_DECREF(src->B_capsule);
    }
  if (src->argv_free != NULL)
    {
      src->argv_free(src->argv);
    }
  src->r = 0;
  src->c = 0;
  src->f = NULL;
  src->A = NULL;
  src->A_capsule = NULL;
  src->B = NULL;
  src->B_capsule = NULL;
  src->s = COMPLEX_ZERO;
  src->op = -1;
  src->transpose = false;
  src->conjugate = false;
  src->simple = true;
  src->argv = NULL;
  src->argv_free = NULL;
  src->argv_clone = NULL;
}
