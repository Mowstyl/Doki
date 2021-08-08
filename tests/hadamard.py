"""Hadamard tests."""
import doki
import numpy as np

sqrt2_2 = np.sqrt(2) / 2

raw_h = [[sqrt2_2, sqrt2_2], [sqrt2_2, -sqrt2_2]]
h = doki.gate(1, raw_h)

raw_x = [[0, 1], [1, 0]]
x = doki.gate(1, raw_x)

num_qubits = 2
r = doki.new(num_qubits)
print([doki.get(r, i) for i in range(2**num_qubits)])

doki.apply(r, h, {0}, None, set())
print([doki.get(r, i) for i in range(2**num_qubits)])

doki.apply(r, x, {1}, {0}, set())
print([doki.get(r, i) for i in range(2**num_qubits)])

doki.apply(r, x, {1}, {0}, None)
print([doki.get(r, i) for i in range(2**num_qubits)])

doki.apply(r, h, {1}, set(), None)
print([doki.get(r, i) for i in range(2**num_qubits)])

doki.apply(r, h, {0, 1}, set(), None)
