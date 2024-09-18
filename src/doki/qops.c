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

#include <Python.h>
#include <errno.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "qgate.h"
#include "qops.h"
#include "qstate.h"

static size_t size_state_capsule(void *raw_capsule);

#ifndef _MSC_VER
__attribute__((const))
#endif
static COMPLEX_TYPE
_densityFun(NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
	    NATURAL_TYPE unused1 __attribute__((unused)),
	    NATURAL_TYPE unused2 __attribute__((unused)),
#else
	    NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
	    void *rawstate);

REAL_TYPE get_global_phase(struct state_vector *state)
{
	NATURAL_TYPE i;
	REAL_TYPE phase;
	COMPLEX_TYPE val;

	if (state->fcarg_init) {
		return state->fcarg;
	}

	phase = 0.0;
	for (i = 0; i < state->size; i++) {
		val = state_get(state, i);
		if (RE(val) != 0. || IM(val) != 0.) {
			if (IM(val) != 0.) {
				phase = ARG(val);
			}
			break;
		}
	}
	state->fcarg = phase;
	state->fcarg_init = 1;

	return phase;
}

REAL_TYPE probability(struct state_vector *state, unsigned int target_id)
{
	NATURAL_TYPE i, index, qty, low, high, target;
	REAL_TYPE value;
	COMPLEX_TYPE val;

	qty = state->size >> 1;
	target = NATURAL_ONE << target_id;
	low = target - 1;
	high = ~low;

	value = 0;
#pragma omp parallel for reduction (+:value) \
                             default (none) \
                             shared (state, qty, low, high, target, COMPLEX_ARRAY_SIZE) \
                             private (i, index, val)
	for (i = 0; i < qty; i++) {
		index = ((i & high) << 1) + target + (i & low);
		val = state_get(state, index);
		value += RE(val) * RE(val) + IM(val) * IM(val);
	}

	return value;
}

unsigned char join(struct state_vector *r, struct state_vector *s1,
		   struct state_vector *s2)
{
	NATURAL_TYPE i, j, new_index;
	COMPLEX_TYPE o1, o2;
	unsigned char exit_code;

	exit_code = state_init(r, s1->num_qubits + s2->num_qubits, false);
	if (exit_code != 0) {
		return exit_code;
	}

#pragma omp parallel for default(none) \
	shared(r, s1, s2, exit_code, COMPLEX_ARRAY_SIZE) \
	private(i, j, o1, o2, new_index)
	for (i = 0; i < s1->size; i++) {
		o1 = state_get(s1, i);
		for (j = 0; j < s2->size; j++) {
			new_index = i * s2->size + j;
			o2 = state_get(s2, j);
			state_set(r, new_index, COMPLEX_MULT(o1, o2));
		}
	}

	return 0;
}

unsigned char measure(struct state_vector *state, bool *result,
		      unsigned int target, struct state_vector *new_state,
		      REAL_TYPE roll)
{
	REAL_TYPE sum;
	unsigned char exit_code;

	sum = probability(state, target);
	*result = sum > roll;
	exit_code = collapse(state, target, *result, sum, new_state);

	return exit_code;
}

unsigned char collapse(struct state_vector *state, unsigned int target_id,
		       bool value, REAL_TYPE prob_one,
		       struct state_vector *new_state)
{
	unsigned char exit_code;
	NATURAL_TYPE i, j, low, high, val;

	if (state->num_qubits == 1) {
		new_state->vector = NULL;
		new_state->num_qubits = 0;
		return 0;
	}

	exit_code = state_init(new_state, state->num_qubits - 1, false);
	if (exit_code != 0) {
		free(new_state);
		return exit_code;
	}
	val = NATURAL_ONE << target_id;
	low = val - 1;
	high = ~low;
	if (!value) {
		prob_one = 1 - prob_one;
		val = 0;
	}

#pragma omp parallel for default(none) \
	shared(state, new_state, low, high, val, COMPLEX_ARRAY_SIZE) \
	private(i, j)
	for (j = 0; j < new_state->size; j++) {
		i = ((j & high) << 1) + val + (j & low);
		state_set(new_state, j, state_get(state, i));
	}
	new_state->norm_const = sqrt(prob_one);

	return 0;
}

unsigned char apply_gate(struct state_vector *state, struct qgate *gate,
			 unsigned int *targets, unsigned int num_targets,
			 unsigned int *controls, unsigned int num_controls,
			 unsigned int *anticontrols,
			 unsigned int num_anticontrols,
			 struct state_vector *new_state)
{
	REAL_TYPE norm_const;
	unsigned char exit_code;
	NATURAL_TYPE control_mask, anticontrol_mask, i, reg_index;
	unsigned int j, k, row;
	COMPLEX_TYPE sum;

	if (new_state == NULL)
		return 10;

	exit_code = state_init(new_state, state->num_qubits, false);
	// 0 -> OK
	// 1 -> Error initializing chunk
	// 2 -> Error allocating chunk
	// 3 -> Error setting values (should never happens since init = 0)
	// 4 -> Error allocating state
	if (exit_code != 0) {
		free(new_state);
		return exit_code;
	}

	control_mask = NATURAL_ZERO;
	for (j = 0; j < num_controls; j++)
		control_mask |= NATURAL_ONE << controls[j];
	anticontrol_mask = NATURAL_ZERO;
	for (j = 0; j < num_anticontrols; j++)
		anticontrol_mask |= NATURAL_ONE << anticontrols[j];

	norm_const = 0;
#pragma omp parallel for reduction (+:norm_const) \
                                     default(none) \
                                     shared (state, new_state, gate, \
                                             targets, num_targets, \
                                             controls, num_controls, \
                                             anticontrols, num_anticontrols, \
                                             control_mask, anticontrol_mask, \
					     COMPLEX_ZERO, COMPLEX_ARRAY_SIZE) \
                                     private (sum, row, reg_index, i, j, k)
	for (i = 0; i < state->size; i++) {
		if ((i & control_mask) == control_mask &&
		    (i & anticontrol_mask) == 0) {
			// Calculate
			sum = COMPLEX_ZERO;
			reg_index = i;
			// We have gate->size elements to add in sum
			for (j = 0; j < gate->size; j++) {
				// We get the value of each target qubit id on the current new state
				// element and we store it in rowbits following the same order as the
				// one in targets
				row = 0;
				for (k = 0; k < num_targets; k++) {
					row += ((i & (NATURAL_ONE
						      << targets[k])) != 0)
					       << k;
					// We check the value of the kth bit of j
					// and set the value of the kth target bit to it
					if ((j & (NATURAL_ONE << k)) != 0)
						reg_index |= NATURAL_ONE
							     << targets[k];
					else
						reg_index &= ~(NATURAL_ONE
							       << targets[k]);
				}
				sum = COMPLEX_ADD(
					sum,
					COMPLEX_MULT(state_get(state,
							       reg_index),
						     gate->matrix[row][j]));
			}
		} else {
			//Copy
			sum = state_get(state, i);
		}
		state_set(new_state, i, sum);
		norm_const += pow(RE(sum), 2) + pow(IM(sum), 2);
	}
	new_state->norm_const = sqrt(norm_const);

	return 0;
}

#ifndef _MSC_VER
__attribute__((const))
#endif
static COMPLEX_TYPE
_densityFun(NATURAL_TYPE i, NATURAL_TYPE j,
#ifndef _MSC_VER
	    NATURAL_TYPE unused1 __attribute__((unused)),
	    NATURAL_TYPE unused2 __attribute__((unused)),
#else
	    NATURAL_TYPE unused1, NATURAL_TYPE unused2,
#endif
	    void *rawstate)
{
	COMPLEX_TYPE elem_i, elem_j, result;
	struct state_vector *state =
		(struct state_vector *)PyCapsule_GetPointer(
			rawstate, "qsimov.doki.state_vector");
	if (state == NULL) {
		return COMPLEX_NAN;
	}

	elem_i = state_get(state, i);
	elem_j = state_get(state, j);
	// printf("state[" NATURAL_STRING_FORMAT "] = " COMPLEX_STRING_FORMAT "\n", i,
	// COMPLEX_STRING(elem_i)); printf("state[" NATURAL_STRING_FORMAT "] = " COMPLEX_STRING_FORMAT
	// "\n", j, COMPLEX_STRING(elem_i));
	result = COMPLEX_MULT(elem_i, conj(elem_j));
	// printf("result = " COMPLEX_STRING_FORMAT "\n", COMPLEX_STRING(result));

	return result;
}

struct Application {
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
new_application(PyObject *state_capsule, PyObject *gate_capsule,
		unsigned int *targets, unsigned int num_targets,
		unsigned int *controls, unsigned int num_controls,
		unsigned int *anticontrols, unsigned int num_anticontrols);

static void free_application(void *raw_app);

static void *clone_application(void *raw_app);

static size_t size_application(void *raw_app);

#ifndef _MSC_VER
__attribute__((const))
#endif
static COMPLEX_TYPE
_ApplyGateFunction(NATURAL_TYPE i,
#ifndef _MSC_VER
		   NATURAL_TYPE unused1 __attribute__((unused)),
		   NATURAL_TYPE unused2 __attribute__((unused)),
		   NATURAL_TYPE unused3 __attribute__((unused)),
#else
		   NATURAL_TYPE unused1, NATURAL_TYPE unused2,
		   NATURAL_TYPE unused3,
#endif
		   void *raw_app);

static struct Application *
new_application(PyObject *state_capsule, PyObject *gate_capsule,
		unsigned int *targets, unsigned int num_targets,
		unsigned int *controls, unsigned int num_controls,
		unsigned int *anticontrols, unsigned int num_anticontrols)
{
	struct Application *data = MALLOC_TYPE(1, struct Application);

	if (data != NULL) {
		struct FMatrix *state, *gate;

		state = (struct FMatrix *)PyCapsule_GetPointer(
			state_capsule, "qsimov.doki.funmatrix");
		gate = (struct FMatrix *)PyCapsule_GetPointer(
			gate_capsule, "qsimov.doki.funmatrix");

		if (state == NULL) {
			errno = 4;
			return NULL;
		}
		if (gate == NULL) {
			errno = 3;
			return NULL;
		}

		Py_INCREF(state_capsule);
		data->state_capsule = state_capsule;
		data->state = state;
		Py_INCREF(gate_capsule);
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

static void free_application(void *raw_app)
{
	struct Application *data = (struct Application *)raw_app;

	if (data == NULL) {
		return;
	}

	data->refcount--;
	if (data->refcount == 0) {
		Py_DECREF(data->state_capsule);
		data->state_capsule = NULL;
		data->state = NULL;
		Py_DECREF(data->gate_capsule);
		data->gate_capsule = NULL;
		data->gate = NULL;
		free(data->targets);
		data->targets = NULL;
		free(data->controls);
		data->controls = NULL;
		free(data->anticontrols);
		data->anticontrols = NULL;
		data->num_targets = 0;
		data->num_controls = 0;
		data->num_anticontrols = 0;
		free(data);
	}
}

static void *clone_application(void *raw_app)
{
	struct Application *data = (struct Application *)raw_app;

	if (data == NULL) {
		return NULL;
	}

	data->refcount++;
	return raw_app;
}

static size_t size_application(void *raw_app)
{
	size_t size;
	struct Application *data = (struct Application *)raw_app;

	if (data == NULL) {
		return 0;
	}
	size = sizeof(struct Application);
	size += FM_mem_size(data->state);
	size += FM_mem_size(data->gate);
	size += data->num_targets * sizeof(unsigned int);
	size += data->num_controls * sizeof(unsigned int);
	size += data->num_anticontrols * sizeof(unsigned int);

	return size;
}

struct FMatrix *apply_gate_fmat(PyObject *state_capsule, PyObject *gate_capsule,
				unsigned int *targets, unsigned int num_targets,
				unsigned int *controls,
				unsigned int num_controls,
				unsigned int *anticontrols,
				unsigned int num_anticontrols)
{
	struct FMatrix *pFM;
	struct Application *data = new_application(
		state_capsule, gate_capsule, targets, num_targets, controls,
		num_controls, anticontrols, num_anticontrols);

	if (data == NULL) {
		errno = 5;
		return NULL;
	}

	pFM = new_FunctionalMatrix(data->state->r, 1, &_ApplyGateFunction, data,
				   free_application, clone_application,
				   size_application);
	if (pFM == NULL) {
		errno = 1;
		free_application(data);
	}

	return pFM;
}

#ifndef _MSC_VER
__attribute__((const))
#endif
static COMPLEX_TYPE
_ApplyGateFunction(NATURAL_TYPE i,
#ifndef _MSC_VER
		   NATURAL_TYPE unused1 __attribute__((unused)),
		   NATURAL_TYPE unused2 __attribute__((unused)),
		   NATURAL_TYPE unused3 __attribute__((unused)),
#else
		   NATURAL_TYPE unused1, NATURAL_TYPE unused2,
		   NATURAL_TYPE unused3,
#endif
		   void *raw_app)
{
	int res;
	NATURAL_TYPE k;
	long long n;
	NATURAL_TYPE mask, row, reg_index = i;
	COMPLEX_TYPE val = COMPLEX_ZERO;
	struct Application *data = (struct Application *)raw_app;

	for (k = 0; k < data->num_controls; ++k) {
		mask = NATURAL_ONE << data->controls[k];
		if (!(i & mask)) {
			res = getitem(data->state, i, 0, &val);
			if (res != 0) {
				printf("Error[C] %d while getting state item " NATURAL_STRING_FORMAT
				       "\n",
				       res, i);
				return COMPLEX_NAN;
			}
			return val;
		}
	}

	for (k = 0; k < data->num_anticontrols; ++k) {
		mask = NATURAL_ONE << data->anticontrols[k];
		if (i & mask) {
			res = getitem(data->state, i, 0, &val);
			if (res != 0) {
				printf("Error[A] %d while getting state item " NATURAL_STRING_FORMAT
				       "\n",
				       res, i);
				return COMPLEX_NAN;
			}
			return val;
		}
	}

	for (n = 0; n < data->gate->r; ++n) {
		// We get the value of each target qubit id on the current new state
		// element and we store it in rowbits following the same order as the
		// one in targets
		COMPLEX_TYPE aux, aux2;

		row = 0;
		for (k = 0; k < data->num_targets; k++) {
			row += ((i & (NATURAL_ONE << data->targets[k])) != 0)
			       << k;
			// We check the value of the kth bit of j
			// and set the value of the kth target bit to it
			if ((n & (NATURAL_ONE << k)) != 0) {
				reg_index |= NATURAL_ONE << data->targets[k];
			} else {
				reg_index &= ~(NATURAL_ONE << data->targets[k]);
			}
		}
		res = getitem(data->state, reg_index, 0, &aux);
		if (res != 0) {
			printf("Error[T] %d while getting state[" NATURAL_STRING_FORMAT
			       "] item " NATURAL_STRING_FORMAT "\n",
			       res, i, reg_index);
			return COMPLEX_NAN;
		}
		res = getitem(data->gate, row, n, &aux2);
		if (res != 0) {
			printf("Error[T] %d while getting gate item " NATURAL_STRING_FORMAT
			       ", " NATURAL_STRING_FORMAT "\n",
			       res, row, n);
			return COMPLEX_NAN;
		}
		val = COMPLEX_ADD(val, COMPLEX_MULT(aux, aux2));
	}

	return val;
}

static size_t size_state_capsule(void *raw_capsule)
{
	struct state_vector *state;
	PyObject *capsule = (PyObject *)raw_capsule;

	if (capsule == NULL) {
		return 0;
	}

	state = (struct state_vector *)PyCapsule_GetPointer(
		capsule, "qsimov.doki.state_vector");

	return state_mem_size(state);
}

struct FMatrix *density_matrix(PyObject *state_capsule)
{
	struct FMatrix *dm = NULL;
	struct state_vector *state =
		PyCapsule_GetPointer(state_capsule, "qsimov.doki.state_vector");

	if (state != NULL) {
		dm = new_FunctionalMatrix(state->size, state->size,
					  &_densityFun, state_capsule,
					  free_capsule, clone_capsule,
					  size_state_capsule);
		if (dm != NULL) {
			Py_INCREF(state_capsule);
		} else {
			errno = 1;
		}
	} else {
		errno = 2;
	}

	return dm;
}
