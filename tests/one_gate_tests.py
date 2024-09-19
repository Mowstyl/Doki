"""One qubit gate tests."""
import argparse
import doki as doki
import numpy as np
import scipy.sparse as sparse
import time as t

from reg_creation_tests import gen_reg, doki_to_np
from timed_test import debug, error, init_args


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


def U_doki(angle1, angle2, angle3, invert, verbose):
    """Return doki U gate (IBM)."""
    return doki.gate_new(1, U_np(angle1, angle2, angle3, invert).tolist(),
                         verbose)


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


def apply_gate(nq, r_np, r_doki, g_sparse, g_doki, target, num_threads, verbose):
    """Apply gate to registry (both numpy+sparse and doki)."""
    # print(doki_to_np(r_doki, nq, verbose))
    # print(g_doki)
    # print({target})
    new_r_doki = doki.registry_apply(r_doki, g_doki, [target], None, None,
                                     num_threads, verbose)
    return (apply_np(nq, r_np, g_sparse, target), new_r_doki)


def test_gates_static(num_qubits, num_threads, prng, verbose):
    """Apply a random 1-qubit gate to each qubit and compare results."""
    rtol = 0
    atol = 1e-13
    r2_np = gen_reg(num_qubits)
    r2_doki = doki.registry_new(num_qubits, False)
    for i in range(num_qubits):
        r1_np = r2_np
        r1_doki = r2_doki
        angles = np.pi * (prng.random(3) * 2 - 1)
        invert = prng.choice(a=[False, True])
        r2_np, r2_doki = apply_gate(num_qubits, r1_np, r1_doki,
                                    U_sparse(*angles, invert),
                                    U_doki(*angles, invert, verbose), i,
                                    num_threads, verbose)
        if not np.allclose(doki_to_np(r2_doki, num_qubits, verbose), r2_np,
                           rtol=rtol, atol=atol):
            debug("i:", i)
            debug("angles:", angles)
            debug("invert:", invert)
            debug("r1_np:", r1_np)
            debug("r1_doki:", doki_to_np(r1_doki, num_qubits, verbose))
            debug("r2_np:", r2_np)
            debug("r2_doki:", doki_to_np(r2_doki, num_qubits, verbose))
            debug("comp:", np.allclose(doki_to_np(r2_doki, num_qubits, verbose),
                                       r2_np, rtol=rtol, atol=atol))
            error("Error applying gate", fatal=True)
        del r1_np
        del r1_doki


def main(min_qubits, max_qubits, num_threads, prng, verbose):
    """Execute test_gates_static once for each posible number in range."""
    a = t.time()
    for nq in range(min_qubits, max_qubits + 1):
        test_gates_static(nq, num_threads, prng, verbose)
    b = t.time()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="OneGateTests",
                                     description="Checks if apply_gate method works with one qubit gates")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, help="whether to print extra information or not")
    parser.add_argument("-n", "--num_qubits", type=int, required=True, help="the starting number of qubits to use")
    parser.add_argument("-m", "--max_qubits", type=int, default=None, help="the max number of qubits to use")
    parser.add_argument("-t", "--num_threads", type=int, default=None, help="the number of threads to use")
    # parser.add_argument("-i", "--iterations", type=int, required=True, help="how many times the test target has to be executed")
    parser.add_argument("-s", "--seed", type=int, default=None, help="sets the seed to use")
    args = parser.parse_args()

    print("One qubit gate application tests:")
    prng = init_args(args)
    main(args.num_qubits, args.max_qubits, args.num_threads, prng, args.verbose)
