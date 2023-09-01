Suika2 Cahpture-and-Replay Comparator
=====================================
This program comares `record` and `replay` data of Suika2.

## Prerequisites
* Python3

## Build on Windows
```
build.bat
```

## Run on Windows
1. Prepare `record` folder by running `suika-capture.exe`
2. Prepare `replay` folder by running `suika-replay.exe`
3. Click `suika-compare.exe`
4. `diff` folder will be created if difference is detected

## Run on Other Platforms
1. Prepare `record` folder by running `suika-capture.exe`
2. Prepare `replay` folder by running `suika-replay.exe`
3. Run `compare.py` in your game folder
4. `diff` folder will be created if difference is detected
