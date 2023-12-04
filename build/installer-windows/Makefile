all: nsis-3.08.zip
	wine nsis-3.08/makensis.exe /INPUTCHARSET UTF8 install-script.nsi

nsis-3.08.zip:
	curl -O https://suika2.com/dl/nsis-3.08.zip
	unzip nsis-3.08.zip

setup:
	sudo apt-get update
	sudo apt-get install wine wine32 winetricks

clean:
	rm -r .wine32 nsis-3.08.zip nsis-3.08 suika-pro.exe games tools suika2-x.x.exe
