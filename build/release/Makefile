# Change this line to create your own release file.
SIGNATURE="Developer ID Application: Keiichi TABATA"

all: suika-2.x.x-en.zip suika-2.x.x-jp.zip

suika-2.x.x-en.zip: ../../suika.exe mac.dmg
	rm -rf suika2 suika-2.x.x-en.zip
	mkdir suika2
	mkdir suika2/bg
	cp -Rv ../../game-en/bg/* suika2/bg/
	mkdir suika2/bgm
	cp -Rv ../../game-en/bgm/* suika2/bgm/
	mkdir suika2/cg
	cp -Rv ../../game-en/cg/* suika2/cg/
	mkdir suika2/ch
	cp -Rv ../../game-en/ch/* suika2/ch/
	mkdir suika2/conf
	cp -Rv ../../game-en/conf/* suika2/conf/
	mkdir suika2/cv
	cp -Rv ../../game-en/cv/* suika2/cv/
	mkdir suika2/font
	cp -Rv ../../game-en/font/* suika2/font/
	mkdir suika2/se
	cp -Rv ../../game-en/se/* suika2/se/
	mkdir suika2/tool
	cp -v ../../tool/package-win.exe suika2/tool/
	cp -v ../../tool/package-mac suika2/tool/
	mkdir suika2/txt
	cp -Rv ../../game-en/txt/* suika2/txt/
	cp -v ../../COPYING suika2/
	cp -v ../../readme-en.html suika2/readme.html
	cp -v ../../suika.exe suika2/
	cp -v mac.dmg suika2/
	zip -r suika-2.x.x-en.zip suika2
	rm -rf suika2

suika-2.x.x-jp.zip: ../../suika.exe mac.dmg
	rm -rf suika2 suika-2.x.x-jp.zip
	mkdir suika2
	mkdir suika2/bg
	cp -Rv ../../game-jp/bg/* suika2/bg/
	mkdir suika2/bgm
	cp -Rv ../../game-jp/bgm/* suika2/bgm/
	mkdir suika2/cg
	cp -Rv ../../game-jp/cg/* suika2/cg/
	mkdir suika2/ch
	cp -Rv ../../game-jp/ch/* suika2/ch/
	mkdir suika2/conf
	cp -Rv ../../game-jp/conf/* suika2/conf/
	mkdir suika2/cv
	cp -Rv ../../game-jp/cv/* suika2/cv/
	mkdir suika2/font
	cp -Rv ../../game-jp/font/* suika2/font/
	mkdir suika2/se
	cp -Rv ../../game-jp/se/* suika2/se/
	mkdir suika2/tool
	cp -v ../../tool/package-win.exe suika2/tool/
	cp -v ../../tool/package-mac suika2/tool/
	mkdir suika2/txt
	cp -Rv ../../game-jp/txt/* suika2/txt/
	cp -v ../../COPYING suika2/
	cp -v ../../readme-jp.html suika2/readme.html
	cp -v ../../suika.exe suika2/
	cp -v mac.dmg suika2/
	zip -r suika-2.x.x-jp.zip suika2
	rm -rf suika2

mac.dmg: ../../suika.app
	rm -rf tmp mac.dmg
	mkdir tmp
	cp -Rv ../../suika.app tmp/
	hdiutil create -fs HFS+ -format UDBZ -srcfolder tmp -volname suika-mac mac.dmg
	codesign --sign $(SIGNATURE) mac.dmg
	rm -rf tmp

clean:
	rm -rf suika2 suika-2.x.x-en.zip suika-2.x.x-jp.zip mac.dmg
