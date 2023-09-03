Suika2 Capture-and-Replay Compare Tool
======================================
This program compares the `record` and the `replay` folders.

## Build on WSL2
Run the following commands:
```
make setup
make bin
```

Then `suika-compare.zip` will be created.
Use `suika-compare.exe` inside `suika-compare.zip`.

## Run on Windows
1. Prepare `record` folder by running `suika-capture.exe`
2. Prepare `replay` folder by running `suika-replay.exe`
3. Click `suika-compare.exe`
4. `diff.txt` will be created if difference is detected

## Run on Other Platforms
1. Prepare `record` folder by running `suika-capture.exe`
2. Prepare `replay` folder by running `suika-replay.exe`
3. Run `compare.py` in your game folder
4. `diff.txt` will be created if difference is detected
