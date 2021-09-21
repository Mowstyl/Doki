@ECHO OFF
docker run --name manylinux2010 --rm -v %cd%:/doki -d -i -t quay.io/pypa/manylinux2010_x86_64
docker exec manylinux2010 /bin/bash -c "mkdir /output"
docker exec manylinux2010 /bin/bash -c "python3.6 -m pip wheel /doki -w /output"
docker exec manylinux2010 /bin/bash -c "python3.7 -m pip wheel /doki -w /output"
docker exec manylinux2010 /bin/bash -c "python3.8 -m pip wheel /doki -w /output"
docker exec manylinux2010 /bin/bash -c "ls -lah /output"
docker exec manylinux2010 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp36*whl -w /doki/dist"
docker exec manylinux2010 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp37*whl -w /doki/dist"
docker exec manylinux2010 /bin/bash -c "auditwheel repair /output/doki_Mowstyl*cp38*whl -w /doki/dist"
docker container stop manylinux2010
pause
