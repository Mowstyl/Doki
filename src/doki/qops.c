#include <Python.h>
#include <errno.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "arraylist.h"
#include "platform.h"
#include "qgate.h"
#include "qops.h"
#include "qstate.h"

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static COMPLEX_TYPE
_densityFun (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
             NATURAL_TYPE unused1 __attribute__ ((unused)),
             NATURAL_TYPE unused2 __attribute__ ((unused)),
#else
             NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
             void *rawstate);

REAL_TYPE
get_global_phase (struct state_vector *state)
{
  NATURAL_TYPE i;
  REAL_TYPE phase;
  COMPLEX_TYPE val;

  if (state->fcarg_init)
    {
      return state->fcarg;
    }

  phase = 0.0;
  for (i = 0; i < state->size; i++)
    {
      val = state_get (state, i);
      if (RE (val) != 0. || IM (val) != 0.)
        {
          if (IM (val) != 0.)
            {
              phase = ARG (val);
            }
          break;
        }
    }
  state->fcarg = phase;
  state->fcarg_init = 1;

  return phase;
}

REAL_TYPE
probability (struct state_vector *state, unsigned int target_id)
{
  NATURAL_TYPE i;
  REAL_TYPE value;
  COMPLEX_TYPE val;

  value = 0;
#pragma omp parallel for reduction (+:value) \
                             default (none) \
                             shared (state, target_id) \
                             private (i, val)
  for (i = 0; i < state->size; i++)
    {
      if ((i & (NATURAL_ONE << target_id)) != 0)
        {
          val = state_get (state, i);
          value += RE (val) * RE (val) + IM (val) * IM (val);
        }
    }

  return value;
}

unsigned char
join (struct state_vector *r, struct state_vector *s1, struct state_vector *s2)
{
  NATURAL_TYPE i, j, new_index;
  COMPLEX_TYPE o1, o2;
  unsigned char exit_code;

  exit_code = state_init (r, s1->num_qubits + s2->num_qubits, 0);
  if (exit_code != 0)
    {
      return exit_code;
    }

#pragma omp parallel for default(none)                                        \
    shared(r, s1, s2, exit_code) private(i, j, o1, o2, new_index)
  for (i = 0; i < s1->size; i++)
    {
      o1 = state_get (s1, i);
      for (j = 0; j < s2->size; j++)
        {
          new_index = i * s2->size + j;
          o2 = state_get (s2, j);
          state_set (r, new_index, COMPLEX_MULT (o1, o2));
        }
    }

  return 0;
}

unsigned char
measure (struct state_vector *state, _Bool *result, unsigned int target,
         struct state_vector *new_state, REAL_TYPE roll)
{
  NATURAL_TYPE i;
  REAL_TYPE sum;
  unsigned char exit_code;

  sum = probability (state, target);
  *result = sum > roll;
  exit_code = collapse (state, target, *result, new_state);

  return exit_code;
}

unsigned char
collapse (struct state_vector *state, unsigned int target_id, _Bool value,
          struct state_vector *new_state)
{
  unsigned char exit_code;
  NATURAL_TYPE i, j, count, step;
  _Bool toggle;
  REAL_TYPE norm_const;
  COMPLEX_TYPE aux;

  if (state->num_qubits == 1)
    {
      // state_clear(state);
      new_state->vector = NULL;
      new_state->num_qubits = 0;
      return 0;
    }

  /*
  new_state = MALLOC_TYPE(1, struct state_vector);
  if (new_state == NULL) {
      return 5;
  }
  */
  exit_code = state_init (new_state, state->num_qubits - 1, 0);
  if (exit_code != 0)
    {
      free (new_state);
      return exit_code;
    }
  norm_const = 0;
  toggle = 0;
  count = 0;
  step = NATURAL_ONE << target_id;
  j = 0;
  for (i = 0; i < state->size; i++)
    {
      if (toggle == value)
        {
          aux = state_get (state, i);
          state_set (new_state, j, aux);
          norm_const += pow (RE (aux), 2) + pow (IM (aux), 2);
          j++;
        }
      count++;
      if (count == step)
        {
          count = 0;
          toggle = !toggle;
        }
    }
  new_state->norm_const = sqrt (norm_const);

  return 0;
}

unsigned char
apply_gate (struct state_vector *state, struct qgate *gate,
            unsigned int *targets, unsigned int num_targets,
            unsigned int *controls, unsigned int num_controls,
            unsigned int *anticontrols, unsigned int num_anticontrols,
            struct state_vector *new_state)
{
  struct array_list_e *not_copy;
  REAL_TYPE norm_const;
  unsigned char exit_code;

  not_copy = MALLOC_TYPE (1, struct array_list_e);
  if (not_copy == NULL)
    {
      return 11;
    }
  exit_code = alist_init (not_copy,
                          state->size >> (num_controls + num_anticontrols));

  if (exit_code != 0)
    {
      free (not_copy);
      return exit_code;
    }

  if (new_state == NULL)
    {
      alist_clear (not_copy);
      free (not_copy);
      return 10;
    }
  exit_code = state_init (new_state, state->num_qubits, 0);
  // 0 -> OK
  // 1 -> Error initializing chunk
  // 2 -> Error allocating chunk
  // 3 -> Error setting values (should never happens since init = 0)
  // 4 -> Error allocating state
  if (exit_code != 0)
    {
      alist_clear (not_copy);
      free (not_copy);
      free (new_state);
      return exit_code;
    }

  norm_const = 0;
  exit_code
      = copy_and_index (state, new_state, controls, num_controls, anticontrols,
                        num_anticontrols, &norm_const, not_copy);

  if (exit_code == 0)
    {
      exit_code = calculate_empty (
          state, gate, targets, num_targets, controls, num_controls,
          anticontrols, num_anticontrols, new_state, not_copy, &norm_const);
      if (exit_code == 0)
        {
          new_state->norm_const = sqrt (norm_const);
        }
      else
        {
          exit_code = 5;
        }
    }
  else
    {
      exit_code = 6;
    }

  alist_clear (not_copy);
  free (not_copy);

  return exit_code;
}

unsigned char
copy_and_index (struct state_vector *state, struct state_vector *new_state,
                unsigned int *controls, unsigned int num_controls,
                unsigned int *anticontrols, unsigned int num_anticontrols,
                REAL_TYPE *norm_const, struct array_list_e *not_copy)
{
  NATURAL_TYPE i, count;
  unsigned int j;
  REAL_TYPE aux_const;
  COMPLEX_TYPE aux;
  unsigned char copy_only;
  aux_const = 0;
  count = 0;
#pragma omp parallel for reduction (+:aux_const) \
                             default(none) \
                             shared (state, not_copy, new_state, \
                                     controls, num_controls, \
                                     anticontrols, num_anticontrols, \
                                     norm_const, count) \
                             private (copy_only, i, j, aux)
  for (i = 0; i < state->size; i++)
    {
      // If there has been any error in this thread, we skip
      copy_only = 0;

      for (j = 0; j < num_controls; j++)
        {
          /* If any control bit is set to 0 we set copy_only to true */
          if ((i & (NATURAL_ONE << controls[j])) == 0)
            {
              copy_only = 1;
              break;
            }
        }
      if (!copy_only)
        {
          for (j = 0; j < num_anticontrols; j++)
            {
              /* If any anticontrol bit is not set to 0 we set copy_only to
               * true */
              if ((i & (NATURAL_ONE << anticontrols[j])) != 0)
                {
                  copy_only = 1;
                  break;
                }
            }
        }
      // If copy_only is true it means that we just need to copy the old state
      // element
      if (copy_only)
        {
          aux = state_get (state, i);
          aux_const += pow (RE (aux), 2) + pow (IM (aux), 2);
          state_set (new_state, i, aux);
        }
      else
        {
#pragma omp critical(add_to_not_copy)
          {
            alist_set (not_copy, count, i);
            count++;
          }
        }
    }
  *norm_const = aux_const;

  return 0;
}

unsigned char
calculate_empty (struct state_vector *state, struct qgate *gate,
                 unsigned int *targets, unsigned int num_targets,
                 unsigned int *controls, unsigned int num_controls,
                 unsigned int *anticontrols, unsigned int num_anticontrols,
                 struct state_vector *new_state, struct array_list_e *not_copy,
                 REAL_TYPE *norm_const)
{
  NATURAL_TYPE i, reg_index, curr_id;
  COMPLEX_TYPE sum;
  REAL_TYPE aux_const;
  unsigned char aux_code;
  unsigned int j, k, row;
  aux_code = 0;
  aux_const = 0;
  reg_index = 0;
  curr_id = 0;
// We can calculate each element of the new state separately
#pragma omp parallel for reduction (+:aux_const) \
                             default(none) \
                             shared (state, not_copy, new_state, gate, \
                                     targets, num_targets, \
                                     controls, num_controls, \
                                     anticontrols, num_anticontrols, \
                                     norm_const) \
                             private (curr_id, sum, row, reg_index, i, j, k)
  for (i = 0; i < not_copy->size; i++)
    {
      // If there has been any error in this thread, we skip
      curr_id = alist_get (not_copy, i);
      reg_index = curr_id;
      sum = COMPLEX_ZERO;
      // We have gate->size elements to add in sum
      for (j = 0; j < gate->size; j++)
        {
          // We get the value of each target qubit id on the current new state
          // element and we store it in rowbits following the same order as the
          // one in targets
          row = 0;
          for (k = 0; k < num_targets; k++)
            {
              row += ((curr_id & (NATURAL_ONE << targets[k])) != 0) << k;
              // We check the value of the kth bit of j
              // and set the value of the kth target bit to it
              if ((j & (NATURAL_ONE << k)) != 0)
                {
                  reg_index |= NATURAL_ONE << targets[k];
                }
              else
                {
                  reg_index &= ~(NATURAL_ONE << targets[k]);
                }
            }
          sum = COMPLEX_ADD (sum, COMPLEX_MULT (state_get (state, reg_index),
                                                gate->matrix[row][j]));
        }
      aux_const += pow (RE (sum), 2) + pow (IM (sum), 2);
      // printf("[DEBUG: %i] -> new_state[%lld]\n", omp_get_thread_num(),
      // curr_id);
      state_set (new_state, curr_id, sum);
    }
  *norm_const += aux_const;
  return aux_code;
}

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static COMPLEX_TYPE
_densityFun (NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
             NATURAL_TYPE unused1 __attribute__ ((unused)),
             NATURAL_TYPE unused2 __attribute__ ((unused)),
#else
             NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
             void *rawstate)
{
  COMPLEX_TYPE elem_i, elem_j, result;
  struct state_vector *state = (struct state_vector *)PyCapsule_GetPointer (
      rawstate, "qsimov.doki.state_vector");
  if (state == NULL)
    {
      return COMPLEX_NAN;
    }

  elem_i = state_get (state, i);
  elem_j = state_get (state, j);
  // printf("state[%lld] = " COMPLEX_STRING_FORMAT "\n", i,
  // COMPLEX_STRING(elem_i)); printf("state[%lld] = " COMPLEX_STRING_FORMAT
  // "\n", j, COMPLEX_STRING(elem_i));
  result = COMPLEX_MULT (elem_i, conj (elem_j));
  // printf("result = " COMPLEX_STRING_FORMAT "\n", COMPLEX_STRING(result));

  return result;
}

struct Application
{
  /* Capsule containing the state */
  PyObject *state_capsule;
  /* State vector */
  struct FMatrix *state;
  /* Capsule containing the gate */
  PyObject *gate_capsule;
  /* Gate matrix */
  struct FMatrix *gate;
  /* Target qubit indexes */
  unsigned int *targets;
  /* Control qubit indexes */
  unsigned int *controls;
  /* Anticontrol qubit indexes */
  unsigned int *anticontrols;
  /* How many references are there to this object */
  NATURAL_TYPE refcount;
  /* Number of target qubits */
  unsigned int num_targets;
  /* Number of control qubits */
  unsigned int num_controls;
  /* Number of anticontrol qubits */
  unsigned int num_anticontrols;
};

static struct Application *
new_application (PyObject *state_capsule, PyObject *gate_capsule,
                 unsigned int *targets, unsigned int num_targets,
                 unsigned int *controls, unsigned int num_controls,
                 unsigned int *anticontrols, unsigned int num_anticontrols);

static void free_application (void *raw_app);

static void *clone_application (void *raw_app);

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static COMPLEX_TYPE
_ApplyGateFunction (NATURAL_TYPE i,
#ifndef _MSC_VER
                    NATURAL_TYPE unused1 __attribute__ ((unused)),
                    NATURAL_TYPE unused2 __attribute__ ((unused)),
                    NATURAL_TYPE unused3 __attribute__ ((unused)),
#else
                    NATURAL_TYPE unused1, NATURAL_TYPE unused2,
                    NATURAL_TYPE unused3,
#endif
                    void *raw_app);

static struct Application *
new_application (PyObject *state_capsule, PyObject *gate_capsule,
                 unsigned int *targets, unsigned int num_targets,
                 unsigned int *controls, unsigned int num_controls,
                 unsigned int *anticontrols, unsigned int num_anticontrols)
{
  struct Application *data = MALLOC_TYPE (1, struct Application);

  if (data != NULL)
    {
      struct FMatrix *state, *gate;

      state = (struct FMatrix *)PyCapsule_GetPointer (state_capsule,
                                                      "qsimov.doki.funmatrix");
      gate = (struct FMatrix *)PyCapsule_GetPointer (gate_capsule,
                                                     "qsimov.doki.funmatrix");

      if (state == NULL)
        {
          errno = 4;
          return NULL;
        }
      if (gate == NULL)
        {
          errno = 3;
          return NULL;
        }

      Py_INCREF (state_capsule);
      data->state_capsule = state_capsule;
      data->state = state;
      Py_INCREF (gate_capsule);
      data->gate_capsule = gate_capsule;
      data->gate = gate;
      data->targets = targets;
      data->num_targets = num_targets;
      data->controls = controls;
      data->num_controls = num_controls;
      data->anticontrols = anticontrols;
      data->num_anticontrols = num_anticontrols;
      data->refcount = 1;
    }

  return data;
}

static void
free_application (void *raw_app)
{
  struct Application *data = (struct Application *)raw_app;

  if (data == NULL)
    {
      return;
    }

  data->refcount--;
  if (data->refcount == 0)
    {
      Py_DECREF (data->state_capsule);
      data->state_capsule = NULL;
      data->state = NULL;
      Py_DECREF (data->gate_capsule);
      data->gate_capsule = NULL;
      data->gate = NULL;
      free (data->targets);
      data->targets = NULL;
      free (data->controls);
      data->controls = NULL;
      free (data->anticontrols);
      data->anticontrols = NULL;
      data->num_targets = 0;
      data->num_controls = 0;
      data->num_anticontrols = 0;
      free (data);
    }
}

static void *
clone_application (void *raw_app)
{
  struct Application *data = (struct Application *)raw_app;

  if (data == NULL)
    {
      return NULL;
    }

  data->refcount++;
  return raw_app;
}

struct FMatrix *
apply_gate_fmat (PyObject *state_capsule, PyObject *gate_capsule,
                 unsigned int *targets, unsigned int num_targets,
                 unsigned int *controls, unsigned int num_controls,
                 unsigned int *anticontrols, unsigned int num_anticontrols)
{
  struct FMatrix *pFM;
  struct Application *data = new_application (
      state_capsule, gate_capsule, targets, num_targets, controls,
      num_controls, anticontrols, num_anticontrols);

  if (data == NULL)
    {
      errno = 5;
      return NULL;
    }

  pFM = new_FunctionalMatrix (data->state->r, 1, &_ApplyGateFunction, data,
                              free_application, clone_application);
  if (pFM == NULL)
    {
      errno = 1;
      free_application (data);
    }

  return pFM;
}

#ifndef _MSC_VER
__attribute__ ((const))
#endif
static COMPLEX_TYPE
_ApplyGateFunction (NATURAL_TYPE i,
#ifndef _MSC_VER
                    NATURAL_TYPE unused1 __attribute__ ((unused)),
                    NATURAL_TYPE unused2 __attribute__ ((unused)),
                    NATURAL_TYPE unused3 __attribute__ ((unused)),
#else
                    NATURAL_TYPE unused1, NATURAL_TYPE unused2,
                    NATURAL_TYPE unused3,
#endif
                    void *raw_app)
{
  int res;
  NATURAL_TYPE mask, reg_index, row;
  COMPLEX_TYPE val = COMPLEX_ZERO;
  struct Application *data = (struct Application *)raw_app;

  for (unsigned int k = 0; k < data->num_controls; ++k)
    {
      mask = NATURAL_ONE << data->controls[k];
      if (!(i & mask))
        {
          res = getitem (data->state, i, 0, &val);
          if (res != 0)
          {
            printf ("Error[C] %d while getting state item\n", res);
            return COMPLEX_NAN;
          }
          return val;
        }
    }

  for (unsigned int k = 0; k < data->num_controls; ++k)
    {
      mask = NATURAL_ONE << data->anticontrols[k];
      if (i & mask)
        {
          res = getitem (data->state, i, 0, &val);
          if (res != 0)
          {
            printf ("Error[A] %d while getting state item\n", res);
            return COMPLEX_NAN;
          }
          return val;
        }
    }

  for (unsigned int n = 0; n < data->gate->r; ++n)
    {
      // We get the value of each target qubit id on the current new state
      // element and we store it in rowbits following the same order as the
      // one in targets
      COMPLEX_TYPE aux, aux2;

      row = 0;
      for (unsigned int k = 0; k < data->num_targets; k++)
        {
          row += ((i & (NATURAL_ONE << data->targets[k])) != 0) << k;
          // We check the value of the kth bit of j
          // and set the value of the kth target bit to it
          if ((n & (NATURAL_ONE << k)) != 0)
            {
              reg_index |= NATURAL_ONE << data->targets[k];
            }
          else
            {
              reg_index &= ~(NATURAL_ONE << data->targets[k]);
            }
        }
      res = getitem (data->state, reg_index, 0, &aux);
      if (res != 0)
        {
          printf ("Error[T] %d while getting state item\n", res);
          return COMPLEX_NAN;
        }
      res = getitem (data->gate, row, n, &aux2);
      if (res != 0)
        {
          printf ("Error[T] %d while getting gate item\n", res);
          return COMPLEX_NAN;
        }
      val = COMPLEX_ADD (val, COMPLEX_MULT (aux, aux2));
    }

  return val;
}

struct FMatrix *
density_matrix (PyObject *state_capsule)
{
  struct FMatrix *dm = NULL;
  struct state_vector *state
      = PyCapsule_GetPointer (state_capsule, "qsimov.doki.state_vector");

  if (state != NULL)
    {
      dm = new_FunctionalMatrix (state->size, state->size, &_densityFun,
                                 state_capsule, free_capsule, clone_capsule);
      if (dm != NULL)
        {
          Py_INCREF (state_capsule);
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

  return dm;
}
