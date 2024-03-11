#!/bin/sh

set -eu

echo "Enter the version to publish (e.g.: 16.50):"
read VERSION

echo "Press enter to continue to publish Suika2/$VERSION."
read str

# Update the Web site.
echo "Updating the Web site."
SAVE_DIR=`pwd`
cd ../web && \
    ./update-templates.sh && \
    ./update-version-stable.sh "$VERSION" && \
    ftp-upload.sh dl/index.html && \
    ftp-upload.sh en/dl/index.html && \
    git add -u dl/index.html en/dl/index.html &&
	git commit -m "web: release stable $VERSION"
cd "$SAVE_DIR"

# Upload to GitHub.
yes "" | gh release create "v2.$VERSION" --title "v2.$VERSION" ~/Sites/suika2.com/dl/suika2-$VERSION.exe ~/Sites/suika2.com/dl/suika2-$VERSION.dmg
