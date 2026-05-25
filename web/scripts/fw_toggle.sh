#!/bin/sh
set -eu
DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
id="${1:-}"
enabled="${2:-}"
if [ -z "$id" ] || [ -z "$enabled" ]; then
  printf '%s\n' '{"ok":false,"error":"missing id or enabled flag"}'
  exit 1
fi
exec python3 "$DIR/fw_lib.py" toggle "$id" "$enabled"
