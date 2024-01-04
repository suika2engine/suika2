usage:
	@echo "Type one of the following commands:"
	@echo "  make setup ... setup wine and python"
	@echo "  make bin   ... build a binary"

setup:
	sudo apt-get update
	sudo apt-get install wine wine64 winetricks
	wget https://www.python.org/ftp/python/3.11.5/python-3.11.5-amd64.exe
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 winetricks corefonts win10
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 wine cmd /c python-3.11.5-amd64.exe /quiet PrependPath=1
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 winetricks vcrun2015
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 wine "C:\\users\\`whoami`\\Local Settings\\Application Data\\Programs\\Python\\Python311\\python.exe" -m pip install pyinstaller

bin:
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 wine "C:\\users\\`whoami`\\Local Settings\\Application Data\\Programs\\Python\\Python311\\python.exe" -m PyInstaller --icon ../../build/mingw/res/suika.ico --noconsole --onefile kag2suika.py
	rm -rf kag2suika
	mkdir kag2suika
	cp dist/kag2suika.exe kag2suika/kag2suika.exe
	cp -R readme.txt kag2suika/
	if [ ! -z "`which sign.sh`" ]; then sign.sh kag2suika/kag2suika.exe; fi
	zip -r kag2suika.zip kag2suika

clean:
	rm -rf build dist 8.spec *.zip kag2suika python-3.11.5-amd64.exe
