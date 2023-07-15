#ifndef __QGATE_H
#define __QGATE_H

#include "qstate.h"

struct qgate
{
  /* number of qubits affected by this gate */
  unsigned int num_qubits;
  /* number of rows (or columns) in this gate */
  NATURAL_TYPE size;
  /* matrix that represents the gate */
  COMPLEX_TYPE **matrix;
};

#endif
