/** \file qstate.h
 *  \brief Functions and structures needed to define a quantum state.
 *
 *  In this file some functions and structures have been defined
 *  to create and destroy a quantum state vector.
 */

/** \def __QSTATE_H
 *  \brief Indicates if qstate.h has already been loaded.
 *
 *  If __QSTATE_H is defined, qstate.h file has already been included.
 */

/** \struct array_list qstate.h "qstate.h"
 *  \brief List of complex number arrays.
 *  A list of complex number arrays (chunks).
 */

#ifndef __QSTATE_H
#define __QSTATE_H

#include "platform.h"

struct array_list
{
  /* size of this chunk */
  NATURAL_TYPE       node_size;
  /* array of elements in this chunk */
  COMPLEX_TYPE      *node_elements;
  /* pointer to next chunk */
  struct array_list *next;
};

struct state_vector
{
  /* id of the first element stored in this computation node */
  NATURAL_TYPE       first_id;
  /* id of the last element stored in this computation node */
  NATURAL_TYPE       last_id;
  /* total (not partial) size of the vector */
  NATURAL_TYPE       size;
  /* number of qubits in this quantum system */
  unsigned int       num_qubits;
  /* partial vector */
  struct array_list *vector;
  /* normalization constant */
  REAL_TYPE          norm_const;
  /* e^-(complex argument of the first element)*/
  COMPLEX_TYPE       fcarg;
};

unsigned char
list_new(struct array_list *this, NATURAL_TYPE size, int init);

void
list_clear(struct array_list *this);

unsigned char
list_chunk(struct array_list *this, int init);

unsigned char
state_init(struct state_vector *state, unsigned int num_qubits, int init);

void
state_clear(struct state_vector *state);

unsigned char
state_set(struct state_vector *state, NATURAL_TYPE index, COMPLEX_TYPE value);

unsigned char
state_get(struct state_vector *state, NATURAL_TYPE index, COMPLEX_TYPE *target, _Bool canonical);

#endif
