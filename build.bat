@ECHO OFF
cls
rmdir "build" /S /Q
rmdir "doki_Mowstyl.egg-info" /S /Q
pip install --user --use-feature=in-tree-build .
