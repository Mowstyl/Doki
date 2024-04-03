/*
 * Doki: Quantum Computer simulator, using state vectors. QSimov core.
 * Copyright (C) 2021  Hernán Indíbil de la Cruz Calvo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

#pragma once
#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

#include "platform.h"

struct array_list_e
{
  /* total (not partial) size of the vector */
  NATURAL_TYPE size;
  /* number of chunks */
  size_t num_chunks;
  /* vector */
  NATURAL_TYPE **vector;
};

unsigned char alist_init (struct array_list_e *this, NATURAL_TYPE size);

void alist_clear (struct array_list_e *this);

void alist_set (struct array_list_e *this, NATURAL_TYPE i, NATURAL_TYPE value);

NATURAL_TYPE
alist_get (struct array_list_e *this, NATURAL_TYPE i);

#endif /* ARRAYLIST_H_ */
