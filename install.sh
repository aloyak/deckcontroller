#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_NAME="deck"
EXE_PATH="${SCRIPT_DIR}/deck"
START_DIR="${SCRIPT_DIR}"

# Install vdf if missing
python3 -c "import vdf" 2>/dev/null || pip3 install --quiet vdf

# Close Steam if running
pkill -f steam 2>/dev/null || true
sleep 1

# Find first Steam user ID
STEAM_DIR="${HOME}/.steam/steam/userdata"
[[ -d "$STEAM_DIR" ]] || STEAM_DIR="${HOME}/.local/share/Steam/userdata"
USER_ID="$(ls "$STEAM_DIR" | head -n1)"
SHORTCUTS_FILE="${STEAM_DIR}/${USER_ID}/config/shortcuts.vdf"

# Add shortcut
python3 - <<PYEOF
import vdf, os

shortcuts_file = "$SHORTCUTS_FILE"
os.makedirs(os.path.dirname(shortcuts_file), exist_ok=True)

if os.path.exists(shortcuts_file):
    with open(shortcuts_file, "rb") as f:
        data = vdf.binary_load(f)
else:
    data = vdf.VDFDict({"shortcuts": {}})

shortcuts = data["shortcuts"]
next_index = str(max((int(k) for k in shortcuts if k.isdigit()), default=-1) + 1)

shortcuts[next_index] = {
    "AppName": "$APP_NAME",
    "Exe": '"$EXE_PATH"',
    "StartDir": '"$START_DIR"',
    "icon": "", "ShortcutPath": "", "LaunchOptions": "",
    "IsHidden": 0, "AllowDesktopConfig": 1, "AllowOverlay": 1,
    "OpenVR": 0, "Devkit": 0, "DevkitGameID": "", "LastPlayTime": 0, "tags": {},
}

with open(shortcuts_file, "wb") as f:
    vdf.binary_dump(data, f)
PYEOF

echo "Done. Launch Steam and find '$APP_NAME' under Library → Non-Steam."