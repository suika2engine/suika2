# Change this line to create your own release file.
SIGNATURE="Developer ID Application: Keiichi TABATA"

suika-2.x.x.zip: ../../game/suika.exe mac.dmg
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
	mkdir suika2/tool
	cp -v ../../tool/package-win.exe suika2/tool/
	cp -v ../../tool/package-mac suika2/tool/
	cp -Rv ../../game/txt suika2/
	cp -v ../../game/COPYING suika2/
	cp -v ../../game/readme.en.html suika2/
	cp -v ../../game/readme.ja.html suika2/
	cp -v ../../game/suika.exe suika2/
	cp -v mac.dmg suika2/
	zip -r suika-2.x.x.zip suika2
	rm -rf suika2

mac.dmg: ../../game/suika.app
	rm -rf tmp mac.dmg
	mkdir tmp
	cp -Rv ../../game/suika.app tmp/
	hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg
	codesign --sign $(SIGNATURE) mac.dmg
	rm -rf tmp

clean:
	rm -rf suika2 suika-2.x.x.zip mac.dmg
