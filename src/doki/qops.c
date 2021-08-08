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
apply_gate(struct state_vector *state, struct qgate *gate,
           unsigned int *targets, unsigned int num_targets,
           unsigned int *controls, unsigned int num_controls,
           unsigned int *anticontrols, unsigned int num_anticontrols)
{
    struct state_vector *new_state;
    struct array_list_e *not_copy;
    REAL_TYPE norm_const;
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

    new_state = MALLOC_TYPE(1, struct state_vector);
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
        }
        else {
            exit_code = 5;
        }
    }
    else {
        exit_code = 6;
    }

    if (exit_code == 0) {
        state_clear(state);
        state->first_id = new_state->first_id;
        state->last_id = new_state->last_id;
        state->size = new_state->size;
        state->vector = new_state->vector;
        state->norm_const = new_state->norm_const;
        new_state->vector = NULL;
    }
    alist_clear(not_copy);
    free(not_copy);
    state_clear(new_state);
    free(new_state);

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
                             shared (state, count, not_copy, new_state, gate, \
                                     targets, num_targets, \
                                     controls, num_controls, \
                                     anticontrols, num_anticontrols, \
                                     exit_code, norm_const) \
                             private (curr_id, copy_only, get, sum, row, reg_index, i, j, k)
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
            exit_code = state_get(state, i, &get);
            if (exit_code > 1) {
                printf("Failed to get old state value for copy\n");
                exit_code = 1;
            }
            *norm_const += pow(creal(get), 2) + pow(cimag(get), 2);
            if (state_set(new_state, i, get) > 1) {
                printf("Failed to copy value to new state\n");
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
                                     norm_const, aux_code) \
                             private (curr_id, copy_only, get, sum, row, reg_index, i, j, k)
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
            aux_code = state_get(state, reg_index, &get);
            if (aux_code == 2) {
                printf("Failed to get old state value\n");
                aux_code = 1;
                break;
            }
            sum = complex_sum(sum, complex_mult(get, gate->matrix[row][j]));
        }
        // norm_const += cabs(sum) * cabs(sum);
        *norm_const += pow(creal(sum), 2) + pow(cimag(sum), 2);
        if (state_set(new_state, curr_id, sum) > 1) {
            printf("Failed to set new state value\n");
            aux_code = 1;
            continue;
        }
    }
    return aux_code;
}
