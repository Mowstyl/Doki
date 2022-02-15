@ECHO OFF
python tests/reg_creation_tests.py 1 14
python tests/one_gate_tests.py 1 14 1
python tests/one_gate_tests.py 1 14 8
python tests/measure_tests.py 1 10 1000 1
python tests/measure_tests.py 1 10 1000 8
python tests/multiple_gate_tests.py 2 10 1
python tests/multiple_gate_tests.py 2 10 8
python tests/join_regs_tests.py 14 1
python tests/join_regs_tests.py 14 8
python tests/canonical_form_tests.py 1 14
python tests/probability_tests.py 1 14 1
python tests/probability_tests.py 1 14 8
python tests/density_matrix_tests.py 1 5
pause
