@ECHO OFF
:: Run container and mount doki directory
docker run --name manylinux_2_28 --rm -v %cd%:/doki -d -i -t quay.io/pypa/manylinux_2_28_x86_64

:: Create output directory
docker exec manylinux_2_28 /bin/bash -c "mkdir /output"

:: Build doki for all available python versions
docker exec manylinux_2_28 /bin/bash -c "python3.6 -m pip wheel /doki -w /output --use-feature=in-tree-build "
docker exec manylinux_2_28 /bin/bash -c "python3.7 -m pip wheel /doki -w /output "
docker exec manylinux_2_28 /bin/bash -c "python3.8 -m pip wheel /doki -w /output "
docker exec manylinux_2_28 /bin/bash -c "python3.9 -m pip wheel /doki -w /output "
:: Scipy doesn't have wheels for 3.10, compile takes too much time. Can't be tested -> not wheels offered
docker exec manylinux_2_28 /bin/bash -c "python3.10 -m pip wheel /doki -w /output "
docker exec manylinux_2_28 /bin/bash -c "python3.11 -m pip wheel /doki -w /output "

:: Create manylinux builds from the raw builds
docker exec manylinux_2_28 /bin/bash -c "ls -lah /output"
docker exec manylinux_2_28 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp36*whl -w /doki/dist"
docker exec manylinux_2_28 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp37*whl -w /doki/dist"
docker exec manylinux_2_28 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp38*whl -w /doki/dist"
docker exec manylinux_2_28 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp39*whl -w /doki/dist"
docker exec manylinux_2_28 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp310*whl -w /doki/dist"
docker exec manylinux_2_28 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp311*whl -w /doki/dist"

:: Install doki for all available python versions
docker exec manylinux_2_28 /bin/bash -c "python3.6 -m pip install --user /doki/dist/doki_Mowstyl*cp36*manylinux_2_28*.whl"
docker exec manylinux_2_28 /bin/bash -c "python3.7 -m pip install --user /doki/dist/doki_Mowstyl*cp37*manylinux_2_28*.whl"
docker exec manylinux_2_28 /bin/bash -c "python3.8 -m pip install --user /doki/dist/doki_Mowstyl*cp38*manylinux_2_28*.whl"
docker exec manylinux_2_28 /bin/bash -c "python3.9 -m pip install --user /doki/dist/doki_Mowstyl*cp39*manylinux_2_28*.whl"
docker exec manylinux_2_28 /bin/bash -c "python3.10 -m pip install --user /doki/dist/doki_Mowstyl*cp310*manylinux_2_28*.whl"
docker exec manylinux_2_28 /bin/bash -c "python3.11 -m pip install --user /doki/dist/doki_Mowstyl*cp311*manylinux_2_28*.whl"

pause

:: Install dependencies for tests
docker exec manylinux_2_28 /bin/bash -c "python3.6 -m pip install --user numpy scipy"
docker exec manylinux_2_28 /bin/bash -c "python3.7 -m pip install --user numpy scipy"
docker exec manylinux_2_28 /bin/bash -c "python3.8 -m pip install --user numpy scipy"
docker exec manylinux_2_28 /bin/bash -c "python3.9 -m pip install --user numpy scipy"
::docker exec manylinux_2_28 /bin/bash -c "apt update"
::docker exec manylinux_2_28 /bin/bash -c "apt install -y libblas-dev liblapack-dev libatlas-base-dev"
docker exec manylinux_2_28 /bin/bash -c "python3.10 -m pip install --user numpy scipy"
docker exec manylinux_2_28 /bin/bash -c "python3.11 -m pip install --user numpy scipy"

:: Run tests for all available python versions
:: Python 3.6
echo Python3.6
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/probability_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.6 /doki/tests/density_matrix_tests.py 1 5"
:: Python 3.7
echo Python3.7
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/probability_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.7 /doki/tests/density_matrix_tests.py 1 5"
:: Python 3.8
echo Python3.8
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/probability_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.8 /doki/tests/density_matrix_tests.py 1 5"
:: Python 3.9
echo Python3.9
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/probability_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.9 /doki/tests/density_matrix_tests.py 1 5"
:: Python 3.10
echo Python3.10
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/probability_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.10 /doki/tests/density_matrix_tests.py 1 5"
:: Python 3.11
echo Python3.11
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/probability_tests.py 1 5 2"
docker exec manylinux_2_28 /bin/bash -c "python3.11 /doki/tests/density_matrix_tests.py 1 5"

:: Stop the container
docker container stop manylinux_2_28
pause
