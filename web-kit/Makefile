all:
	rm -f suika2-web-kit/.DS_Store
	rm -f suika2-web-kit/game/.DS_Store
	rm -f suika2-web-kit/*~
	cp ../build/emscripten/html/index.html suika2-web-kit/game/
	cp ../build/emscripten/html/index.js suika2-web-kit/game/
	cp ../build/emscripten/html/index.wasm suika2-web-kit/game/
	zip -r suika2-web-kit-XXX.zip suika2-web-kit

clean:
	rm -f suika2-web-kit/game/index.*
	rm -f suika2-web-kit-XXX.zip
