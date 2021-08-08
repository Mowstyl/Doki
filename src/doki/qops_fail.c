#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include "platform.h"
#include "qstate.h"
#include "qgate.h"
#include "qops.h"

unsigned char
apply_gate(struct state_vector *state, struct qgate *gate,
           unsigned int *targets, unsigned int num_targets,
           unsigned int *controls, unsigned int num_controls,
           unsigned int *anticontrols, unsigned int num_anticontrols)
{
    struct state_vector *new_state;
    NATURAL_TYPE i, reg_index;
    COMPLEX_TYPE sum, get, old_value, new_value;
    REAL_TYPE norm_const;
    unsigned char exit_code, aux_code;
    unsigned int j, k, row, copy_only;

    new_state = MALLOC_TYPE(1, struct state_vector);
    if (new_state == NULL) {
        return 10;
    }
    exit_code = state_init(new_state, state->num_qubits, 0);
    // 0 -> OK
    // 1 -> Error initializing chunk
    // 2 -> Error allocating chunk
    // 3 -> Error setting values (should never happens since init = 0)
    // 4 -> Error allocating state
    if (exit_code != 0) {
        free(new_state);
        return exit_code;
    }

    norm_const = 0;
    aux_code = 0;
    // We can calculate each element of the new state separately
    // Cambiar esto a parallel for
    #pragma omp parallel for reduction (+:norm_const) \
                             reduction (&:aux_code) \
                             default(none) \
                             shared (state, new_state, gate, \
                                     targets, num_targets, \
                                     controls, num_controls, \
                                     anticontrols, num_anticontrols, \
                                     exit_code, norm_const, aux_code) \
                             private (copy_only, sum, old_value, new_value, row, reg_index, i, j)
    for (i = 0; i < state->size; i++) {
        // If there has been any error in this thread, we skip
        if (aux_code != 0) {
            continue;
        }
        copy_only = 0;

        for (j = 0; j < num_controls; j++) {
            /* If any control bit is set to 0 we set next to true */
            if ((i & (NATURAL_ONE << controls[j])) == 0) {
                copy_only = 1;
                break;
            }
        }
        if (copy_only == 0) {
            for (j = 0; j < num_anticontrols; j++) {
                /* If any anticontrol bit is not set to 0 we set next to true */
                if ((i & (NATURAL_ONE << anticontrols[j])) != 0) {
                    copy_only = 1;
                    break;
                }
            }
        }
        aux_code = state_get(state, i, &old_value);
        if (aux_code > 1) {
            printf("Failed to get old state value for copy");
            aux_code = 1;
        }
        reg_index = i;
        sum = complex_init(0, 0);
        // We have gate->size (same as num_targets) elements to add in sum
        for (j = 0; j < num_targets; j++) {
            // We get the value of each target qubit id on the current new state element
            // and we store it in rowbits following the same order as the one in targets
            row = 0;
            for (k = 0; k < num_targets; k++) {
                row += ((i & (NATURAL_ONE << targets[k])) != 0) << k;
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
                printf("Failed to get old state value");
                aux_code = 1;
                break;
            }
            sum = complex_sum(sum, complex_mult(get, gate->matrix[row][j]));
        }
        printf("[DEBUG] Item %d\n", i);
        printf("\t[DEBUG] Copy only: %d\n", copy_only);
        printf("\t[DEBUG] Old value: %lf + i%lf\n", creal(old_value), cimag(old_value));
        printf("\t[DEBUG] Calculated: %lf + i%lf\n", creal(sum), cimag(sum));
        new_value = complex_sum(complex_mult_r(old_value, copy_only), complex_mult_r(sum, 1 - copy_only));
        printf("\t[DEBUG] New value: %lf + i%lf\n", creal(new_value), cimag(new_value));
        // norm_const += cabs(new_value) * cabs(new_value);
        norm_const += pow(creal(new_value), 2) + pow(cimag(new_value), 2);
        if (state_set(new_state, i, new_value) > 1) {
            printf("Failed to set new state value");
            aux_code = 1;
            continue;
        }
    }
    if (aux_code == 0) {
        new_state->norm_const = sqrt(norm_const);
    }
    else {
        exit_code = 5;
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
    state_clear(new_state);
    free(new_state);

    return exit_code;
}

void
get_bits_ui (_Bool *bits, unsigned int num_bits, unsigned int n)
{
    unsigned int i, aux;

    aux = n;
    for (i = 0; i < num_bits; i++) {
        bits[i] = aux & 1;
        aux = aux >> 1;
    }
}

unsigned int
get_ui_bits (_Bool *bits, unsigned int num_bits)
{
    unsigned int j, result;
    result = 0;
    for (j = 0; j < num_bits; j++) {
        result += bits[num_bits - j - 1];
        if (j > 0) {
            result = result << 1;
        }
    }
    return result;
}
