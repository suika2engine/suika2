Automatic Release
=================
This is the official way of a release.

The author usually creates release files on a `MacBook Pro` because:
* He can build Windows binaries on it by using `mingw-w64`
* He needs at least one macOS host to build macOS apps
* He uses `Certum OpenSource Code Signing in the Cloud` product on his macOS host
* He thinks the certificate cannot be used in a CI because its private key is not extractable

## Preparation
* Run `make setup` on the repositoty root

## How to release
* Update `ChangeLog` that includes release notes in order to tell the version information to the release script
* Run `make do-release` on the repository root
```
make do-release
```
* Release files will be built and uploaded automatically
* The Web site will be updated automatically

## How to update Ubuntu PPA
* On Ubuntu, navigate to the repository root and run the following command:
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
