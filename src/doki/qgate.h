#pragma once
#ifndef QGATE_H_
#define QGATE_H_

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

#endif /* QGATE_H_ */
