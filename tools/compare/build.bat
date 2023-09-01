@echo off

python3 -m pip install opencv-python numpy pyinstaller
python3 -m PyInstaller --onefile compare.py
copy dist\compare.exe suika-compare.exe
rmdir /s /q build dist
del /s compare.spec
