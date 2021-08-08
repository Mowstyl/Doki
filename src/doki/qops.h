#ifndef __QOPS_H
#define __QOPS_H

#include "qstate.h"
#include "qgate.h"

unsigned char
apply_gate(struct state_vector *state, struct qgate *gate,
           unsigned int *targets, unsigned int num_targets,
           unsigned int *controls, unsigned int num_controls,
           unsigned int *anticontrols, unsigned int num_anticontrols);

unsigned char
copy_and_index(struct state_vector *state, struct state_vector *new_state,
               unsigned int *controls, unsigned int num_controls,
               unsigned int *anticontrols, unsigned int num_anticontrols,
               REAL_TYPE *norm_const, struct array_list_e *not_copy);

unsigned char
calculate_empty(struct state_vector *state, struct qgate *gate,
                unsigned int *targets, unsigned int num_targets,
                unsigned int *controls, unsigned int num_controls,
                unsigned int *anticontrols, unsigned int num_anticontrols,
                struct state_vector *new_state, struct array_list_e *not_copy,
                REAL_TYPE *norm_const);

#endif
