"""Probability tests."""
import doki
import numpy as np
import sys


def Ry_doki(angle):
    """Return a Ry gate."""
    npgate = np.array([[np.cos(angle / 2), -np.sin(angle / 2)],
                       [np.sin(angle / 2),  np.cos(angle / 2)]], dtype=complex)
    return doki.gate_new(1, npgate.tolist(), False)


def test_probability(nq, rtol, atol):
    """Test probability method with nq qubits registry."""
    step = 2 * np.pi / nq
    gates = [Ry_doki(step * i) for i in range(nq)]
    reg = doki.registry_new(nq, False)
    for i in range(nq):
        aux = doki.registry_apply(reg, gates[i], [i], None, None, False)
        del reg
        reg = aux
    for i in range(nq):
        odds = doki.registry_prob(reg, i, False)
        exodds = np.sin((step * i) / 2)**2
        if not np.allclose(odds, exodds, rtol=rtol, atol=atol):
            print(f"Obtained Odds: P(M({i})=1) = {odds}")
            print(f"Expected Odds: P(M({i})=1) = {exodds}")
            raise AssertionError("Failed canonical get on measured state")
    del reg


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
        print("Probability method tests...")
        rtol = 0
        atol = 1e-13
        for nq in range(min_qubits, max_qubits + 1):
            test_probability(nq, rtol, atol)
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits>")


if __name__ == "__main__":
    main()
