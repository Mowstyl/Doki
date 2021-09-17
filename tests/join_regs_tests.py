"""Join registry tests."""
import doki
import numpy as np
import sys

from reg_creation_tests import doki_to_np
from one_gate_tests import U_doki


def test_random_join(nq, rtol, atol):
    """Test joining nq registries of one qubit."""
    gates = [U_doki(*(np.pi * (np.random.random_sample(3) * 2 - 1)),
                    np.random.choice(a=[False, True]))
             for i in range(nq)]
    regs = [doki.new(1, False) for i in range(nq)]
    r2s = [doki.apply(regs[i], gates[i], [0], None, None, False)
           for i in range(nq)]
    exreg = doki.new(nq, False)
    for i in range(nq):
        del regs[nq - i - 1]
        aux = doki.apply(exreg, gates[i], [nq - i - 1], None, None, False)
        del exreg
        exreg = aux
    res = r2s[0]
    first = True
    for reg in r2s[1:]:
        aux = doki.join(res, reg, False)
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
        aux = doki.join(reg, res, False)
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
    if len(argv) == 2:
        seed = int(argv[1])
    if 1 <= len(argv) <= 2:
        num_qubits = int(argv[0])
        if (num_qubits < 2):
            raise ValueError("number of qubits must be at least 2")
        if seed is not None and (seed < 0 or seed >= 2**32):
            raise ValueError("seed must be in [0, 2^32 - 1]")
        print("Multiple qubit gate application tests...")
        if seed is None:
            seed = np.random.randint(np.iinfo(np.int32).max)
            print("\tSeed:", seed)
        np.random.seed(seed)
        rtol = 0
        atol = 1e-13
        test_random_join(num_qubits, rtol, atol)
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <number of qubits (min 2)> <seed (optional)>")


if __name__ == "__main__":
    main()
