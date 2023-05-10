"""Registry initialization tests."""
import doki as doki
import numpy as np
import sys


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


def doki_to_np(r_doki, num_qubits, canonical=False):
    """Return numpy array with r_doki's column vector."""
    return np.transpose(np.array([doki.registry_get(r_doki, i, canonical,
                                                    False)
                                  for i in range(2**num_qubits)], ndmin=2))


def check_generation(num_qubits, with_data=False, with_lists=False):
    """Check if doki's new and get work for the specified number of qubits."""
    r_numpy = gen_reg(num_qubits, with_data=with_data)
    r_doki = None
    if with_data:
        aux = r_numpy.reshape(r_numpy.shape[0])
        if with_lists:
            r_doki = doki.registry_new_data(num_qubits, list(aux), False)
        else:
            r_doki = doki.registry_new_data(num_qubits, aux, False)
    else:
        r_doki = doki.registry_new(num_qubits, False)
    if not all(doki_to_np(r_doki, num_qubits) == r_numpy):
        raise AssertionError("Error comparing results of two" +
                                     " qubit gate")


def check_range(min_qubits, max_qubits, with_data=False, with_lists=False):
    """Call check_generation for the specified range of qubits."""
    return [check_generation(nq, with_data=with_data, with_lists=with_lists)
            for nq in range(min_qubits, max_qubits + 1)]


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    if 2 == len(argv):
        min_qubits = int(argv[0])
        max_qubits = int(argv[1])
        seed = None
        if len(argv) >= 3:
            seed = int(argv[2])
        if (min_qubits < 1):
            raise ValueError("minimum number of qubits must be at least 1")
        elif (min_qubits > max_qubits):
            raise ValueError("minimum can't be greater than maximum")
        if seed is not None and (seed < 0 or seed >= 2**32):
            raise ValueError("seed must be in [0, 2^32 - 1]")
        print("Registry creation tests...")
        if seed is None:
            seed = np.random.randint(np.iinfo(np.int32).max)
            print("\tSeed:", seed)
        np.random.seed(seed)
        print("\tEmpty initialization tests...")
        res = check_range(min_qubits, max_qubits)
        print("\tRegistry list initialization tests...")
        res = check_range(min_qubits, max_qubits, with_data=True, with_lists=True)
        print("\tRegistry numpy initialization tests...")
        res = check_range(min_qubits, max_qubits, with_data=True)
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits>")


if __name__ == "__main__":
    main()
