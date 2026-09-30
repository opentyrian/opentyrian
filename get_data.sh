#!/bin/bash
# get_data.sh — fetch the freeware Tyrian 2.1 game data into a directory.
# Idempotent: does nothing if the data is already there.
#
#   ./get_data.sh [dir]     (default: ./data)
#
# Tyrian 2.1 was released as freeware by its author; the zip is fetched from
# the mirror the OpenTyrian README points at.  Filenames are stored in
# lowercase, which is what the engine opens.
set -euo pipefail

DEST="${1:-$(dirname "$0")/data}"
URL="https://camanis.net/tyrian/tyrian21.zip"

have_data() { find "$1" -maxdepth 1 -iname "tyrian1.lvl" 2>/dev/null | grep -q .; }

mkdir -p "$DEST"
if have_data "$DEST"; then
    echo "OK: game data already in $DEST"
    exit 0
fi

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

echo "Downloading Tyrian 2.1 (freeware, ~4 MB) from $URL ..."
curl -fL --progress-bar -o "$TMP/tyrian21.zip" "$URL"
unzip -q -o "$TMP/tyrian21.zip" -d "$TMP/x"

# The archive may nest its files in a folder and name them in UPPERCASE.
# Find the directory holding the level data and flatten it into DEST.
probe=$(find "$TMP/x" -iname "tyrian1.lvl" | head -1)
[ -n "$probe" ] || { echo "ERROR: download did not contain the expected game data" >&2; exit 1; }
src=$(dirname "$probe")

find "$src" -maxdepth 1 -type f | while read -r f; do
    name=$(basename "$f" | tr '[:upper:]' '[:lower:]')
    cp "$f" "$DEST/$name"
done

have_data "$DEST" || { echo "ERROR: game data copy failed" >&2; exit 1; }
echo "OK: game data in $DEST"
