#!/bin/bash

set -eu

# If "clean" is specified
if [ $# = 1 ]; then
	if [ $1 = "clean" ]; then
		echo Cleaning up...
		docker stop suika2-run || true
		docker rm suika2-run || true
		docker image rm suika2-dev || true
		rm -rf suika.exe suika-linux html suika.apk
		exit 0
	fi
fi

# Start
echo Starting builds...

# Build the suika2-dev image
echo Building the Docker image.
docker build -t suika2-dev --add-host archive.ubuntu.com:185.125.190.39 .

# Create an instance of suika2-dev
echo Running the Docker container.
docker run -d -it -v ${PWD%/*/*}:/workspace --name suika2-run suika2-dev

# Build the mingw target
echo Building the Windows binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/mingw && ./build-libs.sh && make"
docker cp suika2-run:/workspace/build/mingw/suika.exe suika.exe

# Build the Linux target
echo Building the Linux binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/linux-x86_64 && ./build-libs.sh && make"
docker cp suika2-run:/workspace/build/linux-x86_64/suika suika-linux

# Build the Web target
echo Building the Web binary.
docker exec -i suika2-run /bin/bash -c "cd /workspace/build/emscripten && source /emsdk/emsdk_env.sh && make"
docker cp suika2-run:/workspace/build/emscripten/html ./

# Build the Android target
echo Building the Android binary.
echo The build will fail a few times, but it is normal.
docker exec -i suika2-run /bin/bash -c "export ANDROID_SDK_ROOT=/cmdline-tools && cd /workspace/build/android && yes | /cmdline-tools/bin/sdkmanager --licenses --sdk_root=/cmdline-tools"
docker exec -i suika2-run /bin/bash -c "export ANDROID_SDK_ROOT=/cmdline-tools && cd /workspace/build/android && gradle clean || true"
docker exec -i suika2-run /bin/bash -c "export ANDROID_SDK_ROOT=/cmdline-tools && cd /workspace/build/android && ./gradlew || true"
docker exec -i suika2-run /bin/bash -c "export ANDROID_SDK_ROOT=/cmdline-tools && cd /workspace/build/android && ./gradlew build --stacktrace || true"
docker exec -i suika2-run /bin/bash -c "export ANDROID_SDK_ROOT=/cmdline-tools && cd /workspace/build/android && ./gradlew build --stacktrace"
docker cp suika2-run:/workspace/build/android/app/build/outputs/apk/debug/app-debug.apk suika.apk

# Trailer
echo Successfully finished the builds.
echo To cleanup, run ./build.sh clean
