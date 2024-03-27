#!/bin/sh

# Stop when failed.
set -eu

#
# Show a welcome message.
#
echo ""
echo "Hello, this is the Suika2 Release Manager."

#
# Check if we run on a macOS host.
#
if [ -z "`uname -a | grep Darwin`" ]; then
    echo "Error: please run on macOS.";
    exit 1;
fi

#
# Check for GNU coreutils.
#
SED='sed'
if [ ! -z "`which gsed`" ]; then
    SED='gsed';
fi
HEAD='head'
if [ ! -z "`which ghead`" ]; then
    HEAD='ghead';
fi

#
# Guess the release version number.
#
VERSION=`grep -a1 '<!-- BEGIN-LATEST-JP -->' ../ChangeLog | tail -n1`
VERSION=`echo $VERSION | cut -d '>' -f 2 | cut -d ' ' -f 1`
VERSION=`echo $VERSION | cut -d '/' -f 2`

#
# Get the release notes.
#
NOTE_JP=`cat ../ChangeLog | awk '/BEGIN-LATEST-JP/,/END-LATEST-JP/' | tail -n +2 | $HEAD -n -1`
NOTE_EN=`cat ../ChangeLog | awk '/BEGIN-LATEST-EN/,/END-LATEST-EN/' | tail -n +2 | $HEAD -n -1`

#
# Do an interactive confirmation.
#
echo ""
echo "Are you sure you want to release version $VERSION?"
echo ""
echo "[Japanese Note]"
echo "$NOTE_JP"
echo ""
echo "[English Note]"
echo "$NOTE_EN"
echo ""
echo "(press enter to proceed)"
read str

#
# Push a tag to GitHub in order to check the version number is available.
#
git tag -a "v2.$VERSION" -m "release"
git push github "v2.$VERSION"

discord-post.sh "自動リリースシステムが起動しました。Suika2/${VERSION}をビルドしています。アップロード完了まで推定で10分です。"

#
# Build "suika.exe".
#
echo ""
echo "Building suika.exe"
say "Windows用のエンジンをビルドしています" &
discord-post.sh "Windows用のエンジンをビルドしています..."
cd engine-windows
rm -f *.o
if [ ! -e libroot ]; then
    ./build-libs.sh;
fi
make -j16
sign.sh suika.exe
cd ..

#
# Build the "Suika.app".
#
echo ""
echo "Building Suika.app (suika-mac.dmg)."
say "Mac用のエンジンをビルドしています" &
discord-post.sh "Mac用のエンジンをビルドしています..."
cd engine-macos
rm -f suika-mac.dmg suika-mac-nosign.dmg
make suika-mac.dmg
cp suika-mac.dmg suika-mac-nosign.dmg
codesign --sign 'Developer ID Application: Keiichi TABATA' suika-mac.dmg
cd ..

#
# Build the Wasm files.
#
echo ""
echo "Building Wasm files."
say "Web用のエンジンをビルドしています" &
discord-post.sh "Web用のエンジンをビルドしています..."
cd engine-wasm
make clean
make
cd ..

#
# Build the iOS source tree.
#
echo ""
echo "Building iOS source tree."
say "iOS用のソースコードを作成しています" &
cd engine-ios
make src > /dev/null
cd ..

#
# Build the Android source tree.
#
echo ""
echo "Building Android source tree."
say "Android用のソースコードを作成しています" &
cd engine-android
make debug > /dev/null
make src > /dev/null
cd ..
cd pro-android
make
cd ..

#
# Build "suika-pro.exe".
#
echo ""
echo "Building suika-pro.exe"
say "Windows用の開発ツールをビルドしています" &
discord-post.sh "Windows用の開発ツールをビルドしています..."
cd pro-windows
rm -f *.o
if [ ! -e libroot ]; then
    cp -Rav ../engine-windows/libroot .;
fi
make -j16 VERSION="$VERSION"
sign.sh suika-pro.exe
cd ..

#
# Build "web-test.exe"
#
echo ""
echo "Building web-test.exe"
say "Windows用のWebテストツールをビルドしています" &
cd ../tools/web-test
make
cd ../../build

#
# Make an installer for Windows.
#
echo ""
echo "Creating an installer for Windows."
say "Windows用のインストーラをビルドしています" &
discord-post.sh "Windows用のインストーラをビルドしています..."

# /
cp -v pro-windows/suika-pro.exe installer-windows/suika-pro.exe

# /games
rm -rf installer-windows/games
find ../games -name '.*' | xargs rm
mkdir installer-windows/games
cp -R ../games/dark installer-windows/games/
cp -R ../games/japanese installer-windows/games/
cp -R ../games/english installer-windows/games/
cp -R ../games/nvl installer-windows/games/
cp -R ../games/nvl-tategaki installer-windows/games/
cp -R ../games/nvl-en installer-windows/games/

# /tools
rm -rf installer-windows/tools
mkdir -p installer-windows/tools
cp  engine-windows/suika.exe installer-windows/tools/
cp engine-macos/suika-mac.dmg installer-windows/tools/
cp -R engine-android/android-src installer-windows/tools/android-src
cp -R engine-ios/ios-src installer-windows/tools/ios-src
mkdir -p installer-windows/tools/web
cp engine-wasm/html/index.html installer-windows/tools/web/index.html
cp engine-wasm/html/index.js installer-windows/tools/web/index.js
cp engine-wasm/html/index.wasm installer-windows/tools/web/index.wasm
cp  ../tools/web-test/web-test.exe installer-windows/tools/web-test.exe
cp -R ../tools/installer installer-windows/tools/installer
cp ../tools/snippets/jp-normal/plaintext.code-snippets installer-windows/plaintext.code-snippets.jp
cp ../tools/snippets/en-normal/plaintext.code-snippets installer-windows/plaintext.code-snippets.en

# Make an installer
cd installer-windows
make
sign.sh suika2-installer.exe

# Also, make a zip
#make zip

cd ..

#
# Build "Suika2 Pro.app".
#
echo ""
echo "Building Suika2 Pro.app (suika2.dmg)"
say "Mac用の開発ツールをビルドしています"
discord-post.sh "Mac用の開発ツールをビルドしています..."
cd pro-macos
rm -f suika2.dmg
make
cd ..

#
# Upload.
#
echo ""
echo "Uploading files."
say "Webサーバにアップロード中です"
discord-post.sh "Webサーバに実行ファイルをアップロード中です..."

ftp-upload.sh installer-windows/suika2-installer.exe "dl/suika2-$VERSION.exe"
ftp-upload.sh pro-macos/suika2.dmg "dl/suika2-$VERSION.dmg"
#ftp-upload.sh installer-windows/suika2.7z "dl/suika2-$VERSION.7z"
echo "Upload completed."

#
# Update the Web site.
#
echo ""
echo "Updating the Web site."
say "Webページを更新中です"
discord-post.sh "Webページを更新中です..."
SAVE_DIR=`pwd`
cd ../web && \
    ./update-templates.sh && \
    ./update-version-latest.sh && \
    ftp-upload.sh dl/index.html && \
    ftp-upload.sh en/dl/index.html && \
    git add -u dl/index.html en/dl/index.html && \
    git commit -m "web: release $VERSION"
cd "$SAVE_DIR"

#
# Restore a non-signed dmg for a store release.
#
mv engine-macos/suika-mac-nosign.dmg engine-macos/suika-mac.dmg

#
# Post to the Discord server.
#
echo ""
echo "Posting to the Discord server."
say "Discordサーバにポストします" &
discord-release-bot.sh

#
# Make a release on GitHub.
#
echo ""
echo "Making a release on GitHub."
say "GitHubでリリースを作成中です"
yes "" | gh release create "v2.$VERSION" --title "v2.$VERSION" ~/Sites/suika2.com/dl/suika2-$VERSION.exe ~/Sites/suika2.com/dl/suika2-$VERSION.dmg

#
# Finish.
#
echo ""
echo "Finished. $VERSION was released!"
say "リリースが完了しました" &
