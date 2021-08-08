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

struct node_e
{
  /* size of this chunk */
  NATURAL_TYPE    node_size;
  /* array of elements in this chunk */
  NATURAL_TYPE   *node_elements;
  /* pointer to next chunk */
  struct node_e  *next;
};

struct array_list_e
{
  /* id of the first element stored in this computation node */
  NATURAL_TYPE    first_id;
  /* id of the last element stored in this computation node */
  NATURAL_TYPE    last_id;
  /* total (not partial) size of the vector */
  NATURAL_TYPE    size;
  /* partial vector */
  struct node_e  *vector;
};

unsigned char
node_new(struct node_e *this, NATURAL_TYPE size);

void
node_clear(struct node_e *this);

unsigned char
node_chunk(struct node_e *this);

unsigned char
alist_init(struct array_list_e *this, NATURAL_TYPE size);

void
alist_clear(struct array_list_e *this);

unsigned char
alist_set(struct array_list_e *this, NATURAL_TYPE index, NATURAL_TYPE value);

unsigned char
alist_get(struct array_list_e *this, NATURAL_TYPE index, NATURAL_TYPE *target);

#endif
