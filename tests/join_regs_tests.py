"""Join registry tests."""
import argparse
import doki as doki
import numpy as np
import os
import sys
import time as t

from reg_creation_tests import doki_to_np
from one_gate_tests import U_doki
from timed_test import error, init_args


def test_random_join(nq, rtol, atol, num_threads, prng, verbose):
    """Test joining nq registries of one qubit."""
    gates = [U_doki(*(np.pi * (prng.random(3) * 2 - 1)),
                    prng.choice(a=[False, True]), verbose)
             for i in range(nq)]
    regs = [doki.registry_new(1, verbose) for i in range(nq)]
    r2s = [doki.registry_apply(regs[i], gates[i], [0], None, None,
                               num_threads, verbose)
           for i in range(nq)]
    exreg = doki.registry_new(nq, verbose)
    for i in range(nq):
        del regs[nq - i - 1]
        aux = doki.registry_apply(exreg, gates[i], [nq - i - 1], None, None,
                                  num_threads, verbose)
        del exreg
        exreg = aux
    res = r2s[0]
    first = True
    for reg in r2s[1:]:
        aux = doki.registry_join(res, reg, num_threads, verbose)
        if not first:
            del res
        else:
            first = False
        res = aux
    if not np.allclose(doki_to_np(res, nq, verbose),
                       doki_to_np(exreg, nq, verbose),
                       rtol=rtol, atol=atol):
        raise AssertionError("Failed right join comparison")
    if not first:
        del res
    res = r2s[-1]
    first = True
    for reg in r2s[nq-2::-1]:
        aux = doki.registry_join(reg, res, num_threads, verbose)
        if not first:
            del res
        else:
            first = False
        res = aux
    if not np.allclose(doki_to_np(res, nq, verbose),
                       doki_to_np(exreg, nq, verbose),
                       rtol=rtol, atol=atol):
        error("Failed left join comparison", fatal=True)
    for i in range(nq):
        del r2s[nq - i - 1]
    if not first:
        del res
    del exreg


def main(min_qubits, max_qubits, num_threads, prng, verbose):
    """Execute all tests."""
    rtol = 0
    atol = 1e-13
    a = t.time()
    for num_qubits in range(min_qubits, max_qubits):
        test_random_join(num_qubits, rtol, atol, num_threads, prng, verbose)
    b = t.time()
    print(f"\tPEACE AND TRANQUILITY: {b - a} s")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="MeasureTests",
                                     description="Checks if measure method works with the expected odds")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, help="whether to print extra information or not")
    parser.add_argument("-n", "--num_qubits", type=int, required=True, help="the starting number of qubits to use")
    parser.add_argument("-m", "--max_qubits", type=int, default=None, help="the max number of qubits to use")
    parser.add_argument("-t", "--num_threads", type=int, default=None, help="the number of threads to use")
    # parser.add_argument("-i", "--iterations", type=int, required=True, help="how many times the test target has to be executed")
    parser.add_argument("-s", "--seed", type=int, default=None, help="sets the seed to use")
    args = parser.parse_args()

    print("Registry tensor product tests:")
    prng = init_args(args)
    main(args.num_qubits, args.max_qubits, args.num_threads, prng, args.verbose)
