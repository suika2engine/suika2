all: suika2-web-kit-2.x.x-en.zip suika2-web-kit-2.x.x-jp.zip

suika2-web-kit-2.x.x-en.zip:
	rm -rf suika2-web-kit suika2-web-kit-2.x.x-en.zip
	mkdir suika2-web-kit
	mkdir suika2-web-kit/game
	cp data01-en.arc suika2-web-kit/game/data01.arc
	cp ../emscripten/html/index.html suika2-web-kit/game/index.html
	cp ../emscripten/html/index.js suika2-web-kit/game/index.js
	cp ../emscripten/html/index.wasm suika2-web-kit/game/index.wasm
	cp readme-en.html suika2-web-kit/readme.html
	zip -r suika2-web-kit-2.x.x-en.zip suika2-web-kit
	rm -rf suika2-web-kit

suika2-web-kit-2.x.x-jp.zip:
	rm -rf suika2-web-kit suika2-web-kit-2.x.x-jp.zip
	mkdir suika2-web-kit
	mkdir suika2-web-kit/game
	cp data01-jp.arc suika2-web-kit/game/data01.arc
	cp ../emscripten/html/index.html suika2-web-kit/game/index.html
	cp ../emscripten/html/index.js suika2-web-kit/game/index.js
	cp ../emscripten/html/index.wasm suika2-web-kit/game/index.wasm
	cp readme-jp.html suika2-web-kit/readme.html
	zip -r suika2-web-kit-2.x.x-jp.zip suika2-web-kit
	rm -rf suika2-web-kit

clean:
	rm -rf suika2-web-kit
	rm -f suika2-web-kit-2.x.x-en.zip suika2-web-kit-2.x.x-jp.zip
