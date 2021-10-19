"""Probability tests."""
import doki as doki
import numpy as np
import os
import sys

from reg_creation_tests import doki_to_np


def Ry_doki(angle):
    """Return a Ry gate."""
    npgate = np.array([[np.cos(angle / 2), -np.sin(angle / 2)],
                       [np.sin(angle / 2),  np.cos(angle / 2)]], dtype=complex)
    return doki.gate_new(1, npgate.tolist(), False)


def test_probability(nq, rtol, atol, num_threads):
    """Test probability method with nq qubits registry."""
    step = 2 * np.pi / nq
    gates = [Ry_doki(step * i) for i in range(nq)]
    reg = doki.registry_new(nq, False)
    for i in range(nq):
        aux = doki.registry_apply(reg, gates[i], [i], None, None,
                                  num_threads, False)
        doki.registry_del(reg, False)
        reg = aux
    for i in range(nq):
        np_old = doki_to_np(reg, nq)
        odds = doki.registry_prob(reg, i, num_threads, False)
        exodds = np.sin((step * i) / 2)**2
        if not np.allclose(odds, exodds, rtol=rtol, atol=atol):
            print(f"Obtained Odds: P(M({i})=1) = {odds}")
            print(f"Expected Odds: P(M({i})=1) = {exodds}")
            np_reg = doki_to_np(reg, nq)
            print("r_doki:", np_reg)
            print("equals old:", np.all(np_reg == np_old))
            print("sum:", np.sum([np.abs(j)**2 for j in np_reg]))
            raise AssertionError("Failed probability check")
    del reg


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    num_threads = None
    if 2 <= len(argv) <= 3:
        min_qubits = int(argv[0])
        max_qubits = int(argv[1])
        if len(argv) >= 3:
            num_threads = int(argv[2])
        if (min_qubits < 1):
            raise ValueError("minimum number of qubits must be at least 1")
        elif (min_qubits > max_qubits):
            raise ValueError("minimum can't be greater than maximum")
        if num_threads is not None and num_threads < -1:
            raise ValueError("num_threads must be at least 1 " +
                             "(0 -> ENV VAR, -1 -> OMP default)")
        elif num_threads == 0:
            num_threads = None
        print("Probability method tests...")
        if num_threads is None:
            num_threads = os.getenv('OMP_NUM_THREADS')
            if num_threads is None:
                num_threads = -1
            elif num_threads <= 0:
                raise ValueError("Error: OMP_NUM_THREADS can't be less than 1")
            print("\tNumber of threads:", num_threads)
        rtol = 0
        atol = 1e-13
        for nq in range(min_qubits, max_qubits + 1):
            test_probability(nq, rtol, atol, num_threads)
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits>" +
                         " <number of threads <optional>")


if __name__ == "__main__":
    main()
