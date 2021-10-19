"""One qubit gate tests."""
import doki_gpu as doki
import numpy as np
import os
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
    return doki.gate_new(1, U_np(angle1, angle2, angle3, invert).tolist(),
                         False)


def apply_np(nq, r, g, target):
    """Apply gate g to numpy column vector r."""
    if g is not None:
        nqg = int(np.log2(g.shape[0]))
        if nq > 1:
            left = nq - target - nqg
            right = target
            if (left > 0):
                g = sparse.kron(Identity(left), g)
            if (right > 0):
                g = sparse.kron(g, Identity(right))
        return g.dot(r)
    else:
        return r[:, :]


def apply_gate(nq, r_np, r_doki, g_sparse, g_doki, target, num_threads):
    """Apply gate to registry (both numpy+sparse and doki)."""
    # print(doki_to_np(r_doki, nq))
    # print(g_doki)
    # print({target})
    new_r_doki = doki.registry_apply(r_doki, g_doki, [target], None, None,
                                     num_threads, False)
    return (apply_np(nq, r_np, g_sparse, target), new_r_doki)


def test_gates_static(num_qubits, num_threads):
    """Apply a random 1-qubit gate to each qubit and compare results."""
    rtol = 0
    atol = 1e-13
    r2_np = gen_reg(num_qubits)
    r2_doki = doki.registry_new(num_qubits, False)
    for i in range(num_qubits):
        r1_np = r2_np
        r1_doki = r2_doki
        angles = np.pi * (np.random.random_sample(3) * 2 - 1)
        invert = np.random.choice(a=[False, True])
        r2_np, r2_doki = apply_gate(num_qubits, r1_np, r1_doki,
                                    U_sparse(*angles, invert),
                                    U_doki(*angles, invert), i,
                                    num_threads)
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
            raise AssertionError("Error applying gate")
        del r1_np
        del r1_doki


def one_gate_range(min_qubits, max_qubits, num_threads):
    """Execute test_gates_static once for each posible number in range."""
    for nq in range(min_qubits, max_qubits + 1):
        test_gates_static(nq, num_threads)


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    seed = None
    num_threads = None
    if 2 <= len(argv) <= 3:
        min_qubits = int(argv[0])
        max_qubits = int(argv[1])
        if len(argv) >= 3:
            num_threads = int(argv[2])
        if len(argv) >= 4:
            seed = int(argv[3])
        if (min_qubits < 1):
            raise ValueError("minimum number of qubits must be at least 1")
        elif (min_qubits > max_qubits):
            raise ValueError("minimum can't be greater than maximum")
        if seed is not None and (seed < 0 or seed >= 2**32):
            raise ValueError("seed must be in [0, 2^32 - 1]")
        if num_threads is not None and num_threads < -1:
            raise ValueError("num_threads must be at least 1 " +
                             "(0 -> ENV VAR, -1 -> OMP default)")
        elif num_threads == 0:
            num_threads = None
        print("One qubit gate application tests...")
        if seed is None:
            seed = np.random.randint(np.iinfo(np.int32).max)
            print("\tSeed:", seed)
        np.random.seed(seed)
        if num_threads is None:
            num_threads = os.getenv('OMP_NUM_THREADS')
            if num_threads is None:
                num_threads = -1
            elif num_threads <= 0:
                raise ValueError("Error: OMP_NUM_THREADS can't be less than 1")
            print("\tNumber of threads:", num_threads)
        one_gate_range(min_qubits, max_qubits, num_threads)
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits>" +
                         " <number of threads (optional)>" +
                         " <seed (optional)>")


if __name__ == "__main__":
    main()
