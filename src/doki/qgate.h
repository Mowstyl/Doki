/*
 * Doki: Quantum Computer simulator, using state vectors. QSimov core.
 * Copyright (C) 2021  Hernán Indíbil de la Cruz Calvo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
