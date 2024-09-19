"""Get canonical state tests."""
import argparse
import doki as doki
import numpy as np
import time as t

from reg_creation_tests import doki_to_np
from timed_test import debug, error, init_args


def phase_doki(angle, verbose):
    """Return a gate with no observable changes (hidden phase)."""
    npgate = np.exp(1j * angle) * np.eye(2)
    return doki.gate_new(1, npgate.tolist(), verbose)


def test_canonical_apply(nq, rtol, atol, num_threads, prng, verbose):
    """Test canonical get with nq qubit registries after gate apply."""
    # print("\tTesting get after apply")
    gates = [phase_doki(np.pi * (prng.random() * 2 - 1), verbose)
             for i in range(nq)]
    reg = doki.registry_new(nq, verbose)
    npreg = doki_to_np(reg, nq, verbose, canonical=False)
    npreg = np.exp(-1j*np.angle(npreg[0, 0])) * npreg
    if not np.allclose(doki_to_np(reg, nq, verbose, canonical=True),
                       npreg,
                       rtol=rtol, atol=atol):
        raise AssertionError("Failed canonical get on clean state")
    for i in range(nq):
        aux = doki.registry_apply(reg, gates[i], [i], None, None,
                                  num_threads, verbose)
        del reg
        reg = aux
        rawnpreg = doki_to_np(reg, nq, verbose, canonical=False)
        npreg = np.exp(-1j*np.angle(rawnpreg[0, 0])) * rawnpreg
        if not np.allclose(doki_to_np(reg, nq, verbose, canonical=True),
                           npreg,
                           rtol=rtol, atol=atol):
            raise AssertionError("Failed canonical get after operating")
    # print("\tTesting get after measure (apply)")
    for i in range(nq - 1):
        aux, _ = doki.registry_measure(reg, 1, [prng.random()],
                                       num_threads, verbose)
        del reg
        reg = aux
        npreg = doki_to_np(reg, nq - i - 1, verbose, canonical=False)
        npreg = np.exp(-1j*np.angle(npreg[0, 0])) * npreg
        if not np.allclose(doki_to_np(reg, nq - i - 1, verbose, canonical=True),
                           npreg,
                           rtol=rtol, atol=atol):
            error("Failed canonical get on measured state", fatal=True)
    del reg


def test_canonical_join_mes(nq, rtol, atol, num_threads, prng, verbose):
    """Test canonical get with nq qubit registries after join and measure."""
    # print("\tTesting get after join")
    gates = [phase_doki(np.pi * (prng.random() * 2 - 1), verbose)
             for i in range(nq)]
    rawregs = [doki.registry_new(1, verbose) for i in range(nq)]
    regs = [doki.registry_apply(rawregs[i], gates[i], [0], None, None,
                                num_threads, verbose)
            for i in range(nq)]
    joined = regs[0]
    for i in range(nq):
        del rawregs[nq - i - 1]
    for i in range(1, nq):
        aux = doki.registry_join(joined, regs[i], num_threads, verbose)
        if i > 1:
            del joined
        joined = aux
        npreg = doki_to_np(joined, i + 1, verbose, canonical=False)
        npreg = np.exp(-1j*np.angle(npreg[0, 0])) * npreg
        if not np.allclose(doki_to_np(joined, i + 1, verbose, canonical=True),
                           npreg,
                           rtol=rtol, atol=atol):
            error("Failed canonical get on joined state", fatal=True)
    for i in range(nq):
        del regs[nq - i - 1]
    # print("\tTesting get after measure (join)")
    for i in range(nq - 1):
        aux, _ = doki.registry_measure(joined, 1, [prng.random()],
                                       num_threads, verbose)
        del joined
        joined = aux
        npreg = doki_to_np(joined, nq - i - 1, verbose, canonical=False)
        npreg = np.exp(-1j*np.angle(npreg[0, 0])) * npreg
        if not np.allclose(doki_to_np(joined, nq - i - 1, verbose, canonical=True),
                           npreg,
                           rtol=rtol, atol=atol):
            error("Failed canonical get on measured state", fatal=True)
    del joined


def main(min_qubits, max_qubits, num_threads, prng, verbose):
    """Execute all tests."""
    rtol = 0
    atol = 1e-13
    a = t.time()
    for nq in range(min_qubits, max_qubits + 1):
        test_canonical_apply(nq, rtol, atol, num_threads, prng, verbose)
        if nq > 1:
            test_canonical_join_mes(nq, rtol, atol, num_threads, prng, verbose)
    b = t.time()
    print(f"\tPEACE AND TRANQUILITY: {(b - a)} s")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="DokiTime",
                                     description="Measures the time needed to execute a predefined quantum program with the specified number of qubits")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, help="whether to print extra information or not")
    parser.add_argument("-n", "--num_qubits", type=int, required=True, help="the starting number of qubits to use")
    parser.add_argument("-m", "--max_qubits", type=int, default=None, help="the max number of qubits to use")
    parser.add_argument("-t", "--num_threads", type=int, default=None, help="the number of threads to use")
    # parser.add_argument("-i", "--iterations", type=int, required=True, help="how many times the test target has to be executed")
    parser.add_argument("-s", "--seed", type=int, default=None, help="sets the seed to use")
    args = parser.parse_args()

    print("Get state without hidden phase tests:")
    prng = init_args(args)
    main(args.num_qubits, args.max_qubits, args.num_threads, prng, args.verbose)
