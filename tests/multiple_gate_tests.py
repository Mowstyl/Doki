"""Multiple qubit gate tests."""
import doki
import numpy as np
import scipy.sparse as sparse
import sys

from reg_creation_tests import gen_reg, doki_to_np
from one_gate_tests import Identity, U_np, U_sparse, U_doki, \
                           apply_np, apply_gate


def swap_downstairs(id1, id2, nq, reg):
    """Swap qubit id1 with next qubit until reaches id2 (id1 < id2)."""
    swap = SWAP_np()
    for i in range(id1, id2):
        reg = apply_np(nq, reg, swap, i)
    return reg


def swap_upstairs(id1, id2, nq, reg):
    """Swap qubit id1 with next qubit until reaches id2 (id1 > id2)."""
    swap = SWAP_np()
    for i in range(id2 - 1, id1 - 1, -1):
        reg = apply_np(nq, reg, swap, i)
    return reg


def swap_downstairs_list(id1, id2, li):
    """Swap list element id1 with the next until reaches id2 (id1 > id2)."""
    for i in range(id1, id2):
        li[i], li[i+1] = li[i+1], li[i]
    return li


def swap_upstairs_list(id1, id2, li):
    """Swap list element id1 with the next until reaches id2 (id1 < id2)."""
    for i in range(id2 - 1, id1 - 1, -1):
        li[i], li[i+1] = li[i+1], li[i]
    return li


def CU(gate, ncontrols):
    """Return n-controlled version of given gate."""
    nqgate = int(np.log2(gate.shape[0]))
    cu = np.eye(2**(nqgate+ncontrols), dtype=complex)
    aux = cu.shape[0] - gate.shape[0]
    for i in range(gate.shape[0]):
        for j in range(gate.shape[1]):
            cu[aux + i, aux + j] = gate[i, j]

    return sparse.csr_matrix(cu)


def negateQubits(qubits, nq, reg):
    """Apply X gate to qubit ids specified."""
    for id in qubits:
        reg = apply_np(nq, reg, np.array([[0, 1], [1, 0]]), id)
    return reg


def sparseTwoGate(gate, raw_id1, raw_id2, nq, reg):
    """Apply a gate to two qubits that might not be next to each other."""
    if raw_id2 < raw_id1:
        id1, id2 = raw_id2, raw_id1
    else:
        id1, id2 = raw_id1, raw_id2
    if id1 < 0 or id2 >= nq:
        reg = None
    else:
        if id2 - id1 > 1:
            reg = swap_downstairs(id1, id2 - 1, nq, reg)
        if raw_id2 < raw_id1:
            reg = apply_np(nq, reg, SWAP_np(), id2 - 1)
        reg = apply_np(nq, reg, gate, id2 - 1)
        if raw_id2 < raw_id1:
            reg = apply_np(nq, reg, SWAP_np(), id2 - 1)
        if id2 - id1 > 1:
            reg = swap_upstairs(id1, id2 - 1, nq, reg)
    return reg


def applyCACU(gate, id, controls, anticontrols, nq, reg):
    """Apply gate with specified controls and anticontrols."""
    cset = set(controls)
    acset = set(anticontrols)
    cuac = list(cset.union(acset))
    if type(id) == list:
        extended_cuac = id + cuac
    else:
        extended_cuac = [id] + cuac
    qubitIds = [i for i in range(nq)]

    reg = negateQubits(acset, nq, reg)
    for i in range(len(extended_cuac)):
        if qubitIds[i] != extended_cuac[i]:
            indaux = qubitIds.index(extended_cuac[i])
            reg = swap_upstairs(i, indaux, nq, reg)
            qubitIds = swap_upstairs_list(i, indaux, qubitIds)
    reg = apply_np(nq, reg, CU(gate, len(cuac)), 0)
    for i in range(nq):
        if qubitIds[i] != i:
            indaux = qubitIds.index(i)
            reg = swap_upstairs(i, indaux, nq, reg)
            qubitIds = swap_upstairs_list(i, indaux, qubitIds)
    reg = negateQubits(acset, nq, reg)
    return reg


def SWAP_np():
    """Return numpy array with SWAP gate."""
    gate = np.zeros(4 * 4, dtype=complex)
    gate = gate.reshape(4, 4)

    gate[0][0] = 1
    gate[1][2] = 1
    gate[2][1] = 1
    gate[3][3] = 1

    return gate


def SWAP_sparse():
    """Return scipy sparse CSR matrix with SWAP gate."""
    return sparse.csr_matrix(SWAP_np())


def SWAP_doki():
    """Return doki SWAP gate."""
    return doki.gate(1, SWAP_np().tolist())


def TwoU_np(angle1_1, angle1_2, angle1_3, invert1,
            angle2_1, angle2_2, angle2_3, invert2):
    """Return numpy two qubit gate that may entangle."""
    U1 = U_np(angle1_1, angle1_2, angle1_3, invert1)
    U2 = U_np(angle2_1, angle2_2, angle2_3, invert2)
    g1 = sparse.kron(U1, Identity(1))
    g2 = np.eye(4, dtype=complex)
    g2[2, 2] = U2[0, 0]
    g2[2, 3] = U2[0, 1]
    g2[3, 2] = U2[1, 0]
    g2[3, 3] = U2[1, 1]
    g = g2.dot(g1.toarray())
    return g


def multiple_target_tests(nq, rtol, atol, verbose=True):
    """Test multiple qubit gate."""
    angles1 = np.random.rand(3)
    angles2 = np.random.rand(3)
    invert1 = np.random.choice(a=[False, True])
    invert2 = np.random.choice(a=[False, True])
    numpygate = TwoU_np(*angles1, invert1, *angles2, invert2)
    sparsegate = sparse.csr_matrix(numpygate)
    dokigate = doki.gate(2, numpygate.tolist(), False)
    r1_np = gen_reg(nq)
    r1_doki = doki.new(nq, False)
    for id1 in range(nq):
        for id2 in range(nq):
            if id1 == id2:
                continue
            r2_np = sparseTwoGate(sparsegate, id1, id2, nq, r1_np)
            r2_doki = doki.apply(r1_doki, dokigate, [id1, id2],
                                 None, None, False)
            if not np.allclose(doki_to_np(r2_doki, nq), r2_np,
                               rtol=rtol, atol=atol):
                if verbose:
                    print(r2_np)
                    print(doki_to_np(r2_doki, nq))
                    print(r2_np == doki_to_np(r2_doki, nq))
                raise AssertionError("Error comparing results of two" +
                                     " qubit gate")
            del r2_doki


def controlled_tests(nq, rtol, atol, verbose=False):
    """Test application of controlled gates."""
    isControl = np.random.choice(a=[False, True])
    qubitIds = [int(id) for id in np.random.permutation(nq)]
    lastid = qubitIds[0]
    control = []
    anticontrol = []
    angles = np.random.rand(3)
    invert = np.random.choice(a=[False, True])
    invstr = ""
    if invert:
        invstr = "-1"
    numpygate = U_sparse(*angles, invert)
    gate = U_doki(*angles, invert)
    r1_np = gen_reg(nq)
    r1_doki = doki.new(nq, False)
    # print(nq)
    r2_np, r2_doki = apply_gate(nq, r1_np, r1_doki, numpygate, gate, lastid)
    del r1_np
    del r1_doki
    for id in qubitIds[1:]:
        r1_np = r2_np
        r1_doki = r2_doki
        if isControl:
            control.append(lastid)
        else:
            anticontrol.append(lastid)
        if verbose:
            print("   id: " + str(id))
            print("   controls: " + str(control))
            print("   anticontrols: " + str(anticontrol))
        r2_np = applyCACU(numpygate, id, control, anticontrol, nq, r1_np)
        r2_doki = doki.apply(r1_doki, gate, [int(id)],
                             set(control), set(anticontrol), False)
        isControl = not isControl
        lastid = id
        if not np.allclose(doki_to_np(r2_doki, nq), r2_np,
                           rtol=rtol, atol=atol):
            if verbose:
                print("\t\tGate: U(" + angles + ")" + invstr +
                      " to qubit " + str(lastid))
                print(r2_np)
                print(doki_to_np(r2_doki, nq))
                print(r2_np == doki_to_np(r2_doki, nq))
            raise AssertionError("Error comparing results of controlled gates")
        del r1_np
        del r1_doki
    return True


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    seed = None
    if len(argv) == 3:
        seed = int(argv[2])
    if 2 <= len(argv) <= 3:
        min_qubits = int(argv[0])
        max_qubits = int(argv[1])
        if (min_qubits < 2):
            raise ValueError("minimum number of qubits must be at least 2")
        elif (min_qubits > max_qubits):
            raise ValueError("minimum can't be greater than maximum")
        if seed is not None and (seed < 0 or seed >= 2**32):
            raise ValueError("seed must be in [0, 2^32 - 1]")
        print("Multiple qubit gate application tests...")
        if seed is None:
            seed = np.random.randint(np.iinfo(np.int32).max)
            print("\tSeed:", seed)
        np.random.seed(seed)
        rtol = 0
        atol = 1e-13
        print("\tControlled gate application tests...")
        for nq in range(min_qubits, max_qubits + 1):
            controlled_tests(nq, rtol, atol)
        print("\tMultiple target gate application tests...")
        for nq in range(min_qubits, max_qubits + 1):
            multiple_target_tests(nq, rtol, atol)
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 2)>" +
                         " <maximum number of qubits> <seed (optional)>")


if __name__ == "__main__":
    main()
