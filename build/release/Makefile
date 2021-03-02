all: ../../game/suika.exe ../../game/suika.app
	rm -rf suika2 suika-2.x.x.zip
	mkdir suika2
	cp -Rv ../../game/bg suika2/
	cp -Rv ../../game/bgm suika2/
	cp -Rv ../../game/cg suika2/
	cp -Rv ../../game/ch suika2/
	cp -Rv ../../game/conf suika2/
	cp -Rv ../../game/cv suika2/
	cp -Rv ../../game/font suika2/
	cp -Rv ../../game/se suika2/
	cp -Rv ../../tool suika2/
	cp -Rv ../../game/txt suika2/
	cp -v ../../game/COPYING suika2/
	cp -v ../../game/readme.en.html suika2/
	cp -v ../../game/readme.ja.html suika2/
	cp -v ../../game/suika.exe suika2/
	cp -Rv ../../game/suika.app suika2/
	zip -r suika-2.x.x.zip suika2

clean:
	rm -rf suika2 suika-2.x.x.zip
