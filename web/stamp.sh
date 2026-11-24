#!/bin/sh
# Writes a content hash of the game into index.html (const BUILD), so the URLs of
# build/doom.js and build/doom.wasm change whenever the game does.
cd "$(dirname "$0")" || exit 1
h=$(cat build/doom.js build/doom.wasm | sha1sum | cut -c1-8)
sed -i "s/const BUILD = '[^']*'/const BUILD = '$h'/" index.html
echo "build id $h"
