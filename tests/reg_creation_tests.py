"""Registry initialization tests."""
import doki as doki
import numpy as np
import sys


def gen_reg(nq):
    """Generate registry of nq qubits initialized to 0."""
    r = np.zeros((2**nq, 1), dtype=complex)
    r[0, 0] = 1
    return r


def doki_to_np(r_doki, num_qubits, canonical=False):
    """Return numpy array with r_doki's column vector."""
    return np.transpose(np.array([doki.registry_get(r_doki, i, canonical,
                                                    False)
                                  for i in range(2**num_qubits)], ndmin=2))


def check_generation(num_qubits):
    """Check if doki's new and get work for the specified number of qubits."""
    r_doki = doki.registry_new(num_qubits, False)
    r_numpy = gen_reg(num_qubits)
    return all(doki_to_np(r_doki, num_qubits) == r_numpy)


def check_range(min_qubits, max_qubits):
    """Call check_generation for the specified range of qubits."""
    return [check_generation(nq) for nq in range(min_qubits, max_qubits + 1)]


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
        print("Registry initialization tests...")
        res = check_range(min_qubits, max_qubits)
        if not all(res):
            fails = [i + min_qubits for i in range(len(res)) if not res[i]]
            raise AssertionError("Failed tests: " + str(fails))
        else:
            print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits>")


if __name__ == "__main__":
    main()
