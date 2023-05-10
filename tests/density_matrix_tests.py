"""Registry initialization tests."""
import doki
import numpy as np
import sys


def check_density_matrix(num_qubits):
    size = 2**num_qubits
    state = doki.registry_new(num_qubits, False)
    ket = np.array([[doki.registry_get(state, i, False, False)]
                    for i in range(size)])
    bra = ket.conj().T
    expected_rho = np.dot(ket, bra)
    rho = doki.registry_density(state, False)
    np_rho = np.array([[doki.funmatrix_get(rho, i, j, False)
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


def check_partial_trace(num_qubits):
    if num_qubits < 2:
        return
    rsize = 2**(num_qubits-1)
    state = doki.registry_new(num_qubits, False)
    rho = doki.registry_density(state, False)
    rrho = doki.funmatrix_partialtrace(rho, 0, False)
    np_rrho = np.array([[doki.funmatrix_get(rrho, i, j, False)
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


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    if 2 == len(argv):
        min_qubits = int(argv[0])
        max_qubits = int(argv[1])
        if (min_qubits < 1):
            raise ValueError("minimum number of qubits must be at least 1")
        elif (min_qubits > max_qubits):
            raise ValueError("minimum can't be greater than maximum")
        print("Density matrix tests...")
        for nq in range(min_qubits, max_qubits + 1):
            check_density_matrix(nq)
        if max_qubits > 1:
            print("\tPartial trace tests...")
            for nq in range(max(2, min_qubits), max_qubits + 1):
                check_partial_trace(nq)
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits>")


if __name__ == "__main__":
    main()
