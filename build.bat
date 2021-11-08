@ECHO OFF
SET DokiVersion=1.3.0
:: Removing old windows wheels
del "dist\doki_gpu_Mowstyl-*-cp*-win_amd64.whl"

:: Cleaning Python3.9
py -3.9 -m pip uninstall doki_gpu_Mowstyl -y
rmdir "doki_gpu_Mowstyl.egg-info" /S /Q
:: Building Python3.9
py -3.9 -m build
:: Installing Python3.9
py -3.9 -m pip install --user dist/doki_gpu_Mowstyl-%DokiVersion%-cp39-cp39-win_amd64.whl

pause

:: Cleaning Python3.6
py -3.6 -m pip uninstall doki_gpu_Mowstyl -y
rmdir "doki_gpu_Mowstyl.egg-info" /S /Q
:: Building Python3.6
py -3.6 -m build
:: Installing Python3.6
py -3.6 -m pip install --user dist/doki_gpu_Mowstyl-%DokiVersion%-cp36-cp36m-win_amd64.whl

:: Cleaning Python3.7
py -3.7 -m pip uninstall doki_gpu_Mowstyl -y
rmdir "doki_gpu_Mowstyl.egg-info" /S /Q
:: Building Python3.7
py -3.7 -m build
:: Installing Python3.7
py -3.7 -m pip install --user dist/doki_gpu_Mowstyl-%DokiVersion%-cp37-cp37m-win_amd64.whl

:: Cleaning Python3.8
py -3.8 -m pip uninstall doki_gpu_Mowstyl -y
rmdir "doki_gpu_Mowstyl.egg-info" /S /Q
:: Building Python3.8
py -3.8 -m build
:: Installing Python3.8
py -3.8 -m pip install --user dist/doki_gpu_Mowstyl-%DokiVersion%-cp38-cp38-win_amd64.whl

pause
