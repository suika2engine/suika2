Suika2 Capture-and-Replay Compare Tool
======================================

This program compares the `record` folder and the `replay` folder.

# Preparation

1. Run `suika-capture.exe`
  - Play logs will be recorded into `record` folder
2. Update `suika.exe`
3. Run `suika-replay.exe`
  - The play logs in `record` folder will be replayed
  - Replay results will be stored in the `replay` folder

# Run

Run `suika-compare.exe` and it will compare the `record` and the `replay` folders.

If there is no difference, this program will show "OK: no diff."

If there are differences, this program will show "BAD: xx diffs.",
`diff.txt` will be created. It contains file names of screenshots that have differences.
