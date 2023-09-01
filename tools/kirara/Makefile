usage:
	@echo "This is the build system for Kirara."
	@echo
	@echo "You can run the following commands:"
	@echo "  make setup  ... install node.js dependencies"
	@echo "  make update ... fetch the latest Suika2 binaries"
	@echo "  make run    ... run for a test"
	@echo "  make win    ... make an installer for Windows"
	@echo "  make mac    ... make an installer for macOS"
	@echo

setup:
	npm install

update:
	cd apps && ./update-suika2.sh && cd ..

run:
	npm start

win:
	npx electron-builder --win --x64

mac:
	npx electron-builder --mac --x64

clean:
	rm -rf dist logs node_modules *~ src/*~
