@echo off

python3 -m pip install pyinstaller
python3 -m PyInstaller --onefile --noconsole routes.py
copy dist\routes.exe suika-routes.exe
rmdir /s /q build dist
del /s routes.spec
