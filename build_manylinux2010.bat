@ECHO OFF
:: Run container and mount doki directory
docker run --name manylinux2010 --rm -v %cd%:/doki -d -i -t quay.io/pypa/manylinux2010_x86_64

:: Create output directory
docker exec manylinux2010 /bin/bash -c "mkdir /output"

:: Build doki for all available python versions
docker exec manylinux2010 /bin/bash -c "python3.6 -m pip wheel /doki -w /output --use-feature=in-tree-build "
docker exec manylinux2010 /bin/bash -c "python3.7 -m pip wheel /doki -w /output --use-feature=in-tree-build "
docker exec manylinux2010 /bin/bash -c "python3.8 -m pip wheel /doki -w /output --use-feature=in-tree-build "
docker exec manylinux2010 /bin/bash -c "python3.9 -m pip wheel /doki -w /output --use-feature=in-tree-build "
:: Scipy doesn't have wheels for 3.10, compile takes too much time. Can't be tested -> not wheels offered
::docker exec manylinux2010 /bin/bash -c "python3.10 -m pip wheel /doki -w /output --use-feature=in-tree-build "

:: Create manylinux builds from the raw builds
docker exec manylinux2010 /bin/bash -c "ls -lah /output"
docker exec manylinux2010 /bin/bash -c "auditwheel repair /output/doki_gpu_Mowstyl*cp36*whl -w /doki/dist"
docker exec manylinux2010 /bin/bash -c "auditwheel repair /output/doki_gpu_Mowstyl*cp37*whl -w /doki/dist"
docker exec manylinux2010 /bin/bash -c "auditwheel repair /output/doki_gpu_Mowstyl*cp38*whl -w /doki/dist"
docker exec manylinux2010 /bin/bash -c "auditwheel repair /output/doki_gpu_Mowstyl*cp39*whl -w /doki/dist"
::docker exec manylinux2010 /bin/bash -c "auditwheel repair /output/doki_gpu_Mowstyl*cp310*whl -w /doki/dist"

:: Install doki for all available python versions
docker exec manylinux2010 /bin/bash -c "python3.6 -m pip install --user /doki/dist/doki_gpu_Mowstyl*cp36*manylinux2010*.whl"
docker exec manylinux2010 /bin/bash -c "python3.7 -m pip install --user /doki/dist/doki_gpu_Mowstyl*cp37*manylinux2010*.whl"
docker exec manylinux2010 /bin/bash -c "python3.8 -m pip install --user /doki/dist/doki_gpu_Mowstyl*cp38*manylinux2010*.whl"
docker exec manylinux2010 /bin/bash -c "python3.9 -m pip install --user /doki/dist/doki_gpu_Mowstyl*cp39*manylinux2010*.whl"
::docker exec manylinux2010 /bin/bash -c "python3.10 -m pip install --user /doki/dist/doki_gpu_Mowstyl*cp310*manylinux2010*.whl"

:: Install dependencies for tests
docker exec manylinux2010 /bin/bash -c "python3.6 -m pip install --user numpy scipy"
docker exec manylinux2010 /bin/bash -c "python3.7 -m pip install --user numpy scipy"
docker exec manylinux2010 /bin/bash -c "python3.8 -m pip install --user numpy scipy"
docker exec manylinux2010 /bin/bash -c "python3.9 -m pip install --user numpy scipy"
::docker exec manylinux2010 /bin/bash -c "yum install -y blas-devel lapack-devel atlas-devel"
::docker exec manylinux2010 /bin/bash -c "python3.10 -m pip install --user numpy scipy"

:: Run tests for all available python versions
:: Python 3.6
echo Python3.6
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux2010 /bin/bash -c "python3.6 /doki/tests/probability_tests.py 1 5 2"
:: Python 3.7
echo Python3.7
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux2010 /bin/bash -c "python3.7 /doki/tests/probability_tests.py 1 5 2"
:: Python 3.8
echo Python3.8
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux2010 /bin/bash -c "python3.8 /doki/tests/probability_tests.py 1 5 2"
:: Python 3.9
echo Python3.9
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/reg_creation_tests.py 1 5"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/one_gate_tests.py 1 5 1"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/one_gate_tests.py 1 5 2"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/measure_tests.py 1 5 1000 1"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/measure_tests.py 1 5 1000 2"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/multiple_gate_tests.py 2 5 1"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/multiple_gate_tests.py 2 5 2"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/join_regs_tests.py 5 1"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/join_regs_tests.py 5 2"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/canonical_form_tests.py 1 5"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/probability_tests.py 1 5 1"
docker exec manylinux2010 /bin/bash -c "python3.9 /doki/tests/probability_tests.py 1 5 2"
:: Python 3.10
::echo Python3.10
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/reg_creation_tests.py 1 5"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/one_gate_tests.py 1 5 1"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/one_gate_tests.py 1 5 2"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/measure_tests.py 1 5 1000 1"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/measure_tests.py 1 5 1000 2"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/multiple_gate_tests.py 2 5 1"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/multiple_gate_tests.py 2 5 2"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/join_regs_tests.py 5 1"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/join_regs_tests.py 5 2"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/canonical_form_tests.py 1 5"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/probability_tests.py 1 5 1"
::docker exec manylinux2010 /bin/bash -c "python3.10 /doki/tests/probability_tests.py 1 5 2"

:: Stop the container
docker container stop manylinux2010
pause
