"""Hadamard tests."""
import doki
import numpy as np
import time as t

# seed = 6470
seed = int(t.time() * 1000)
print("Seed:", seed)
doki.seed(seed)

sqrt2_2 = np.sqrt(2) / 2

raw_h = [[sqrt2_2, sqrt2_2], [sqrt2_2, -sqrt2_2]]
h = doki.gate(1, raw_h)

raw_x = [[0, 1], [1, 0]]
x = doki.gate(1, raw_x)

num_qubits = 2
r = doki.new(num_qubits)
r2 = doki.apply(r, h, {0}, None, set())
del r
print("R1:", [doki.get(r2, i) for i in range(2**num_qubits)])
r, m = doki.measure(r2, 0)
# del r2  # OJO! Si mides nada obtienes el mismo registro, no una copia.
print("Mes:", m)
print("R2:", [doki.get(r, i) for i in range(2**(num_qubits))])
del r
r = doki.new(num_qubits)
r2 = doki.apply(r, h, {0}, None, set())
del r
print("R1:", [doki.get(r2, i) for i in range(2**num_qubits)])
r, m = doki.measure(r2, 1)
del r2
print("Mes:", m)
print("R2:", [doki.get(r, i) for i in range(2**(num_qubits-1))])
del r
r = doki.new(num_qubits)
r2 = doki.apply(r, h, {0}, None, set())
del r
print("R1:", [doki.get(r2, i) for i in range(2**num_qubits)])
r, m = doki.measure(r2, 2)
del r2
print("Mes:", m)
print("R2:", [doki.get(r, i) for i in range(2**(num_qubits-1))])
del r
r = doki.new(num_qubits)
r2 = doki.apply(r, h, {0}, None, set())
del r
print("R1:", [doki.get(r2, i) for i in range(2**num_qubits)])
r, m = doki.measure(r2, 3)
del r2
print("Mes:", m)
# print([doki.get(r, i) for i in range(2**(num_qubits-1))])
del r
