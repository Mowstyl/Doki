"""Measurement tests."""
import doki
import gc
import numpy as np
import scipy.sparse as sparse
import sys

from one_gate_tests import apply_np, apply_gate
from reg_creation_tests import gen_reg, doki_to_np


def RZ_np(angle, invert):
    """Return numpy array with rotation gate around Z axis."""
    gate = np.zeros(4, dtype=complex).reshape(2, 2)
    if not invert:
        gate[0, 0] = np.cos(-angle/2) + np.sin(-angle/2) * 1j
        gate[1, 1] = np.cos(angle/2) + np.sin(angle/2) * 1j
    else:
        gate[0, 0] = np.cos(-angle/2) - np.sin(-angle/2) * 1j
        gate[1, 1] = np.cos(angle/2) - np.sin(angle/2) * 1j
    return gate


def compare_no_phase(num_qubits, r_doki, r_np, rtol, atol):
    """Compare Doki and Numpy registry ignoring hidden phase."""
    r_d_np = doki_to_np(r_doki, num_qubits)
    darg = np.angle(r_d_np[0, 0])
    r_d_np = np.power(np.e, darg * -1j) * r_d_np
    narg = np.angle(r_np[0, 0])
    r_np = np.power(np.e, narg * -1j) * r_np
    # print("DokiC:", r_d_np)
    # print("NumpyC:", r_np)
    return np.allclose(r_d_np, r_np, rtol=rtol, atol=atol)


def H_e_np(quantity):
    """Return a list of Hadamard gates multipied by Rz."""
    angle = (2 * np.pi) / quantity
    sqrt2_2 = np.sqrt(2) / 2
    h = np.array([[sqrt2_2, sqrt2_2], [sqrt2_2, -sqrt2_2]])
    return [np.dot(RZ_np(angle, False), h) for i in range(quantity)]


def get_H_e(quantity):
    """Return H_e_np in both sparse and doki formats."""
    h_es = H_e_np(quantity)
    h_e_doki = [doki.gate(1, h_e.tolist(), False) for h_e in h_es]
    h_e_sp = [sparse.csr_matrix(h_e) for h_e in h_es]
    return h_e_doki, h_e_sp


def build_np(gates, ids):
    """Apply gate[ids[i]] to qubit i of a new registry."""
    nq = len(ids)
    r_np = gen_reg(nq)
    for i in range(nq):
        aux = apply_np(nq, r_np, gates[ids[i]], i)
        del r_np
        r_np = aux
    return r_np


def check_build(num_qubits, h_e_doki, h_e_sp, rtol, atol):
    """Test registry after gate application."""
    r_doki = doki.new(num_qubits, False)
    r_np = gen_reg(num_qubits)
    for i in range(num_qubits):
        if h_e_sp[i] is not None and h_e_doki[i] is not None:
            aux_r_n, aux_r_d = apply_gate(num_qubits, r_np, r_doki,
                                          h_e_sp[i], h_e_doki[i], i)
            del r_doki
            del r_np
            r_doki = aux_r_d
            r_np = aux_r_n
    if not np.allclose(doki_to_np(r_doki, num_qubits), r_np,
                       rtol=rtol, atol=atol):
        raise AssertionError("Error applying gates")
    return r_doki, r_np


def check_nothing(num_qubits, r_doki, rtol, atol):
    """Test measurement with mask 0 for specified number of qubits."""
    aux_r_d, m = doki.measure(r_doki, 0, [], False)
    if (not np.allclose(doki_to_np(r_doki, num_qubits),
                        doki_to_np(aux_r_d, num_qubits),
                        rtol=rtol, atol=atol)) \
            or len(m) != num_qubits or not all(x is None for x in m):
        raise AssertionError("Error with mask 0")
    del m


def check_statistics(mess, iterations, bounds):
    """Test if the measure chances are within the expected range."""
    nq = mess.shape[1]
    total = mess.sum() / (iterations * nq)
    if total < bounds[3] or total > bounds[4]:
        print("\t[WARNING] Total chance", total * 100)
        print("\t\tTry to repeat the test with more iterations")
        if total < bounds[2] or total > bounds[5]:
            raise AssertionError("Measurement appears to be biased")
    partials = np.array([mess[:, i].sum() / iterations
                         for i in range(nq)])
    if any(prob < bounds[1] or prob > bounds[6] for prob in partials):
        print("\t[WARNING] Chance of triggering", partials * 100)
        print("\t\tTry to repeat the test with more iterations")
        if any(x < bounds[0] or x > bounds[7] for x in partials):
            raise AssertionError("Measurement appears to be biased " +
                                 "for some qubits")


def check_everything(num_qubits, r_doki, rtol, atol,
                     iterations, bounds, classic=None):
    """Test measurement with max mask for specified number of qubits."""
    # if classic is not None:
    #     print(doki_to_np(r_doki, num_qubits))
    aux_r_d, m = doki.measure(r_doki, 2**num_qubits - 1,
                              np.random.rand(num_qubits).tolist(), False)
    doki_errored = False
    try:
        doki.get(aux_r_d, 0, False)
    except Exception as e:
        if type(e).__module__ + "." + type(e).__name__ == "qsimov.doki.error":
            doki_errored = True
    if not doki_errored or len(m) != num_qubits \
            or not all(x is True or x is False for x in m):
        raise AssertionError("Error measuring all qubits")
    mess = np.zeros((iterations, num_qubits), dtype=int)
    for i in range(iterations):
        reg, mes = doki.measure(r_doki, 2**num_qubits - 1,
                                np.random.rand(num_qubits).tolist(), False)
        del reg
        for j in range(num_qubits):
            mess[i, j] = int(mes[j])
    gc.collect()
    if classic is None:
        check_statistics(mess, iterations, bounds)
    else:
        if not all(np.all(mes == classic) for mes in mess):
            for i in range(iterations):
                if not np.all(mess[i] == classic):
                    print(classic)
                    print(mess[i])
            raise AssertionError("Value differs from expected")
    del mess


def check_half(num_qubits, gates, r_doki, rtol, atol,
               iterations, bounds, classic=None):
    """Test measuring half qubits."""
    ids = [i for i in range(num_qubits)]
    mess = np.zeros((iterations, len(ids)//2), dtype=int)
    nots = []
    for i in range(iterations):
        not_ids = np.random.choice(ids, size=len(ids)//2, replace=False)
        nots.append(not_ids)
        yes_ids = np.setdiff1d(ids, not_ids)
        yes_ids.sort()
        mask = int(np.array([2**id for id in not_ids]).sum())
        # if classic is not None:
        #     print("Doki0:", doki_to_np(r_doki, num_qubits))
        #     print("Mask:", mask)
        aux_r_d, mes = doki.measure(r_doki, mask,
                                    np.random.rand(len(not_ids)).tolist(),
                                    False)
        # if classic is not None:
        #     print("Doki1:", doki_to_np(aux_r_d, len(yes_ids)))
        #     print("ref:", aux_r_d)
        aux_r_np = build_np(gates, yes_ids)
        if (not compare_no_phase(len(yes_ids), aux_r_d, aux_r_np,
                                 rtol, atol)) \
                or len(mes) != num_qubits:
            print("Numpy:", aux_r_np)
            print("Doki:", doki_to_np(aux_r_d, len(yes_ids)))
            raise AssertionError("Error measuring half qubits. Mask: " +
                                 str(mask))
        del aux_r_d
        mes = mes[::-1]
        for j in range(len(not_ids)):
            mess[i, j] = int(mes[not_ids[j]])
    gc.collect()
    if classic is None:
        check_statistics(mess, iterations, bounds)
    else:
        expected = [[classic[num_qubits - id - 1]
                     for id in nots[i]]
                    for i in range(iterations)]
        if not all(np.all(mess[i] == expected[i]) for i in range(iterations)):
            for i in range(iterations):
                if not np.all(mess[i] == expected[i]):
                    print(classic)
                    print(nots[i])
                    print(mess[i])
                    print(expected[i])
            raise AssertionError("Value differs from expected")


def check_measure_superposition(num_qubits, rtol, atol, iterations=1000,
                                bounds=[.3, .4, .4, .45, .55, .6, .6, .7]):
    """Test measurement with specified number of qubits and Hadamards."""
    h_e_doki, h_e_sp = get_H_e(num_qubits)
    r_doki, r_np = check_build(num_qubits, h_e_doki, h_e_sp, rtol, atol)
    print("\t\tTesting mask = 0")
    check_nothing(num_qubits, r_doki, rtol, atol)
    print("\t\tTesting mask = max")
    check_everything(num_qubits, r_doki, rtol, atol, iterations, bounds)
    if (num_qubits > 1):
        print("\t\tTesting mask = half")
        check_half(num_qubits, h_e_sp, r_doki, rtol, atol, iterations, bounds)
    del r_doki
    del r_np


def check_measure_classic(num_qubits, rtol, atol, iterations=1000):
    """Test measurement with specified number of qubits and X gates."""
    raw_x = [[0, 1], [1, 0]]
    x_sp = sparse.csr_matrix(raw_x)
    x_d = doki.gate(1, raw_x, False)
    values = np.random.choice(a=[0, 1], size=num_qubits)
    x_d_list = [x_d if value else None for value in values]
    x_sp_list = [x_sp if value else None for value in values]
    # print(values)
    r_doki, r_np = check_build(num_qubits, x_d_list, x_sp_list, rtol, atol)
    print("\t\tTesting mask = max")
    check_everything(num_qubits, r_doki, rtol, atol,
                     iterations, None, values[::-1])
    if (num_qubits > 1):
        print("\t\tTesting mask = half")
        check_half(num_qubits, x_sp_list, r_doki, rtol, atol,
                   iterations, None, values[::-1])
    del r_doki
    del r_np


def main():
    """Execute all tests."""
    argv = sys.argv[1:]
    seed = None
    iterations = 1000
    if len(argv) >= 3:
        iterations = int(argv[2])
    if len(argv) == 4:
        seed = int(argv[3])
    if 2 <= len(argv) <= 4:
        min_qubits = int(argv[0])
        max_qubits = int(argv[1])
        if (min_qubits < 1):
            raise ValueError("minimum number of qubits must be at least 1")
        elif (min_qubits > max_qubits):
            raise ValueError("minimum can't be greater than maximum")
        if iterations <= 0:
            raise ValueError("Iterations must be at least 1")
        if seed is not None and (seed < 0 or seed >= 2**32):
            raise ValueError("seed must be in [0, 2^32 - 1]")
        print("Measurement tests...")
        if seed is None:
            seed = np.random.randint(np.iinfo(np.int32).max)
            print("\tSeed:", seed)
        np.random.seed(seed)
        rtol = 0
        atol = 1e-13
        print("\tSuperposition tests...")
        for nq in range(min_qubits, max_qubits + 1):
            check_measure_superposition(nq, rtol, atol, iterations)
        gc.collect()
        print("\tClassic tests...")
        for nq in range(min_qubits, max_qubits + 1):
            check_measure_classic(nq, rtol, atol, iterations)
        gc.collect()
        print("\tPEACE AND TRANQUILITY")
    else:
        raise ValueError("Syntax: " + sys.argv[0] +
                         " <minimum number of qubits (min 1)>" +
                         " <maximum number of qubits>" +
                         " <iterations=1000> <seed (optional)>")


if __name__ == "__main__":
    main()
