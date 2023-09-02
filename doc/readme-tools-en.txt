===============
Suika2 Variants
===============

Please copy each tool to the folder that contains suika.exe and run it!

##
## For Windows
##

suika-64.exe    ... Slightly faster on 64-bit Windows machines
suika-arm64.exe ... Quite faster on Arm64 Windows (such as the Surface Pro) machines

##
## Testing Tools
##

suika-capture.exe ... This records game play into the 'record' folder
suika-replay.exe  ... This replays from the 'record' folder and outputs to the 'replay' folder

 * Usage
  A. For bug reproduction:
     - Record test playthroughs and reproduce bugs at any time
  B. For regression detection:
     - Record before updating Suika2 and replay after the update,
       then you can find regression if the record and replay folders differ

 * TIPS
  - You can check the differecne between record and replay folders using
    suika-compare.exe that is distributed separatedly (difference is a bug)
  - You can rename record folder, and drop it to suika-replay.exe

##
## For macOS
##

suika-capture.dmg ... Capture app for macOS
suika-replay.dmg  ... Replay app for macOS

##
## For Linux
##

suika-linux        ... For Linux (Use with Steam distribution)
