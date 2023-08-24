@echo off

REM Start
echo Starting builds...

REM Build the suika2-dev image
echo Building the Docker image.
docker build -t suika2-dev --add-host archive.ubuntu.com:185.125.190.39 .

REM Create an instance of suika2-dev
echo Running the Docker container.
docker run -d -it -v %~dp0\..\..:/workspace --name suika2-run suika2-dev

REM Build the mingw target
echo Building the Windows binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/mingw && make"
docker cp suika2-run:/workspace/build/mingw/suika.exe suika.exe

REM Build the mingw-pro target
echo Building the Windows Pro binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/mingw-pro && make"
docker cp suika2-run:/workspace/build/mingw-pro/suika-pro.exe suika-pro.exe

REM Build the mingw-64 target
echo Building the Windows 64bit binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/mingw-64 && make"
docker cp suika2-run:/workspace/build/mingw-pro/suika-64.exe suika-64.exe

REM Build the mingw-capture target
echo Building the Windows Capture binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/mingw-capture && make"
docker cp suika2-run:/workspace/build/mingw-capture/suika-capture.exe suika-capture.exe

REM Build the mingw-replay target
echo Building the Windows Replay binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/mingw-replay && make"
docker cp suika2-run:/workspace/build/mingw-capture/suika-replay.exe suika-replay.exe

REM Build the mingw-arm64 target
echo Building the Windows Arm64 binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/mingw-arm64 && make"
docker cp suika2-run:/workspace/build/mingw-capture/suika-arm64.exe suika-arm64.exe

REM Build the linux-x86_64 target
echo Building the linux-x86_64 binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/linux-x86_64 && make"
docker cp suika2-run:/workspace/build/linux-x86_64/suika suika-linux

REM Build the linux-x86_64-replay target
echo Building the linux-x86_64-replay binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/linux-x86_64-replay && make"
docker cp suika2-run:/workspace/build/linux-x86_64-replay/suika-replay suika-linux-replay

REM Build the Web target
echo Building the Web binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/emscripten && source /emsdk/emsdk_env.sh && make"
docker cp suika2-run:/workspace/build/emscripten/html ./

REM Build the Android target
echo Building the Android binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/android && ./build-on-linux.sh"
docker cp suika2-run:/workspace/build/android/app/build/outputs/apk/debug/app-debug.apk suika.apk

REM Stop the suika2-run instance
echo Stopping the docker container.
docker stop suika2-run
docker rm suika2-run

REM Trailer
echo Successfully finished the builds.
pause
