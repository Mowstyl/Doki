#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

#include "platform.h"
#include "qstate.h"
#include "arraylist.h"
#include "qgate.h"
#include "qops.h"


unsigned char
probability(struct state_vector *state, unsigned int target_id, REAL_TYPE *value)
{
    NATURAL_TYPE i, step, count;
    COMPLEX_TYPE aux;
    _Bool read;
    unsigned char exit_code;

    step = NATURAL_ONE << target_id;
    read = 0;
    count = 0;
    *value = 0;

    for (i = 0; i < state->size; i++) {
        if (read != 0) {
            exit_code = state_get(state, i, &aux, 0);
            if (exit_code != 0) {
                break;
            }
            *value += pow(creal(aux), 2) + pow(cimag(aux), 2);
        }
        count++;
        if (count == step) {
            read = !read;
            count = 0;
        }
    }

    return exit_code;
}

unsigned char
join(struct state_vector *r, struct state_vector *s1, struct state_vector *s2)
{
    NATURAL_TYPE i, j, new_index;
    COMPLEX_TYPE o1, o2, aux;
    REAL_TYPE arg;
    unsigned char exit_code, aux_code;
    _Bool errored;

    exit_code = state_init(r, s1->num_qubits + s2->num_qubits, 0);
    if (exit_code != 0) {
        return exit_code;
    }
    aux_code = 0;
    errored = 0;
    #pragma omp parallel for reduction (|:exit_code) \
                             default(none) \
                             shared (r, s1, s2) \
                             private (i, o1, aux_code, new_index, errored)
    for (i = 0; i < s1->size; i++) {
        if (aux_code != 0) {
            continue;
        }
        aux_code |= state_get(s1, i, &o1, 0);
        if (aux_code != 0 && !errored) {
            errored = 1;
            printf("Failed to get state element %llu from s1\n", i);
        }
        for (j = 0; j < s2->size; j++) {
            if (aux_code != 0) {
                break;
            }
            new_index = i * s2->size + j;
            aux_code |= state_get(s2, j, &o2, 0);
            if (aux_code != 0 && !errored) {
                errored = 1;
                printf("Failed to get state element %llu from s2\n", j);
            }
            aux_code |= state_set(r, new_index, complex_mult(o1, o2));
            if (aux_code != 0 && !errored) {
                errored = 1;
                printf("Failed to set state element %llu\n", new_index);
            }
        }
        exit_code |= aux_code;
    }
    if (exit_code != 0) {
        state_clear(r);
        return 5;
    }
    else {
        exit_code = state_get(r, 0, &aux, 0);
        if (exit_code == 0) {
            arg = carg(aux);
            r->fcarg = complex_init(cos(arg), -sin(arg));
        }
    }
    return 0;
}

unsigned char
measure(struct state_vector *state, _Bool *result, unsigned int target,
        struct state_vector *new_state, REAL_TYPE roll)
{
    NATURAL_TYPE i, count, step;
    COMPLEX_TYPE aux;
    REAL_TYPE sum;
    unsigned char toggle, exit_code;

    toggle = 0;
    count = 0;
    // rand(); // If the seeds are close the first random will be close
    // roll = (REAL_TYPE) rand() / (REAL_TYPE) RAND_MAX;
    // Value of bit changes each step (2^target)
    step = NATURAL_ONE << target;
    exit_code = 0;
    sum = 0;
    // printf("[DEBUG] Zero chance ");
    for (i = 0; i < state->size; i++) {
        if (sum > roll || exit_code != 0) {
            // printf(">");
            break;
        }
        if (toggle != 0) {
            sum += 0;
        }
        else {
            exit_code = state_get(state, i, &aux, 0);
            sum += pow(creal(aux), 2) + pow(cimag(aux), 2);
        }
        count++;
        if (count == step) {
            count = 0;
            toggle = !toggle;
        }
    }
    // printf("= %lf\n", sum);
    // printf("[DEBUG] Roll: %lf\n", roll);
    if (exit_code == 0) {
        // printf("= %lf\n", sum);
        // printf("[DEBUG] Roll: %lf\n", roll);
        *result = sum <= roll;
        exit_code = collapse(state, target, *result, new_state);
        if (exit_code != 0) {
            exit_code += 2;  // Max code from state_get is 2
        }
    }

    return exit_code;
}

unsigned char
collapse(struct state_vector *state, unsigned int target_id, _Bool value,
         struct state_vector *new_state)
{
    unsigned char exit_code;
    NATURAL_TYPE i, j, count, step;
    _Bool toggle;
    REAL_TYPE norm_const;
    COMPLEX_TYPE aux;
    REAL_TYPE arg;

    if (state->num_qubits == 1) {
        // state_clear(state);
        new_state->vector = NULL;
        new_state->num_qubits = 0;
        return 0;
    }

    /*
    new_state = MALLOC_TYPE(1, struct state_vector);
    if (new_state == NULL) {
        return 5;
    }
    */
    exit_code = state_init(new_state, state->num_qubits - 1, 0);
    if (exit_code != 0) {
        free(new_state);
        return exit_code;
    }
    norm_const = 0;
    toggle = 0;
    count = 0;
    step = NATURAL_ONE << target_id;
    j = 0;
    for (i = 0; i < state->size; i++) {
        if (exit_code != 0) {
            break;
        }
        if (toggle == value) {
            exit_code |= state_get(state, i, &aux, 0);
            exit_code |= state_set(new_state, j, aux);
            norm_const += pow(creal(aux), 2) + pow(cimag(aux), 2);
            j++;
        }
        count++;
        if (count == step) {
            count = 0;
            toggle = !toggle;
        }
    }

    if (exit_code == 0) {
        /*
        state_clear(state);
        state->first_id = new_state->first_id;
        state->last_id = new_state->last_id;
        state->size = new_state->size;
        state->num_qubits = new_state->num_qubits;
        state->vector = new_state->vector;
        */
        new_state->norm_const = sqrt(norm_const);
        exit_code = state_get(new_state, 0, &aux, 0);
        if (exit_code == 0) {
            arg = carg(aux);
            new_state->fcarg = complex_init(cos(arg), -sin(arg));
        }
        // new_state->vector = NULL;
    }
    // state_clear(new_state);
    // free(new_state);

    return exit_code;
}

unsigned char
apply_gate(struct state_vector *state, struct qgate *gate,
           unsigned int *targets, unsigned int num_targets,
           unsigned int *controls, unsigned int num_controls,
           unsigned int *anticontrols, unsigned int num_anticontrols,
           struct state_vector *new_state)
{
    struct array_list_e *not_copy;
    REAL_TYPE norm_const, arg;
    COMPLEX_TYPE aux;
    unsigned char exit_code;

    not_copy = MALLOC_TYPE(1, struct array_list_e);
    if (not_copy == NULL) {
        return 11;
    }
    exit_code = alist_init(not_copy, state->size >> (num_controls + num_anticontrols));

    if (exit_code != 0) {
        free(not_copy);
        return exit_code;
    }

    if (new_state == NULL) {
        alist_clear(not_copy);
        free(not_copy);
        return 10;
    }
    exit_code = state_init(new_state, state->num_qubits, 0);
    // 0 -> OK
    // 1 -> Error initializing chunk
    // 2 -> Error allocating chunk
    // 3 -> Error setting values (should never happens since init = 0)
    // 4 -> Error allocating state
    if (exit_code != 0) {
        alist_clear(not_copy);
        free(not_copy);
        free(new_state);
        return exit_code;
    }

    norm_const = 0;
    exit_code = copy_and_index(state, new_state,
                               controls, num_controls,
                               anticontrols, num_anticontrols,
                               &norm_const, not_copy);

    if (exit_code == 0) {
        exit_code = calculate_empty(state, gate, targets, num_targets,
                                    controls, num_controls,
                                    anticontrols, num_anticontrols,
                                    new_state, not_copy, &norm_const);
        if (exit_code == 0) {
            new_state->norm_const = sqrt(norm_const);
            exit_code = state_get(new_state, 0, &aux, 0);
            if (exit_code == 0) {
                arg = carg(aux);
                new_state->fcarg = complex_init(cos(arg), -sin(arg));
            }
        }
        else {
            exit_code = 5;
        }
    }
    else {
        exit_code = 6;
    }

    alist_clear(not_copy);
    free(not_copy);

    return exit_code;
}

unsigned char
copy_and_index(struct state_vector *state, struct state_vector *new_state,
               unsigned int *controls, unsigned int num_controls,
               unsigned int *anticontrols, unsigned int num_anticontrols,
               REAL_TYPE *norm_const, struct array_list_e *not_copy)
{
    NATURAL_TYPE i, count;
    unsigned int j;
    COMPLEX_TYPE get;
    unsigned char exit_code, copy_only;
    *norm_const = 0;
    exit_code = 0;
    count = 0;
    #pragma omp parallel for reduction (+:count) \
                             reduction (|:exit_code) \
                             default(none) \
                             shared (state, not_copy, new_state, \
                                     controls, num_controls, \
                                     anticontrols, num_anticontrols, \
                                     norm_const) \
                             private (copy_only, get, i, j)
    for (i = 0; i < state->size; i++) {
        // If there has been any error in this thread, we skip
        if (exit_code != 0) {
            break;
        }
        copy_only = 0;

        for (j = 0; j < num_controls; j++) {
            /* If any control bit is set to 0 we set copy_only to true */
            if ((i & (NATURAL_ONE << controls[j])) == 0) {
                copy_only = 1;
                break;
            }
        }
        if (!copy_only) {
            for (j = 0; j < num_anticontrols; j++) {
                /* If any anticontrol bit is not set to 0 we set copy_only to true */
                if ((i & (NATURAL_ONE << anticontrols[j])) != 0) {
                    copy_only = 1;
                    break;
                }
            }
        }
        // If copy_only is true it means that we just need to copy the old state element
        if (copy_only) {
            exit_code = state_get(state, i, &get, 0);
            if (exit_code > 1) {
                // printf("[DEBUG] Failed to get old state value for copy\n");
                exit_code = 1;
            }
            *norm_const += pow(creal(get), 2) + pow(cimag(get), 2);
            if (state_set(new_state, i, get) > 1) {
                // printf("[DEBUG] Failed to copy value to new state\n");
                exit_code = 1;
                continue;
            }
        }
        else {
            exit_code = alist_set(not_copy, count, i);
            if (exit_code > 0) {
                continue;
            }
            count++;
        }
    }

    return exit_code;
}

unsigned char
calculate_empty(struct state_vector *state, struct qgate *gate,
                unsigned int *targets, unsigned int num_targets,
                unsigned int *controls, unsigned int num_controls,
                unsigned int *anticontrols, unsigned int num_anticontrols,
                struct state_vector *new_state, struct array_list_e *not_copy,
                REAL_TYPE *norm_const)
{
    NATURAL_TYPE i, reg_index, curr_id;
    COMPLEX_TYPE sum, get;
    unsigned char aux_code;
    unsigned int j, k, row;
    aux_code = 0;
    // We can calculate each element of the new state separately
    #pragma omp parallel for reduction (|:aux_code) \
                             default(none) \
                             shared (state, not_copy, new_state, gate, \
                                     targets, num_targets, \
                                     controls, num_controls, \
                                     anticontrols, num_anticontrols, \
                                     norm_const) \
                             private (curr_id, get, sum, row, reg_index, i, j, k)
    for (i = 0; i < not_copy->size; i++) {
        // If there has been any error in this thread, we skip
        if (aux_code != 0) {
            continue;
        }
        alist_get(not_copy, i, &curr_id);
        reg_index = curr_id;
        sum = complex_init(0, 0);
        // We have gate->size elements to add in sum
        for (j = 0; j < gate->size; j++) {
            // We get the value of each target qubit id on the current new state element
            // and we store it in rowbits following the same order as the one in targets
            row = 0;
            for (k = 0; k < num_targets; k++) {
                row += ((curr_id & (NATURAL_ONE << targets[k])) != 0) << k;
            }
            for (k = 0; k < gate->num_qubits; k++) {
                // We check the value of the kth bit of j
                // and set the value of the kth target bit to it
                if ((j & (NATURAL_ONE << k)) != 0) {
                    reg_index |= NATURAL_ONE << targets[k];
                }
                else {
                    reg_index &= ~(NATURAL_ONE << targets[k]);
                }
            }
            aux_code = state_get(state, reg_index, &get, 0);
            if (aux_code == 2) {
                // printf("[DEBUG] Failed to get old state value\n");
                aux_code = 1;
                break;
            }
            sum = complex_sum(sum, complex_mult(get, gate->matrix[row][j]));
        }
        // norm_const += cabs(sum) * cabs(sum);
        *norm_const += pow(creal(sum), 2) + pow(cimag(sum), 2);
        if (state_set(new_state, curr_id, sum) > 1) {
            // printf("[DEBUG] Failed to set new state value\n");
            aux_code = 1;
            continue;
        }
    }
    return aux_code;
}

#ifndef _MSC_VER
__attribute__ ((const))
#endif
COMPLEX_TYPE
_densityFun(NATURAL_TYPE i, NATURAL_TYPE j,
            #ifndef _MSC_VER
            NATURAL_TYPE unused1 __attribute__((unused)), NATURAL_TYPE unused2 __attribute__((unused)),
            #else
            NATURAL_TYPE unused1, NATURAL_TYPE unused2,
            #endif
            void *rawstate)
{
    struct state_vector *state = (struct state_vector*) rawstate;
    COMPLEX_TYPE elem_i,
                 elem_j,
                 result;
    unsigned char error_code;

    error_code = state_get(state, i, &elem_i, 0);
    error_code |= state_get(state, j, &elem_j, 0);

    if (error_code == 0) {
        result = complex_mult(elem_i, conj(elem_j));
    }
    else {
        result = complex_init(NAN, NAN);
    }

    return result;
}

FunctionalMatrix*
densityMat(struct state_vector *state)
{
    FunctionalMatrix *dm = NULL;

    if (state != NULL) {
        dm = new_FunctionalMatrix(state->size, state->size, &_densityFun, state);
    }

    return dm;
}
