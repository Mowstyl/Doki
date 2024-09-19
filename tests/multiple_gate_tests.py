"""Multiple qubit gate tests."""
import argparse
import doki as doki
import numpy as np
import scipy.sparse as sparse
import time as t

from reg_creation_tests import gen_reg, doki_to_np
from one_gate_tests import Identity, U_np, U_sparse, U_doki, \
                           apply_np, apply_gate
from timed_test import debug, error, init_args


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


def SWAP_doki(verbose):
    """Return doki SWAP gate."""
    return doki.gate_new(1, SWAP_np().tolist(), verbose)


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


def multiple_target_tests(nq, rtol, atol, num_threads, prng, verbose):
    """Test multiple qubit gate."""
    angles1 = prng.random(3)
    angles2 = prng.random(3)
    invert1 = prng.choice(a=[False, True])
    invert2 = prng.choice(a=[False, True])
    numpygate = TwoU_np(*angles1, invert1, *angles2, invert2)
    sparsegate = sparse.csr_matrix(numpygate)
    dokigate = doki.gate_new(2, numpygate.tolist(), verbose)
    r1_np = gen_reg(nq)
    r1_doki = doki.registry_new(nq, verbose)
    for id1 in range(nq):
        for id2 in range(nq):
            if id1 == id2:
                continue
            r2_np = sparseTwoGate(sparsegate, id1, id2, nq, r1_np)
            r2_doki = doki.registry_apply(r1_doki, dokigate, [id1, id2],
                                          None, None, num_threads, verbose)
            if not np.allclose(doki_to_np(r2_doki, nq, verbose), r2_np,
                               rtol=rtol, atol=atol):
                debug(r2_np)
                debug(doki_to_np(r2_doki, nq, verbose))
                debug(r2_np == doki_to_np(r2_doki, nq, verbose))
                error("Error comparing results of two qubit gate", fatal=True)
            del r2_doki


def controlled_tests(nq, rtol, atol, num_threads, prng, verbose):
    """Test application of controlled gates."""
    isControl = prng.choice(a=[False, True])
    qubitIds = [int(id) for id in prng.permutation(nq)]
    lastid = qubitIds[0]
    control = []
    anticontrol = []
    angles = prng.random(3)
    invert = prng.choice(a=[False, True])
    invstr = ""
    if invert:
        invstr = "-1"
    numpygate = U_sparse(*angles, invert)
    gate = U_doki(*angles, invert, verbose)
    r1_np = gen_reg(nq)
    r1_doki = doki.registry_new(nq, verbose)
    # print(nq)
    r2_np, r2_doki = apply_gate(nq, r1_np, r1_doki, numpygate, gate, lastid,
                                num_threads, verbose)
    del r1_np
    del r1_doki
    for id in qubitIds[1:]:
        r1_np = r2_np
        r1_doki = r2_doki
        if isControl:
            control.append(lastid)
        else:
            anticontrol.append(lastid)
        debug("\t\tid:", id)
        debug("\t\tcontrols:", control)
        debug("\t\tanticontrols:", anticontrol)
        r2_np = applyCACU(numpygate, id, control, anticontrol, nq, r1_np)
        r2_doki = doki.registry_apply(r1_doki, gate, [int(id)],
                                      set(control), set(anticontrol),
                                      num_threads, verbose)
        isControl = not isControl
        lastid = id
        if not np.allclose(doki_to_np(r2_doki, nq, verbose), r2_np,
                           rtol=rtol, atol=atol):
            debug(f"\t\tGate: U({angles}){invstr} to qubit {lastid}")
            debug(r2_np)
            debug(doki_to_np(r2_doki, nq, verbose))
            debug(r2_np == doki_to_np(r2_doki, nq, verbose))
            error("Error comparing results of controlled gates", fatal=True)
        del r1_np
        del r1_doki
    return True


def main(min_qubits, max_qubits, num_threads, prng, verbose):
    """Execute all tests."""
    rtol = 0
    atol = 1e-13
    print("\tControlled gate application tests...")
    a = t.time()
    for nq in range(min_qubits, max_qubits + 1):
        controlled_tests(nq, rtol, atol, num_threads, prng, verbose)
    b = t.time()
    print("\tMultiple target gate application tests...")
    c = t.time()
    for nq in range(min_qubits, max_qubits + 1):
        multiple_target_tests(nq, rtol, atol, num_threads, prng, verbose)
    d = t.time()
    print(f"\tPEACE AND TRANQUILITY: {(b - a) + (d - c)} s")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="MultipleGateTests",
                                     description="Checks if apply_gate method works with multi-qubit gates")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, help="whether to print extra information or not")
    parser.add_argument("-n", "--num_qubits", type=int, required=True, help="the starting number of qubits to use")
    parser.add_argument("-m", "--max_qubits", type=int, default=None, help="the max number of qubits to use")
    parser.add_argument("-t", "--num_threads", type=int, default=None, help="the number of threads to use")
    # parser.add_argument("-i", "--iterations", type=int, required=True, help="how many times the test target has to be executed")
    parser.add_argument("-s", "--seed", type=int, default=None, help="sets the seed to use")
    args = parser.parse_args()

    print("Multiple qubit gate application tests:")
    prng = init_args(args)
    main(args.num_qubits, args.max_qubits, args.num_threads, prng, args.verbose)
