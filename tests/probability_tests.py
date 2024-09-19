"""Probability tests."""
import argparse
import doki as doki
import numpy as np
import os
import time as t

from reg_creation_tests import doki_to_np
from timed_test import debug, error, init_args


def Ry_doki(angle, verbose):
    """Return a Ry gate."""
    npgate = np.array([[np.cos(angle / 2), -np.sin(angle / 2)],
                       [np.sin(angle / 2),  np.cos(angle / 2)]], dtype=complex)
    return doki.gate_new(1, npgate.tolist(), verbose)


def test_probability(nq, rtol, atol, num_threads, verbose):
    """Test probability method with nq qubits registry."""
    step = 2 * np.pi / nq
    gates = [Ry_doki(step * i, verbose) for i in range(nq)]
    reg = doki.registry_new(nq, False)
    for i in range(nq):
        aux = doki.registry_apply(reg, gates[i], [i], None, None,
                                  num_threads, verbose)
        doki.registry_del(reg, False)
        reg = aux
    for i in range(nq):
        np_old = doki_to_np(reg, nq, verbose)
        odds = doki.registry_prob(reg, i, num_threads, verbose)
        exodds = np.sin((step * i) / 2)**2
        if not np.allclose(odds, exodds, rtol=rtol, atol=atol):
            debug(f"Obtained Odds: P(M({i})=1) = {odds}")
            debug(f"Expected Odds: P(M({i})=1) = {exodds}")
            np_reg = doki_to_np(reg, nq, verbose)
            debug("r_doki:", np_reg)
            debug("equals old:", np.all(np_reg == np_old))
            debug("sum:", np.sum([np.abs(j)**2 for j in np_reg]))
            error("Failed probability check", fatal=True)
    del reg


def main(min_qubits, max_qubits, num_threads, verbose):
    """Execute all tests."""
    rtol = 0
    atol = 1e-13
    a = t.time()
    for nq in range(min_qubits, max_qubits + 1):
        test_probability(nq, rtol, atol, num_threads, verbose)
    b = t.time()
    print(f"\tPEACE AND TRANQUILITY: {b - a}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="ProbabilityTest",
                                     description="Checks if the probabilities outputed by doki function are the expected ones")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, help="whether to print extra information or not")
    parser.add_argument("-n", "--num_qubits", type=int, required=True, help="the starting number of qubits to use")
    parser.add_argument("-m", "--max_qubits", type=int, default=None, help="the max number of qubits to use")
    parser.add_argument("-t", "--num_threads", type=int, default=None, help="the number of threads to use")
    # parser.add_argument("-i", "--iterations", type=int, required=True, help="how many times the test target has to be executed")
    # parser.add_argument("-s", "--seed", type=int, default=None, help="sets the seed to use")
    args = parser.parse_args()

    print("Probability method tests:")
    prng = init_args(args)
    main(args.num_qubits, args.max_qubits, args.num_threads, args.verbose)
