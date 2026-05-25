#!/bin/sh
set -eu
DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
id="${1:-}"
if [ -z "$id" ]; then
  printf '%s\n' '{"ok":false,"error":"invalid rule id"}'
  exit 1
fi
exec python3 "$DIR/fw_lib.py" del "$id"
