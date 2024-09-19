"""Registry initialization tests."""
import argparse
import doki as doki
import numpy as np
import time as t

from timed_test import error, init_args


def gen_reg(nq, with_data=False):
    """Generate registry of nq qubits initialized to 0."""
    r = None
    size = 1 << nq  # 2**nq
    if with_data:
        r = np.random.rand(size)
        r = r / np.linalg.norm(r)
        r.shape = (size, 1)
    else:
        r = np.zeros((size, 1), dtype=complex)
        r[0, 0] = 1
    return r


def doki_to_np(r_doki, num_qubits, verbose, canonical=False):
    """Return numpy array with r_doki's column vector."""
    return np.transpose(np.array([doki.registry_get(r_doki, i, canonical,
                                                    verbose)
                                  for i in range(2**num_qubits)], ndmin=2))


def check_generation(num_qubits, verbose, with_data=False, with_lists=False):
    """Check if doki's new and get work for the specified number of qubits."""
    r_numpy = gen_reg(num_qubits, with_data=with_data)
    r_doki = None
    if with_data:
        aux = r_numpy.reshape(r_numpy.shape[0])
        if with_lists:
            r_doki = doki.registry_new_data(num_qubits, list(aux), verbose)
        else:
            r_doki = doki.registry_new_data(num_qubits, aux, verbose)
    else:
        r_doki = doki.registry_new(num_qubits, verbose)
    if not all(doki_to_np(r_doki, num_qubits, verbose) == r_numpy):
        error("Error comparing results of two qubit gate", fatal=True)


def check_range(min_qubits, max_qubits, verbose, with_data=False, with_lists=False):
    """Call check_generation for the specified range of qubits."""
    for nq in range(min_qubits, max_qubits + 1):
        check_generation(nq, verbose, with_data=with_data, with_lists=with_lists)


def main(min_qubits, max_qubits, verbose):
    """Execute all tests."""
    print("\tEmpty initialization tests...")
    a = t.time()
    res = check_range(min_qubits, max_qubits, verbose)
    b = t.time()
    print("\tRegistry list initialization tests...")
    c = t.time()
    res = check_range(min_qubits, max_qubits, verbose, with_data=True, with_lists=True)
    d = t.time()
    print("\tRegistry numpy initialization tests...")
    e = t.time()
    res = check_range(min_qubits, max_qubits, verbose, with_data=True)
    f = t.time()
    print(f"\tPEACE AND TRANQUILITY: {(b - a) + (d - c) + (f - e)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="RegCreationTests",
                                     description="Tests the creation of registries")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, help="whether to print extra information or not")
    parser.add_argument("-n", "--num_qubits", type=int, required=True, help="the starting number of qubits to use")
    parser.add_argument("-m", "--max_qubits", type=int, default=None, help="the max number of qubits to use")
    # parser.add_argument("-t", "--num_threads", type=int, default=None, help="the number of threads to use")
    # parser.add_argument("-i", "--iterations", type=int, default=None, help="how many times the test target has to be executed")
    # parser.add_argument("-s", "--seed", type=int, default=None, help="sets the seed to use")
    args = parser.parse_args()

    print("Registry creation tests:")
    prng = init_args(args)
    main(args.num_qubits, args.max_qubits, args.verbose)
