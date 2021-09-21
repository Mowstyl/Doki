@ECHO OFF
python tests/reg_creation_tests.py 1 14
python tests/one_gate_tests.py 1 14
python tests/measure_tests.py 1 10 1000
python tests/multiple_gate_tests.py 2 10
python tests/join_regs_tests.py 14
python tests/canonical_form_tests.py 1 14
python tests/probability_tests.py 1 14
pause
