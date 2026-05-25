#!/bin/sh
set -eu
DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
id="${1:-}"
json="${2:-}"
if [ -z "$id" ] || [ -z "$json" ]; then
  printf '%s\n' '{"ok":false,"error":"missing id or rule json"}'
  exit 1
fi
exec python3 "$DIR/fw_lib.py" update "$id" "$json"
