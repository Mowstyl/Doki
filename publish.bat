@ECHO OFF
python -m twine upload --repository testpypi dist/*
pause
