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
    COMPLEX_TYPE sum, get;
    double norm_const, aux;
    unsigned char exit_code, aux_code, next;
    _Bool *colbits, *rowbits;
    unsigned int j, k, row;

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
    #pragma omp parallel default(none) shared (state, new_state, gate, \
                                               targets, num_targets, \
                                               controls, num_controls, \
                                               anticontrols, num_anticontrols, \
                                               exit_code, norm_const) \
                                       private (aux, aux_code, next, sum, row, i, j, colbits, rowbits)
    {
        aux_code = 0;
        // Cargarme colbits y rowbits, se puede poner incluso dentro de la inicialización del for
        colbits = MALLOC_TYPE(gate->num_qubits, _Bool);
        rowbits = MALLOC_TYPE(gate->num_qubits, _Bool);
        if (colbits == NULL || rowbits == NULL) {
            printf("Failed column/row bit array allocation");
            aux_code = 5;
        }

        if (aux_code == 0) {
            // We can calculate each element of the new state separately
            // Cambiar esto a parallel for
            #pragma omp for reduction (+:norm_const)
            for (i = 0; i < state->size; i++) {
                // If there has been any error in this thread, we skip
                if (aux_code != 0) {
                    continue;
                }
                next = 0;

                for (j = 0; j < num_controls; j++) {
                    /* If any control bit is set to 0 we set next to true */
                    if ((i & (NATURAL_ONE << controls[j])) == 0) {
                        next = 1;
                        break;
                    }
                }
                if (!next) {
                    for (j = 0; j < num_anticontrols; j++) {
                        /* If any anticontrol bit is not set to 0 we set next to true */
                        if ((i & (NATURAL_ONE << anticontrols[j])) != 0) {
                            next = 1;
                            break;
                        }
                    }
                }
                // If next is true it means that we just need to copy the old state element
                if (next) {
                    aux_code = state_get(state, i, &get);
                    if (aux_code <= 1) {
                        aux_code = state_set(new_state, i, get);
                        if (aux_code <= 1) {
                            aux_code = 0;
                            aux = cabs(get);
                            norm_const += aux * aux;
                        }
                        else {
                            printf("Failed to copy old state value to new state");
                            aux_code = 7;
                        }
                    }
                    else {
                        printf("Failed to get old state value for copy");
                        aux_code = 6;
                    }
                    continue;
                }
                // Mirar para sacar lo anterior para CPU y lo posterior para GPU (sin ramas)
                if (aux_code == 0) {
                    reg_index = i;
                    sum = complex_init(0, 0);
                    // We have gate->size elements to add in sum
                    for (j = 0; j < gate->size; j++) {
                        // We get the value of each target qubit id on the current new state element
                        // and we store it in rowbits following the same order as the one in targets
                        for (k = 0; k < num_targets; k++) {
                            rowbits[k] = i & (NATURAL_ONE << targets[k]);
                            printf("[DEBUG]: bit %d -> %d\n", k, rowbits[k]);
                        }
                        // rowbits (base 2) is the row of the gate matrix we are using
                        row = get_ui_bits(rowbits, num_targets);

                        // We turn j into base 2 and store it in colbits
                        get_bits_ui(colbits, gate->num_qubits, j);
                        for (k = 0; k < gate->num_qubits; k++) {
                            if (colbits[k]) {
                                reg_index |= NATURAL_ONE << targets[k];
                            }
                            else {
                                reg_index &= ~(NATURAL_ONE << targets[k]);
                            }
                        }
                        // printf("[DEBUG]: (i, j) -> (%d, %d)\n", i, j);
                        printf("[DEBUG]: Row %d\n", row);
                        // printf("[DEBUG]: RegID %d\n", reg_index);
                        aux_code = state_get(state, reg_index, &get);
                        if (aux_code == 2) {
                            printf("Failed to get old state value");
                            aux_code = 8;
                            break;
                        }
                        sum = complex_sum(sum, complex_mult(get, gate->matrix[row][j]));
                    }
                    aux = cabs(sum);
                    norm_const += aux * aux;
                    if (state_set(new_state, i, sum) > 1) {
                        printf("Failed to set new state value");
                        aux_code = 9;
                        continue;
                    }
                }
            }
            if (aux_code == 0) {
                new_state->norm_const = sqrt(norm_const);
            }
        }
        if (aux_code != 5) {
            free(colbits);
            free(rowbits);
        }
        if (aux_code != 0) {
            exit_code = 5;
        }
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
