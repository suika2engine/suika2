===============
Suika2 Variants
===============

Please copy each tool to the folder that contains suika.exe and run it!

##
## Testing Tools
##
suika-capture.exe ... This records game play into record folder
suika-replay.exe  ... This replays record folder and put replay folder

 * Usage
  A. For bug reproduction:
     - Record test plays and reproduce bugs anytime.
  B. For regression detection:
     - Record before the change and replay after the change,
       then you can find regression if record and replay folders differ.

 * TIPS
  - You can check the differecne between record and replay folders using
    suika-compare.exe that is distributed separatedly (difference is a bug)
  - You can rename record folder, and drop it to suika-replay.exe

##
## For Windows
##
suika-64.exe    ... Slightly faster on 64-bit Windows
suika-arm64.exe ... Quite faster on Arm64 Windows (such as Surface Pro)

##
## For Mac
##
suika-capture.dmg ... The capture app for Mac (TODO: replay app for Mab)

##
## For Linux
##
suika-linux        ... For Linux (Steam)
suika-linux-replay ... For automatic testing on GitHub
