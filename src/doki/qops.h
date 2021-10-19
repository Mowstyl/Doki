#ifndef __QOPS_H
#define __QOPS_H

#include "arraylist.h"
#include "qstate.h"
#include "qgate.h"
#include "funmatrix.h"

unsigned char
join(struct state_vector *r, struct state_vector *s1, struct state_vector *s2);

unsigned char
measure(struct state_vector *state, _Bool *result, unsigned int target,
        struct state_vector *new_state, REAL_TYPE roll);

REAL_TYPE
probability(struct state_vector *state, unsigned int target_id);

REAL_TYPE
get_global_phase(struct state_vector *state);

unsigned char
collapse(struct state_vector *state, unsigned int id, _Bool value,
         struct state_vector *new_state);

unsigned char
apply_gate(struct state_vector *state, struct qgate *gate,
           unsigned int *targets, unsigned int num_targets,
           unsigned int *controls, unsigned int num_controls,
           unsigned int *anticontrols, unsigned int num_anticontrols,
           struct state_vector *new_state);

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
            void *rawstate);

FunctionalMatrix*
densityMat(struct state_vector *state);

#endif
