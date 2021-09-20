"""Get canonical state tests."""
import doki
import numpy as np
import sys

from reg_creation_tests import doki_to_np


def phase_doki(angle):
    """Return a gate with no observable changes (hidden phase)."""
    npgate = np.exp(1j * angle) * np.eye(2)
    return doki.gate_new(1, npgate.tolist(), False)


def test_canonical_apply(nq, rtol, atol):
    """Test canonical get with nq qubit registries after gate apply."""
    print("\tTesting get after apply")
    gates = [phase_doki(np.pi * (np.random.rand() * 2 - 1)) for i in range(nq)]
    reg = doki.registry_new(nq, False)
    npreg = doki_to_np(reg, nq, canonical=False)
    npreg = np.exp(-1j*np.angle(npreg[0, 0])) * npreg
    if not np.allclose(doki_to_np(reg, nq, canonical=True),
                       npreg,
                       rtol=rtol, atol=atol):
        raise AssertionError("Failed canonical get on clean state")
    for i in range(nq):
        aux = doki.registry_apply(reg, gates[i], [i], None, None, False)
        del reg
        reg = aux
        rawnpreg = doki_to_np(reg, nq, canonical=False)
        npreg = np.exp(-1j*np.angle(rawnpreg[0, 0])) * rawnpreg
        if not np.allclose(doki_to_np(reg, nq, canonical=True),
                           npreg,
                           rtol=rtol, atol=atol):
            raise AssertionError("Failed canonical get after operating")
    print("\tTesting get after measure (apply)")
    for i in range(nq - 1):
        aux, _ = doki.registry_measure(reg, 1, [np.random.rand()], False)
        del reg
        reg = aux
        npreg = doki_to_np(reg, nq - i - 1, canonical=False)
        npreg = np.exp(-1j*np.angle(npreg[0, 0])) * npreg
        if not np.allclose(doki_to_np(reg, nq - i - 1, canonical=True),
                           npreg,
                           rtol=rtol, atol=atol):
            raise AssertionError("Failed canonical get on measured state")
    del reg


def test_canonical_join_mes(nq, rtol, atol):
    """Test canonical get with nq qubit registries after join and measure."""
    print("\tTesting get after join")
    gates = [phase_doki(np.pi * (np.random.rand() * 2 - 1)) for i in range(nq)]
    rawregs = [doki.registry_new(1, False) for i in range(nq)]
    regs = [doki.registry_apply(rawregs[i], gates[i], [0], None, None, False)
            for i in range(nq)]
    joined = regs[0]
    for i in range(nq):
        del rawregs[nq - i - 1]
    for i in range(1, nq):
        aux = doki.registry_join(joined, regs[i], False)
        if i > 1:
            del joined
        joined = aux
        npreg = doki_to_np(joined, i + 1, canonical=False)
        npreg = np.exp(-1j*np.angle(npreg[0, 0])) * npreg
        if not np.allclose(doki_to_np(joined, i + 1, canonical=True),
                           npreg,
                           rtol=rtol, atol=atol):
            raise AssertionError("Failed canonical get on joined state")
    for i in range(nq):
        del regs[nq - i - 1]
    print("\tTesting get after measure (join)")
    for i in range(nq - 1):
        aux, _ = doki.registry_measure(joined, 1, [np.random.rand()], False)
        del joined
        joined = aux
        npreg = doki_to_np(joined, nq - i - 1, canonical=False)
        npreg = np.exp(-1j*np.angle(npreg[0, 0])) * npreg
        if not np.allclose(doki_to_np(joined, nq - i - 1, canonical=True),
                           npreg,
                           rtol=rtol, atol=atol):
            raise AssertionError("Failed canonical get on measured state")
    del joined


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    seed = None
    if len(argv) == 3:
        seed = int(argv[2])
    if 2 <= len(argv) <= 3:
        min_qubits = int(argv[0])
        max_qubits = int(argv[1])
        if (min_qubits < 1):
            raise ValueError("minimum number of qubits must be at least 1")
        elif (min_qubits > max_qubits):
            raise ValueError("minimum can't be greater than maximum")
        if seed is not None and (seed < 0 or seed >= 2**32):
            raise ValueError("seed must be in [0, 2^32 - 1]")
        print("Get state without hidden phase tests...")
        if seed is None:
            seed = np.random.randint(np.iinfo(np.int32).max)
            print("\tSeed:", seed)
        np.random.seed(seed)
        rtol = 0
        atol = 1e-13
        for nq in range(min_qubits, max_qubits + 1):
            test_canonical_apply(nq, rtol, atol)
            if nq > 1:
                test_canonical_join_mes(nq, rtol, atol)
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits> <seed (optional)>")


if __name__ == "__main__":
    main()
