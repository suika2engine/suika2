Automatic Release
=================
This is the official way of releases.

The author creates release files on a macOS host because:
* He can build Windows binaries on a macOS host using mingw-w64
* He needs at least one macOS host to build macOS binaries
* He uses `Certum Open Source Code Signing in the Cloud` product on his macOS host
* He thinks the certificate cannot be used in a CI because its private key is not extractable

## Preparation
* He has a `build/.env` file for his credentials:
```
FTP_LOCAL=~/Sites/suika2.com/dl
FTP_USER=his-ftp-account
FTP_PASSWORD=his-ftp-password
FTP_URL=ftp://suika2.com/dl
```
* He has a `SIGNATURE` variable in `build/macos/Makefile` to sign dmg files

## How to release a main ZIP
* Update the changelog in readme files (this tells the release script a new version number):
  * `doc/readme-jp.html`
  * `doc/readme-en.html`
* On terminal, navigate to the repository root and run the following command:
```
make do-release
```
* A release zip file will be built automatically
* The zip file will be uploaded to `suika2.com/dl/`
* The HTML pages on `suika2.com` will be updated automatically

## How to update template games
* On WSL2, navigate to the repository root and run the following command:
```
make update-templates
```
* Template game files will be uploaded to the Web server automatically

## How to update Ubuntu PPA
* On WSL2, navigate to the repository root and run the following command:
```
cd build/ppa
./create.sh
```
* Note that the author has a GPG key registered to launchpad site
* Note that the author has `~/.dput.cf` file:
```
[suika2]
fqdn = ppa.launchpad.net
method = ftp
incoming = ~ktabata/ubuntu/
login = anonymous
allow_unsigned_uploads = 0
```
