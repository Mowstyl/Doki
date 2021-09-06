"""Hadamard tests."""
import doki
import numpy as np
import scipy.sparse as sparse
import sys

from reg_creation_tests import gen_reg, doki_to_np


def Identity(nq):
    """Return sparse matrix with Identity gate."""
    return sparse.identity(2**nq)


def U_np(angle1, angle2, angle3, invert):
    """Return numpy array with U gate (IBM)."""
    gate = np.zeros(4, dtype=complex).reshape(2, 2)
    cosan = np.cos(angle1/2)
    sinan = np.sin(angle1/2)
    mult = 1
    if invert:
        mult = -1
    gate[0, 0] = cosan
    if not invert:
        gate[0, 1] = -sinan * np.cos(angle3) - sinan * np.sin(angle3) * 1j
        gate[1, 0] = sinan * np.cos(angle2) + sinan * np.sin(angle2) * 1j
    else:
        gate[0, 1] = sinan * np.cos(angle2) - sinan * np.sin(angle2) * 1j
        gate[1, 0] = -sinan * np.cos(angle3) + sinan * np.sin(angle3) * 1j
    gate[1, 1] = cosan * np.cos(angle2+angle3) \
        + mult * cosan * np.sin(angle2 + angle3) * 1j
    return gate


def U_sparse(angle1, angle2, angle3, invert):
    """Return scipy sparse CSR matrix with U gate (IBM)."""
    return sparse.csr_matrix(U_np(angle1, angle2, angle3, invert))


def U_doki(angle1, angle2, angle3, invert):
    """Return doki U gate (IBM)."""
    return doki.gate(1, U_np(angle1, angle2, angle3, invert).tolist())


def apply_gate(nq, r, r_doki, g_sparse, g_doki, target):
    """Apply gate to registry (both numpy+sparse and doki)."""
    new_r_doki = doki.apply(r_doki, g_doki, {target}, None, None)
    if nq > 1:
        left = nq - target - 1
        right = target
        if (left > 0):
            g_sparse = sparse.kron(Identity(left), g_sparse)
        if (right > 0):
            g_sparse = sparse.kron(g_sparse, Identity(right))
    new_r = g_sparse.dot(r)
    return (new_r, new_r_doki)


def test_gates_static(num_qubits):
    """Apply a random 1-qubit gate to each qubit and compare results."""
    rtol = 0
    atol = 1e-13
    r2_np = gen_reg(num_qubits)
    r2_doki = doki.new(num_qubits)
    fails = []
    for i in range(num_qubits):
        r1_np = r2_np
        r1_doki = r2_doki
        angles = np.pi * (np.random.random_sample(3) * 2 - 1)
        invert = np.random.choice(a=[False, True])
        r2_np, r2_doki = apply_gate(num_qubits, r1_np, r1_doki,
                                    U_sparse(*angles, invert),
                                    U_doki(*angles, invert), i)
        if not np.allclose(doki_to_np(r2_doki, num_qubits), r2_np,
                           rtol=rtol, atol=atol):
            '''
            print("i:", i)
            print("angles:", angles)
            print("invert:", invert)
            print("r1_np:", r1_np)
            print("r1_doki:", doki_to_np(r1_doki, num_qubits))
            print("r2_np:", r2_np)
            print("r2_doki:", doki_to_np(r2_doki, num_qubits))
            print("comp:", np.allclose(doki_to_np(r2_doki, num_qubits), r2_np,
                                       rtol=rtol, atol=atol))
            '''
            fails.append((angles, invert, i))
        del r1_np
        del r1_doki
    if len(fails) == 0:
        return None
    return fails


def one_gate_range(min_qubits, max_qubits):
    """Execute test_gates_static once for each posible number in range."""
    res = [(nq, test_gates_static(nq))
           for nq in range(min_qubits, max_qubits + 1)]
    return [elem for elem in res if elem[1]]  # List of failed tests


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    seed = None
    if len(argv) == 3:
        seed = int(argv[2])
    if 2 <= len(argv) <= 3:
        min_qubits = int(argv[0])
        max_qubits = int(argv[1])
        if (min_qubits < 1):
            raise ValueError("minimum number of qubits must be at least 1")
        elif (min_qubits > max_qubits):
            raise ValueError("minimum can't be greater than maximum")
        if seed is not None and (seed < 0 or seed >= 2**32):
            raise ValueError("seed must be in [0, 2^32 - 1]")
        print("One qubit gate application tests...")
        if seed is None:
            seed = np.random.randint(np.iinfo(np.int32).max)
            print("Seed:", seed)
        np.random.seed(seed)
        res = one_gate_range(min_qubits, max_qubits)
        if any(res):
            raise AssertionError("Failed tests: " + str(res))
        else:
            print("PEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits> <seed (optional)>")


if __name__ == "__main__":
    main()
