"""Join registry tests."""
import doki as doki
import numpy as np
import os
import sys
import time as t

from reg_creation_tests import doki_to_np
from one_gate_tests import U_doki


def test_random_join(nq, rtol, atol, num_threads):
    """Test joining nq registries of one qubit."""
    gates = [U_doki(*(np.pi * (np.random.random_sample(3) * 2 - 1)),
                    np.random.choice(a=[False, True]))
             for i in range(nq)]
    regs = [doki.registry_new(1, False) for i in range(nq)]
    r2s = [doki.registry_apply(regs[i], gates[i], [0], None, None,
                               num_threads, False)
           for i in range(nq)]
    exreg = doki.registry_new(nq, False)
    for i in range(nq):
        del regs[nq - i - 1]
        aux = doki.registry_apply(exreg, gates[i], [nq - i - 1], None, None,
                                  num_threads, False)
        del exreg
        exreg = aux
    res = r2s[0]
    first = True
    for reg in r2s[1:]:
        aux = doki.registry_join(res, reg, num_threads, False)
        if not first:
            del res
        else:
            first = False
        res = aux
    if not np.allclose(doki_to_np(res, nq), doki_to_np(exreg, nq),
                       rtol=rtol, atol=atol):
        raise AssertionError("Failed right join comparison")
    if not first:
        del res
    res = r2s[-1]
    first = True
    for reg in r2s[nq-2::-1]:
        aux = doki.registry_join(reg, res, num_threads, False)
        if not first:
            del res
        else:
            first = False
        res = aux
    if not np.allclose(doki_to_np(res, nq), doki_to_np(exreg, nq),
                       rtol=rtol, atol=atol):
        raise AssertionError("Failed left join comparison")
    for i in range(nq):
        del r2s[nq - i - 1]
    if not first:
        del res
    del exreg


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    seed = None
    num_threads = None
    if 1 <= len(argv) <= 3:
        num_qubits = int(argv[0])
        if len(argv) >= 2:
            num_threads = int(argv[1])
        if len(argv) >= 3:
            seed = int(argv[2])
        if (num_qubits < 2):
            raise ValueError("number of qubits must be at least 2")
        if seed is not None and (seed < 0 or seed >= 2**32):
            raise ValueError("seed must be in [0, 2^32 - 1]")
        if num_threads is not None and num_threads < -1:
            raise ValueError("num_threads must be at least 1 " +
                             "(0 -> ENV VAR, -1 -> OMP default)")
        elif num_threads == 0:
            num_threads = None
        print("Registry tensor product tests...")
        if seed is None:
            seed = np.random.randint(np.iinfo(np.int32).max)
            print("\tSeed:", seed)
        np.random.seed(seed)
        if num_threads is None:
            num_threads = os.getenv('OMP_NUM_THREADS')
            if num_threads is None:
                num_threads = -1
            elif num_threads <= 0:
                raise ValueError("Error: OMP_NUM_THREADS can't be less than 1")
            print("\tNumber of threads:", num_threads)
        rtol = 0
        atol = 1e-13
        a = t.time()
        test_random_join(num_qubits, rtol, atol, num_threads)
        b = t.time()
        print(f"\tPEACE AND TRANQUILITY: {b - a} s")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <number of qubits (min 2)>" +
                         " <number of threads (optional)>" +
                         " <seed (optional)>")


if __name__ == "__main__":
    main()
