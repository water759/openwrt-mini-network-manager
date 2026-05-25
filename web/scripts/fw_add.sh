#!/bin/sh
set -eu
DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
json="${1:-}"
if [ -z "$json" ]; then
  printf '%s\n' '{"ok":false,"error":"missing rule json"}'
  exit 1
fi
exec python3 "$DIR/fw_lib.py" add "$json"
