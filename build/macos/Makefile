all: libroot app-suika app-suika-pro app-suika-capture app-suika-replay

noupload: libroot
	./xcbuild-wrap-noupload.sh suika
	./xcbuild-wrap-noupload.sh suika-pro
	./xcbuild-wrap-noupload.sh suika-capture
	./xcbuild-wrap-noupload.sh suika-replay

libroot:
	./build-libs.sh

app-suika: libroot
	./xcbuild-wrap.sh suika

app-suika-pro: libroot
	./xcbuild-wrap.sh suika-pro

app-suika-capture: libroot
	./xcbuild-wrap.sh suika-capture

app-suika-replay: libroot
	./xcbuild-wrap.sh suika-replay
