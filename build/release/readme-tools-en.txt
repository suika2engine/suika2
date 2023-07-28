===============
Suika2 Variants
===============

Please copy each tool to the folder that contains suika.exe (or with macOS equivalent, extract the DMG there) and run it!

##
## Testing Tools
##
suika-capture.exe ... This records game play into the 'record' folder
suika-replay.exe  ... This replays from the record folder and outputs to the replay folder

 * Usage
  A. For bug reproduction:
     - Record test playthroughs and reproduce bugs at any time.
  B. For regression detection:
     - Record before the change and replay after the change,
       then you can find regression if the record and replay folders differ.

 * TIPS
  - You can check the differecne between the record and replay folders using
    a free software called "WinMerge". (difference is a bug)
  - You can rename the record folder, and drop it onto suika-replay.exe

##
## For Windows
##
suika-64.exe    ... Slightly faster on 64-bit Windows machines
suika-arm64.exe ... Quite faster on Arm64 Windows (such as the Surface Pro) machines

##
## For Mac
##
suika-capture.dmg ... The capture app for macOS
suika-replay.dmg ... The replay app for macOS

##
## For Linux
##
suika-linux        ... For Linux (Steam)
suika-linux-replay ... For automatic testing on GitHub
