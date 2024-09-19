@ECHO OFF
echo Building sources...
python -m build --sdist
echo Done!
pause
echo Building wheels...
python -m cibuildwheel --output-dir wheelhouse
echo Done!
pause
