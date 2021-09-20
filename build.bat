@ECHO OFF
pip uninstall doki_Mowstyl -y
rmdir "doki_Mowstyl.egg-info" /S /Q
rmdir "dist" /S /Q
python -m build
pip install --user dist/doki_Mowstyl-1.0.0-cp38-cp38-win_amd64.whl
