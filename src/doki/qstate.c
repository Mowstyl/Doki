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

#include "qstate.h"
#include "platform.h"
#include <stdbool.h>

unsigned char state_init(struct state_vector *this, unsigned int num_qubits,
			 bool init)
{
	size_t i, offset, errored_chunk;
	bool errored;

	if (num_qubits > MAX_NUM_QUBITS) {
		return 3;
	}
	this->size = NATURAL_ONE << num_qubits;
	this->fcarg_init = 0;
	this->fcarg = -10.0;
	this->num_qubits = num_qubits;
	this->norm_const = 1;
	this->num_chunks = this->size / COMPLEX_ARRAY_SIZE;
	offset = this->size % COMPLEX_ARRAY_SIZE;
	if (offset > 0) {
		this->num_chunks++;
	} else {
		offset = COMPLEX_ARRAY_SIZE;
	}
	this->vector = MALLOC_TYPE(this->num_chunks, COMPLEX_TYPE *);
	if (this->vector == NULL) {
		return 1;
	}
	errored = 0;
	for (i = 0; i < this->num_chunks - 1; i++) {
		if (init) {
			this->vector[i] =
				CALLOC_TYPE(COMPLEX_ARRAY_SIZE, COMPLEX_TYPE);
		} else {
			this->vector[i] =
				MALLOC_TYPE(COMPLEX_ARRAY_SIZE, COMPLEX_TYPE);
		}
		if (this->vector[i] == NULL) {
			errored_chunk = i;
			errored = 1;
			break;
		}
	}
	if (!errored) {
		if (init) {
			this->vector[this->num_chunks - 1] =
				CALLOC_TYPE(offset, COMPLEX_TYPE);
		} else {
			this->vector[this->num_chunks - 1] =
				MALLOC_TYPE(offset, COMPLEX_TYPE);
		}
		if (this->vector[this->num_chunks - 1] == NULL) {
			errored = 1;
			errored_chunk = this->num_chunks - 1;
		}
	}
	if (errored) {
		for (i = 0; i < errored_chunk; i++) {
			free(this->vector[i]);
		}
		free(this->vector);
		return 2;
	}
	if (init) {
		this->vector[0][0] = COMPLEX_ONE;
	}

	return 0;
}

unsigned char state_clone(struct state_vector *dest,
			  struct state_vector *source)
{
	NATURAL_TYPE i;
	unsigned char exit_code;
	exit_code = state_init(dest, source->num_qubits, false);
	if (exit_code != 0) {
		return exit_code;
	}
#pragma omp parallel for default(none) \
	shared(source, dest, exit_code, COMPLEX_ARRAY_SIZE) private(i)
	for (i = 0; i < source->size; i++) {
		dest->vector[i / COMPLEX_ARRAY_SIZE][i % COMPLEX_ARRAY_SIZE] =
			state_get(source, i);
	}
	return 0;
}

void state_clear(struct state_vector *this)
{
	size_t i;
	if (this->vector != NULL) {
		for (i = 0; i < this->num_chunks; i++) {
			free(this->vector[i]);
		}
		free(this->vector);
	}
	this->vector = NULL;
	this->num_chunks = 0;
	this->num_qubits = 0;
	this->size = 0;
	this->norm_const = 0.0;
}

size_t state_mem_size(struct state_vector *this)
{
	size_t state_size;
	if (this == NULL) {
		return 0;
	}
	state_size = sizeof(struct state_vector);
	state_size += this->num_chunks * sizeof(COMPLEX_TYPE *);
	state_size += (this->num_chunks - 1) * COMPLEX_ARRAY_SIZE *
		      sizeof(COMPLEX_TYPE);
	state_size += (this->size % COMPLEX_ARRAY_SIZE) * sizeof(COMPLEX_TYPE);
	return state_size;
}
