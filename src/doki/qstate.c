#include "platform.h"
#include "qstate.h"


unsigned char
state_init(struct state_vector *this, unsigned int num_qubits, int init)
{
    size_t i, offset, errored_chunk;
    _Bool errored;

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
    }
    else {
        offset = COMPLEX_ARRAY_SIZE;
    }
    this->vector = MALLOC_TYPE(this->num_chunks, COMPLEX_TYPE*);
    if (this->vector == NULL) {
        return 1;
    }
    errored = 0;
    for (i = 0; i < this->num_chunks - 1; i++) {
        if (init) {
            this->vector[i] = CALLOC_TYPE(COMPLEX_ARRAY_SIZE, COMPLEX_TYPE);
        }
        else {
            this->vector[i] = MALLOC_TYPE(COMPLEX_ARRAY_SIZE, COMPLEX_TYPE);
        }
        if (this->vector[i] == NULL) {
            errored_chunk = i;
            errored = 1;
            break;
        }
    }
    if (!errored) {
        if (init) {
            this->vector[this->num_chunks-1] = CALLOC_TYPE(offset, COMPLEX_TYPE);
        }
        else {
            this->vector[this->num_chunks-1] = MALLOC_TYPE(offset, COMPLEX_TYPE);
        }
        if (this->vector[this->num_chunks-1] == NULL) {
            errored = 1;
            errored_chunk = this->num_chunks-1;
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


unsigned char
state_clone(struct state_vector *dest, struct state_vector *source)
{
    NATURAL_TYPE i;
    unsigned char exit_code;
    exit_code = state_init(dest, source->num_qubits, 0);
    if (exit_code != 0) {
        return exit_code;
    }
    #pragma omp parallel for default(none) \
                             shared (source, dest, exit_code) \
                             private (i)
    for (i = 0; i < source->size; i++) {
        dest->vector[i/COMPLEX_ARRAY_SIZE][i%COMPLEX_ARRAY_SIZE] = state_get(source, i);
    }
    return 0;
}


void
state_clear(struct state_vector *this)
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

void
state_set(struct state_vector *this, NATURAL_TYPE i, COMPLEX_TYPE value)
{
    this->vector[i/COMPLEX_ARRAY_SIZE][i%COMPLEX_ARRAY_SIZE] = value;
}


COMPLEX_TYPE
state_get(struct state_vector *this, NATURAL_TYPE i)
{
    COMPLEX_TYPE val;
    val = complex_div_r(this->vector[i/COMPLEX_ARRAY_SIZE][i%COMPLEX_ARRAY_SIZE], this->norm_const);
    return fix_value(val, -1, -1, 1, 1);
}
