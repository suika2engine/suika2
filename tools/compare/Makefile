usage:
	@echo "Type one of the following commands:"
	@echo "  make setup ... setup wine and python"
	@echo "  make bin   ... build a binary"

setup:
	#sudo apt-get update
	#sudo apt-get install wine wine64 winetricks
	#wget https://www.python.org/ftp/python/3.11.5/python-3.11.5-amd64.exe
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 winetricks corefonts win10
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 wine cmd /c python-3.11.5-amd64.exe /quiet PrependPath=1
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 winetricks vcrun2015
	WINEPREFIX=`pwd`/.wine64 WINARCH=win64 wine "C:\\users\\`whoami`\\Local Settings\\Application Data\\Programs\\Python\\Python311\\python.exe" -m pip install opencv-python numpy pyinstaller

bin:
	WINEPREFIX=`pwd`/.wine64 WINARCH=win32 wine "C:\\users\\`whoami`\\Local Settings\\Application Data\\Programs\\Python\\Python311\\python.exe" -m PyInstaller --onefile compare.py
	rm -rf suika-compare
	mkdir suika-compare
	cp dist/compare.exe suika-compare/suika-compare.exe
	cp readme-jp.txt readme-en.txt suika-compare/
	if [ ! -z "`which sign.sh`" ]; then sign.sh suika-compare/suika-compare.exe; fi
	zip -r suika-compare.zip suika-compare

clean:
	rm -rf build dist compare.spec suika-compare.zip suika-compare python-3.11.5-amd64.exe
