import argparse
import doki as doki
import numpy as np
import os
import sys
import time as t

from numpy.random import SeedSequence


def main(min_qubits, max_qubits, num_threads, prng):
    x_np = np.array([[0, 1], [1, 0]])
    x_doki = doki.gate_new(1, x_np.tolist(), _verbose)

    sqrt2_2 = np.sqrt(2) / 2
    h_np = np.array([[sqrt2_2, sqrt2_2], [sqrt2_2, -sqrt2_2]])
    h_doki = doki.gate_new(1, h_np.tolist(), _verbose)

    times = np.empty(max_qubits + 1 - min_qubits, dtype=float)
    for num_qubits in range(min_qubits, max_qubits + 1):
        debug(f"\tChecking time needed with {num_qubits} qubits...")
        toggle = True
        controls = {0}
        anticontrols = set()
        a = t.time()
        r_doki = doki.registry_new(num_qubits, _verbose)
        r_doki = doki.registry_apply(r_doki, h_doki, [0], None, None, num_threads, _verbose)
        diff = t.time() - a
        for i in range(1, num_qubits):
            a = t.time()
            r_doki = doki.registry_apply(r_doki, x_doki, [i], controls, anticontrols, num_threads, _verbose)
            diff += (t.time() - a)
            if toggle:
                anticontrols.add(i)
            else:
                controls.add(i)
            toggle = not toggle
        a = t.time()
        r_doki, _ = doki.registry_measure(r_doki, (1 << num_qubits - 1), [prng.random() for i in range(num_qubits)], num_threads, _verbose)
        diff += (t.time() - a)
        times[num_qubits-min_qubits] = diff
        debug(f"\t\t{diff} s")
    print("\tTimes:", str(times).replace("\n", "\n\t       "))
    print("\tTotal:", times.sum(), "s")


def error(*args, fatal=False, prefix="", error_code=1, **kwargs):
    if fatal:
        prefix = prefix + "[ERROR]"
    else:
        prefix = prefix + "[WARNING]"
    print(prefix, *args, file=sys.stderr, **kwargs)
    if fatal:
        exit(error_code)


def debug(*args, **kwargs):
    if _verbose:
        print(*args, **kwargs)


def init_args(args):
    global _verbose
    _verbose = args.verbose
    prng = None
    if hasattr(args, "seed"):
        if args.seed is not None and args.seed < 0:
            error("The seed cannot be a negative integer", fatal=True)
        ss = SeedSequence(args.seed)
        print(f"\tUsing seed = {ss.entropy}")
        prng = np.random.default_rng(ss)
        # mini_seed = int(prng.integers(0, np.iinfo(np.int32).max))
        # debug(f"\tmini_seed: {mini_seed}")
    if hasattr(args, "num_qubits"):
        if args.max_qubits is None:
            args.max_qubits = args.num_qubits
            debug(f"\tnum_qubits: {args.num_qubits}")
        else:
            if args.num_qubits > args.max_qubits:
                error("num_qubits has to be smaller than max_qubits. Inverting parameters...")
                args.num_qubits, args.max_qubits = args.max_qubits, args.num_qubits
            debug(f"\tmin: {args.num_qubits}", "max: " + str(args.max_qubits))
        if args.num_qubits <= 0:
            error("The number of qubits has to be a strictly positive integer", fatal=True)
    if hasattr(args, "iterations"):
        if args.iterations < 1:
            error("The number of iterations has to be a strictly positive integer", fatal=True)
    if hasattr(args, "num_threads"):
        if args.num_threads is None:
            args.num_threads = os.getenv('OMP_NUM_THREADS')
            if args.num_threads is not None:
                args.num_threads = int(args.num_threads)
        if args.num_threads is None:
            args.num_threads = -1
        elif args.num_threads == 0 or args.num_threads < -1:
            error("The number of threads must be a strictly positive integer (or -1 to let OpenMP decide)", fatal=True)
        print("\tNumber of threads:", args.num_threads)
    return prng


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="DokiTime",
                                     description="Measures the time needed to execute a predefined quantum program with the specified number of qubits")
    parser.add_argument("-v", "--verbose", action="store_true", default=False, help="whether to print extra information or not")
    parser.add_argument("-n", "--num_qubits", type=int, required=True, help="the starting number of qubits to use")
    parser.add_argument("-m", "--max_qubits", type=int, default=None, help="the max number of qubits to use")
    parser.add_argument("-t", "--num_threads", type=int, default=None, help="the number of threads to use")
    # parser.add_argument("-i", "--iterations", type=int, required=True, help="how many times the test target has to be executed")
    parser.add_argument("-s", "--seed", type=int, default=None, help="sets the seed to use")
    args = parser.parse_args()

    print("Time needed to run sample program:")
    prng = init_args(args)
    main(args.num_qubits, args.max_qubits, args.num_threads, prng)
