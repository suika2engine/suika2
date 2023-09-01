all: libroot app-suika app-suika-pro app-suika-capture app-suika-replay pack.mac

# This builds 'libroot' directory.
libroot:
	./build-libs.sh

# This builds 'suika.app' (the main game engine).
app-suika: libroot
	./xcbuild-wrap.sh suika

# This builds 'suika-pro.app' (the debugger).
app-suika-pro: libroot
	./xcbuild-wrap.sh suika-pro

# This builds 'suika-capture.app' (the capture app).
app-suika-capture: libroot
	./xcbuild-wrap.sh suika-capture

# This builds 'suika-replay.app' (the replay app).
app-suika-replay: libroot
	./xcbuild-wrap.sh suika-replay

# This is for Kirara internal (command line packager).
pack.mac:
	cd ../../tools/pack && \
	make pack.mac && \
	cp pack.mac ../../build/cloud/ && \
	cd ../../build/macos

# This is just for test build.
noupload: libroot
	./xcbuild-wrap-noupload.sh suika
	./xcbuild-wrap-noupload.sh suika-pro
	./xcbuild-wrap-noupload.sh suika-capture
	./xcbuild-wrap-noupload.sh suika-replay

# This phony target copies dmg files to the project root.
install:
	cp mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg ../../

# This phony target cleans up this directory.
clean:
	rm -rf mac.dmg mac-pro.dmg mac-capture.dmg mac-replay.dmg tmp zip mac.zip
