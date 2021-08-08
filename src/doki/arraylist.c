#include <stdlib.h>
#include <stdio.h>
#include "platform.h"
#include "arraylist.h"


unsigned char
node_chunk(struct node_e *this)
{
    /*
    Returns 0 if everything went well.
    Returns 1 if malloc for node_elements failed.
    */
    this->node_elements = MALLOC_TYPE(this->node_size, NATURAL_TYPE);
    if (this->node_elements == NULL) {
        return 1;
    }

    return 0;
}

unsigned char
node_new(struct node_e *this, NATURAL_TYPE size)
{
    NATURAL_TYPE chunk_size, num_chunks, i, remainder;
    struct node_e *last;

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
        if (node_chunk(last) != 0) {
            last->next = NULL;
            node_clear(this);
            return 1;  // Error 1 means failed to initialize chunk
        }
        last->next = MALLOC_TYPE(1, struct node_e);
        if (last->next == NULL) {
            node_clear(this);
            return 2;  // Error 2 means failed to allocate chunk
        }
        last = last->next;
    }
    if (remainder > 0) {
        last->next = NULL;
        last->node_size = remainder;
        if (node_chunk(last) != 0) {
            /* Error 1 means failed to initialize chunk */
            return 1;
        }
    }

    return 0;
}

void
node_clear(struct node_e *this)
{
    struct node_e *curr, *next;

    next = this;
    // We free memory of the complex array for each chunk
    while (next != NULL) {
        curr = next;
        free(curr->node_elements);
        next = curr->next;
        free(curr);
    }
}

unsigned char
alist_init(struct array_list_e *this, NATURAL_TYPE size)
{
    unsigned char vector_result;

    this->first_id = 0;
    this->size = size;
    this->last_id = this->size - 1;
    this->vector = MALLOC_TYPE(1, struct node_e);

    vector_result = 4;
    if (this->vector != NULL) {
        vector_result = node_new(this->vector, this->size);
    }

    if (vector_result > 0) {
        alist_clear(this);
    }

    return vector_result;
}


void
alist_clear(struct array_list_e *this)
{
    if (this->vector != NULL) {
        node_clear(this->vector);
    }
}

unsigned char
alist_set(struct array_list_e *this, NATURAL_TYPE index, NATURAL_TYPE value)
{
    NATURAL_TYPE i, position, chunk_id, partial_id;
    struct node_e *chunk;

    if (index >= this->size) {
        return 2;  // 1 means index out of bounds
    }

    if (index < this->first_id || index > this->last_id) {
        /* Not in this computation node */
        return 1;
    }

    partial_id = index - this->first_id;
    chunk_id = partial_id / this->vector->node_size;
    position = partial_id % this->vector->node_size;
    chunk = this->vector;
    for (i = 0; i < chunk_id; i++) {
        chunk = chunk->next;
    }
    chunk->node_elements[position] = value;

    return 0;
}

unsigned char
alist_get(struct array_list_e *this, NATURAL_TYPE index, NATURAL_TYPE *target)
{
    NATURAL_TYPE i, position, chunk_id, partial_id;
    struct node_e *chunk;

    if (index >= this->size) {
        return 2;  // 1 means index out of bounds
    }

    if (index < this->first_id || index > this->last_id) {
        /* Not in this computation node */
        return 1;
    }

    partial_id = index - this->first_id;
    chunk_id = partial_id / this->vector->node_size;
    position = partial_id % this->vector->node_size;
    chunk = this->vector;
    for (i = 0; i < chunk_id; i++) {
        chunk = chunk->next;
    }
    *target = chunk->node_elements[position];

    return 0;
}
