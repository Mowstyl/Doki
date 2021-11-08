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

struct state_vector
{
    /* total size of the vector */
    NATURAL_TYPE   size;
    /* number of chunks */
    size_t         num_chunks;
    /* number of qubits in this quantum system */
    unsigned int   num_qubits;
    /* partial vector */
    COMPLEX_TYPE **vector;
    /* normalization constant */
    REAL_TYPE      norm_const;
    /* fcarg initialized */
    _Bool          fcarg_init;
    /* first complex argument */
    REAL_TYPE      fcarg;
};

/** \fn unsigned char state_init(struct state_vector *this, unsigned int num_qubits, int init);
 *  \brief Initialize a state vector structure.
 *  \param this Pointer to an already allocated state_vector structure.
 *  \param num_qubits The number of qubits represented by this state (a maximum of MAX_NUM_QUBITS).
 *  \param init Whether to initialize to {1, 0, ..., 0} or not.
 *  \return 0 if ok, 1 if failed to allocate vector, 2 if failed to allocate any chunk, 3 if num_qubits > MAX_NUM_QUBITS.
 */
unsigned char
state_init(struct state_vector *this, unsigned int num_qubits, int init);

/** \fn unsigned char state_clone(struct state_vector *dest, struct state_vector *source);
 *  \brief Clone a state vector structure.
 *  \param dest Pointer to an already allocated state_vector structure i which the copy will be stored.
 *  \param source Pointer to the state_vector structure that has to be cloned.
 *  \return 0 if ok, 1 if failed to allocate dest vector, 2 if failed to allocate any chunk.
 */
unsigned char
state_clone(struct state_vector *dest, struct state_vector *source);

void
state_clear(struct state_vector *this);

void
state_set(struct state_vector *this, NATURAL_TYPE i, COMPLEX_TYPE value);

COMPLEX_TYPE
state_get(struct state_vector *this, NATURAL_TYPE i);

#endif
