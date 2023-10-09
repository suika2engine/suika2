Automatic Release
=================
This is the official way of releases.

The author uses a Windows/WSL2 host and a macOS host because:
* He preferred Windows to sign exe files
* He uses `Certum Open Source Code Signing in the Cloud` product
* He thinks the product cannot be used in a CI because its private key is not extractable

## Preparation
* On the WSL2 host:
  * He has a `build/.env` file for his credentials:
  ```
  MACOS_HOST=his-mac-host-name.local
  MACOS_USER=his-mac-user-name
  FTP_LOCAL=~/Sites/suika2.com/dl
  FTP_USER=his-ftp-account
  FTP_PASSWORD=his-ftp-password
  FTP_URL=ftp://suika2.com/dl
  ```
* On the macOS host:
  * He has a public key on WSL2 in Mac's `~/.ssh/authorized_keys`
  * He has a ssh server turned on
  * He has a `build/macos/.passwd` file and its content is his login password for keychain access to sign codes
  * He has a `SIGNATURE` variable in `build/macos/Makefile` to sign dmg files

## IMPORTANT NOTICE
Remember, **open the lid of a MacBook Pro with Apple Silicon** during builds.
Otherwise, sign and notarization via ssh will frequently fail.

## How to release a main ZIP
* Update the changelog in readme on WSL2 (this tells the release script a new version number):
  * `doc/readme-jp.html`
  * `doc/readme-en.html`
* On WSL2, navigate to the repository root and run the following command:
```
make do-release
```
* A release file and a helper zip will be built automatically
* They will be uploaded to `suika2.com/dl/`
* The HTML pages on `suika2.com` will be updated automatically using `release-html.sh` script

## How to release Kirara
* On WSL2, navigate to the repository root and run the following command:
```
make do-release-kirara
```
* Release file for Windows and macOS will be built automatically
* They will be uploaded to `suika2.com/dl/`
* The HTML pages on `suika2.com` will **NOT** be updated automatically

## How to update template games
* On WSL2, navigate to the repository root and run the following command:
```
make do-release-templates
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
