"""Join registry tests."""
import doki
import numpy as np

sqrt2_2 = np.sqrt(2) / 2

raw_h = [[sqrt2_2, sqrt2_2], [sqrt2_2, -sqrt2_2]]
h = doki.gate(1, raw_h)

raw_x = [[0, 1], [1, 0]]
x = doki.gate(1, raw_x)

r1 = doki.new(1)
print("R1:", [doki.get(r1, i) for i in range(2)])
r2 = doki.new(1)
print("R2:", [doki.get(r2, i) for i in range(2)])

doki.apply(r2, x, {0}, None, set())
print("R2:", [doki.get(r2, i) for i in range(2)])
r12 = doki.join(r1, r2)
print("R12:", [doki.get(r12, i) for i in range(4)])
del r12
r21 = doki.join(r2, r1)
print("R21:", [doki.get(r21, i) for i in range(4)])
del r21

doki.apply(r1, h, {0}, None, None)
print("R1:", [doki.get(r1, i) for i in range(2)])
doki.apply(r2, h, {0}, set(), set())
print("R2:", [doki.get(r2, i) for i in range(2)])
r12 = doki.join(r1, r2)
print("R12:", [doki.get(r12, i) for i in range(4)])
del r12
r21 = doki.join(r2, r1)
print("R21:", [doki.get(r21, i) for i in range(4)])
del r21
del r1
del r2
