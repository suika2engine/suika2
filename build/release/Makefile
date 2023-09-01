all: suika-2.x.x-en.zip suika-2.x.x-jp.zip

suika-2.x.x-en.zip: ../cloud/suika.exe ../cloud/suika-pro.exe ../cloud/suika-capture.exe ../cloud/suika-replay.exe ../cloud/mac.dmg ../cloud/mac-pro.dmg ../cloud/mac-capture.dmg ../cloud/mac-replay.dmg ../cloud/suika-linux ../cloud/suika-linux-replay
	rm -rf suika2 suika-2.x.x-en.zip
	mkdir suika2
	mkdir suika2/anime
	cp -Rv ../../game-en/anime/* suika2/anime/
	mkdir suika2/bg
	cp -Rv ../../game-en/bg/* suika2/bg/
	mkdir suika2/bgm
	cp -Rv ../../game-en/bgm/* suika2/bgm/
	mkdir suika2/cg
	cp -Rv ../../game-en/cg/* suika2/cg/
	mkdir suika2/ch
	cp -Rv ../../game-en/ch/* suika2/ch/
	mkdir suika2/conf
	cp -Rv ../../game-en/conf/*.txt suika2/conf/
	mkdir suika2/cv
	cp -Rv ../../game-en/cv/* suika2/cv/
	mkdir suika2/font
	cp -Rv ../../game-en/font/* suika2/font/
	mkdir suika2/gui
	cp -Rv ../../game-en/gui/*.txt suika2/gui/
	mkdir suika2/rule
	cp -Rv ../../game-en/rule/* suika2/rule/
	mkdir suika2/mov
	mkdir suika2/se
	cp -Rv ../../game-en/se/* suika2/se/
	mkdir suika2/txt
	cp -Rv ../../game-en/txt/*.txt suika2/txt/
	mkdir suika2/wms
	cp -Rv ../../game-en/wms/*.txt suika2/wms/
	cp -v ../../doc/COPYING suika2/
	cp -v readme-en.html suika2/README.html
	mkdir suika2/.vscode
	cp -v ../../tools/snippets/en-normal/plaintext.code-snippets suika2/.vscode/
	cp -v ../cloud/suika.exe suika2/
	cp -v ../cloud/suika-pro.exe suika2/
	cp -v ../cloud/mac.dmg suika2/
	cp -v ../cloud/mac-pro.dmg suika2/
	mkdir suika2/tools
	cp -v ./readme-tools-en.txt suika2/tools/README.TXT
	cp -v ../cloud/suika-capture.exe suika2/tools/
	cp -v ../cloud/suika-replay.exe suika2/tools/
	cp -v ../cloud/suika-64.exe suika2/tools/
	cp -v ../cloud/suika-arm64.exe suika2/tools/
	cp -v ../cloud/mac-capture.dmg suika2/tools/
	cp -v ../cloud/mac-replay.dmg suika2/tools/
	cp -v ../cloud/suika-linux suika2/tools/
	cp -v ../cloud/suika-linux-replay suika2/tools/
	mkdir suika2/tools/web
	cp -v ../emscripten/html/index.html suika2/tools/web/
	cp -v ../emscripten/html/index.js suika2/tools/web/
	cp -v ../emscripten/html/index.wasm suika2/tools/web/
	cp -v ../emscripten/about-en.txt suika2/tools/web/about.txt
	zip -r suika-2.x.x-en.zip suika2
	rm -rf suika2

suika-2.x.x-jp.zip: ../cloud/suika.exe ../cloud/suika-pro.exe ../cloud/suika-capture.exe ../cloud/suika-replay.exe ../cloud/mac.dmg ../cloud/mac-pro.dmg ../cloud/mac-capture.dmg ../cloud/mac-replay.dmg ../cloud/suika-linux ../cloud/suika-linux-replay
	rm -rf suika2 suika-2.x.x-jp.zip
	mkdir suika2
	mkdir suika2/anime
	cp -Rv ../../game-en/anime/* suika2/anime/
	mkdir suika2/bg
	cp -Rv ../../game-jp/bg/* suika2/bg/
	mkdir suika2/bgm
	cp -Rv ../../game-jp/bgm/* suika2/bgm/
	mkdir suika2/cg
	cp -Rv ../../game-jp/cg/* suika2/cg/
	mkdir suika2/ch
	cp -Rv ../../game-jp/ch/* suika2/ch/
	mkdir suika2/conf
	cp -Rv ../../game-jp/conf/*.txt suika2/conf/
	mkdir suika2/cv
	cp -Rv ../../game-jp/cv/* suika2/cv/
	mkdir suika2/font
	cp -Rv ../../game-jp/font/* suika2/font/
	mkdir suika2/gui
	cp -Rv ../../game-jp/gui/*.txt suika2/gui/
	mkdir suika2/rule
	cp -Rv ../../game-jp/rule/* suika2/rule/
	mkdir suika2/mov
	mkdir suika2/se
	cp -Rv ../../game-jp/se/* suika2/se/
	mkdir suika2/txt
	cp -Rv ../../game-jp/txt/*.txt suika2/txt/
	mkdir suika2/wms
	cp -Rv ../../game-jp/wms/*.txt suika2/wms/
	cp -v ../../doc/COPYING suika2/
	cp -v readme-jp.html suika2/README.html
	mkdir suika2/.vscode
	cp -v ../../tools/snippets/jp-normal/plaintext.code-snippets suika2/.vscode/
	cp -v ../cloud/suika.exe suika2/
	cp -v ../cloud/suika-pro.exe suika2/
	cp -v ../cloud/mac.dmg suika2/
	cp -v ../cloud/mac-pro.dmg suika2/
	mkdir suika2/tools
	cp -v ./readme-tools-jp.txt suika2/tools/README.TXT
	cp -v ../cloud/suika-capture.exe suika2/tools/
	cp -v ../cloud/suika-replay.exe suika2/tools/
	cp -v ../cloud/suika-64.exe suika2/tools/
	cp -v ../cloud/suika-arm64.exe suika2/tools/
	cp -v ../cloud/mac-capture.dmg suika2/tools/
	cp -v ../cloud/mac-replay.dmg suika2/tools/
	cp -v ../cloud/suika-linux suika2/tools/
	cp -v ../cloud/suika-linux-replay suika2/tools/
	mkdir suika2/tools/web
	cp -v ../emscripten/html/index.html suika2/tools/web/
	cp -v ../emscripten/html/index.js suika2/tools/web/
	cp -v ../emscripten/html/index.wasm suika2/tools/web/
	cp -v ../emscripten/about-jp.txt suika2/tools/web/about.txt
	zip -r suika-2.x.x-jp.zip suika2
	rm -rf suika2

clean:
	rm -rf suika2 suika-2.x.x-en.zip suika-2.x.x-jp.zip
