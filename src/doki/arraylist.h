/** \file arraylist.h
 *  \brief Functions and structures needed to define an arraylist.
 *
 *  In this file some functions and structures have been defined
 *  to create and destroy a quantum state vector.
 */

/** \def __ARRAYLIST_H
 *  \brief Indicates if arraylist.h has already been loaded.
 *
 *  If __ARRAYLIST_H is defined, arraylist.h file has already been included.
 */

/** \struct array_list_e arraylist.h "arraylist.h"
 *  \brief List of int arrays.
 *  A list of int arrays (chunks).
 */

#ifndef __ARRAYLIST_H
#define __ARRAYLIST_H

#include "platform.h"

struct array_list_e
{
  /* total (not partial) size of the vector */
  NATURAL_TYPE   size;
  /* number of chunks */
  size_t         num_chunks;
  /* vector */
  NATURAL_TYPE **vector;
};

unsigned char
alist_init(struct array_list_e *this, NATURAL_TYPE size);

void
alist_clear(struct array_list_e *this);

void
alist_set(struct array_list_e *this, NATURAL_TYPE i, NATURAL_TYPE value);

NATURAL_TYPE
alist_get(struct array_list_e *this, NATURAL_TYPE i);

#endif
