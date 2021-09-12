#include <stdlib.h>
#include <stdio.h>
#include "platform.h"
#include "qstate.h"


unsigned char
list_chunk(struct array_list *this, int init)
{
    /*
    Returns 0 if everything went well.
    Returns 1 if malloc for node_elements failed.
    */
    if (!init) {
        this->node_elements = MALLOC_TYPE(this->node_size, COMPLEX_TYPE);
    }
    else {
        this->node_elements = CALLOC_TYPE(this->node_size, COMPLEX_TYPE);
    }
    if (this->node_elements == NULL) {
        return 1;
    }

    return 0;
}

unsigned char
list_new(struct array_list *this, NATURAL_TYPE size, int init)
{
    NATURAL_TYPE chunk_size, num_chunks, i, remainder;
    struct array_list *last;

    if (size <= CHUNK_MAX) {
        num_chunks = 1;
        remainder = size;
    }
    else {
        num_chunks = size / CHUNK_MAX;
        remainder = size % CHUNK_MAX;
        chunk_size = CHUNK_MAX;
        if (remainder > 0) {
            num_chunks++;
        }
    }
    last = this;
    for (i = 0; i < num_chunks - 1; i++) {
        last->node_size = chunk_size;
        if (list_chunk(last, init) != 0) {
            last->next = NULL;
            list_clear(this);
            return 1;  // Error 1 means failed to initialize chunk
        }
        last->next = MALLOC_TYPE(1, struct array_list);
        if (last->next == NULL) {
            list_clear(this);
            return 2;  // Error 2 means failed to allocate chunk
        }
        last = last->next;
    }
    if (remainder > 0) {
        last->next = NULL;
        last->node_size = remainder;
        if (list_chunk(last, init) != 0) {
            /* Error 1 means failed to initialize chunk */
            return 1;
        }
    }

    return 0;
}

void
list_clear(struct array_list *this)
{
    struct array_list *curr, *next;

    next = this;
    // We free memory of the complex array for each chunk
    // printf("[DEBUG] FREE LIST!\n");
    while (next != NULL) {
        curr = next;
        free(curr->node_elements);
        next = curr->next;
        free(curr);
    }
}

unsigned char
state_init(struct state_vector *state, unsigned int num_qubits, int init)
{
    unsigned char vector_result;

    state->first_id = 0;
    state->size = NATURAL_ONE << num_qubits;
    state->last_id = state->size - 1;
    state->num_qubits = num_qubits;
    state->norm_const = 1;
    state->vector = MALLOC_TYPE(1, struct array_list);
    if (state->vector != NULL) {
        vector_result = list_new(state->vector, state->size, init);
    }
    else {
        vector_result = 4;
    }
    if (vector_result == 0 && init) {
        vector_result = state_set(state, 0, complex_init(1, 0));
        if (vector_result != 0) {
            vector_result += 2;
        }
    }
    if (vector_result > 0) {
        state_clear(state);
    }

    return vector_result;
}


void
state_clear(struct state_vector *state)
{
    // printf("[DEBUG] IT'S FREE REAL STATE!\n");
    if (state->vector != NULL) {
        list_clear(state->vector);
    }
    state->num_qubits = 0;
    state->size = 0;
    state->first_id = 0;
    state->last_id = 0;
    state->norm_const = 0.0;
    state->vector = NULL;
}

unsigned char
state_set(struct state_vector *state, NATURAL_TYPE index, COMPLEX_TYPE value)
{
    NATURAL_TYPE i, position, chunk_id, partial_id;
    struct array_list *chunk;

    if (index >= state->size) {
        return 2;  // 1 means index out of bounds
    }

    if (index < state->first_id || index > state->last_id) {
        /* Not in this computation node */
        return 1;
    }

    partial_id = index - state->first_id;
    chunk_id = partial_id / state->vector->node_size;
    position = partial_id % state->vector->node_size;
    chunk = state->vector;
    for (i = 0; i < chunk_id; i++) {
        chunk = chunk->next;
    }
    chunk->node_elements[position] = value;

    return 0;
}

unsigned char
state_get(struct state_vector *state, NATURAL_TYPE index, COMPLEX_TYPE *target)
{
    NATURAL_TYPE i, position, chunk_id, partial_id;
    COMPLEX_TYPE aux;
    struct array_list *chunk;

    if (index >= state->size) {
        printf("[DEBUG] state->size: %llu\n", state->size);
        printf("[DEBUG] index: %llu\n", index);
        return 2;  // 1 means index out of bounds
    }

    if (index < state->first_id || index > state->last_id) {
        /* Not in this computation node */
        return 1;
    }

    partial_id = index - state->first_id;
    chunk_id = partial_id / state->vector->node_size;
    position = partial_id % state->vector->node_size;
    chunk = state->vector;
    for (i = 0; i < chunk_id; i++) {
        chunk = chunk->next;
    }
    aux = complex_div_r(chunk->node_elements[position], state->norm_const);
    *target = fix_value(aux);

    return 0;
}
