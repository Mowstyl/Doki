"""Registry initialization tests."""
import argparse
import doki
import numpy as np
import time as t

from timed_test import debug, error, init_args


def check_density_matrix(num_qubits, verbose):
    size = 2**num_qubits
    state = doki.registry_new(num_qubits, verbose)
    ket = np.array([[doki.registry_get(state, i, False, verbose)]
                    for i in range(size)])
    bra = ket.conj().T
    expected_rho = np.dot(ket, bra)
    rho = doki.registry_density(state, False)
    np_rho = np.array([[doki.funmatrix_get(rho, i, j, verbose)
                        for j in range(size)]
                       for i in range(size)])
    if not np.allclose(expected_rho, np_rho):
        print("Actual density matrix differs from expected")
        print("Expected:")
        print(expected_rho)
        print("Actual:")
        print(np_rho)
        print("Checks:")
        print(expected_rho == np_rho)
        raise AssertionError("Failed density matrix test " +
                             f"with {num_qubits} qubits")


def check_partial_trace(num_qubits, verbose):
    if num_qubits < 2:
        return
    rsize = 2**(num_qubits-1)
    state = doki.registry_new(num_qubits, verbose)
    rho = doki.registry_density(state, verbose)
    rrho = doki.funmatrix_partialtrace(rho, 0, verbose)
    np_rrho = np.array([[doki.funmatrix_get(rrho, i, j, verbose)
                        for j in range(rsize)]
                       for i in range(rsize)])
    expected_rrho = np.zeros((rsize, rsize), dtype=complex)
    expected_rrho[0, 0] = 1
    if not np.allclose(expected_rrho, np_rrho):
        print("Actual reduced density matrix differs from expected")
        print("Expected:")
        print(expected_rrho)
        print("Actual:")
        print(np_rrho)
        print("Checks:")
        print(expected_rrho == np_rrho)
        raise AssertionError("Failed reduced density matrix test " +
                             f"with {num_qubits} qubits")


def main(min_qubits, max_qubits, verbose):
    """Execute all tests."""
    a = t.time()
    for nq in range(min_qubits, max_qubits + 1):
        check_density_matrix(nq, verbose)
    b = t.time()
    c, d = 0, 0
    if max_qubits > 1:
        print("\tPartial trace tests...")
        c = t.time()
        for nq in range(max(2, min_qubits), max_qubits + 1):
            check_partial_trace(nq, verbose)
        d = t.time()
    print(f"\tPEACE AND TRANQUILITY: {(b - a) + (d - c)} s")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="DokiTime",
                                     description="Measures the time needed to execute a predefined quantum program with the specified number of qubits")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, help="whether to print extra information or not")
    parser.add_argument("-n", "--num_qubits", type=int, required=True, help="the starting number of qubits to use")
    parser.add_argument("-m", "--max_qubits", type=int, default=None, help="the max number of qubits to use")
    # parser.add_argument("-t", "--num_threads", type=int, default=None, help="the number of threads to use")
    # parser.add_argument("-i", "--iterations", type=int, required=True, help="how many times the test target has to be executed")
    # parser.add_argument("-s", "--seed", type=int, default=None, help="sets the seed to use")
    args = parser.parse_args()

    print("Density matrix tests:")
    init_args(args)
    main(args.num_qubits, args.max_qubits, args.verbose)
