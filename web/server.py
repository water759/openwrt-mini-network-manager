import ipaddress
import json
import os
import subprocess
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse

STATS_PATH = "/tmp/netmon/stats.json"
HOST = "0.0.0.0"
PORT = 8080
SCRIPTS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "scripts")
PROTOCOLS = frozenset({"tcp", "udp", "icmp"})
ACTIONS = frozenset({"ACCEPT", "DROP"})
DIRECTIONS = frozenset({"inbound", "forward"})
ZONES = frozenset({"lan", "wan", "device", "any", "custom"})
def read_traffic():
    #读取 stats.json；文件不存在或解析失败时返回空结构
    empty = {
        "totals": {
            "rx_bytes": 0,
            "tx_bytes": 0,
            "rx_pkts": 0,
            "tx_pkts": 0,
        },
        "flows": [],
    }
    if not os.path.isfile(STATS_PATH):
        return empty
    try:
        with open(STATS_PATH, "r", encoding="utf-8") as f:
            return json.load(f)
    except (json.JSONDecodeError, OSError):
        return empty

def script_path(name):
    return os.path.join(SCRIPTS_DIR, name)


def run_script(name, *args, timeout=15):
    path = script_path(name)
    if not os.path.isfile(path):
        return {"ok": False, "error": f"script not found: {name}"}
    try:
        proc = subprocess.run(
            [path, *args],
            capture_output=True,
            text=True,
            timeout=timeout,
            check=False,
        )
    except subprocess.TimeoutExpired:
        return {"ok": False, "error": f"script timeout: {name}"}
    except OSError as e:
        return {"ok": False, "error": str(e)}

    out = (proc.stdout or "").strip()
    if not out:
        err = (proc.stderr or "").strip() or f"exit {proc.returncode}"
        return {"ok": False, "error": err}

    try:
        data = json.loads(out.splitlines()[-1])
    except json.JSONDecodeError:
        return {"ok": False, "error": f"invalid json from {name}", "raw": out}

    if proc.returncode != 0 and data.get("ok") is not False:
        data["ok"] = False
        data.setdefault("error", f"script exit {proc.returncode}")
    return data


def valid_ip_or_cidr(s):
    if s is None:
        return False
    text = str(s).strip()
    if not text:
        return False
    try:
        ipaddress.ip_network(text, strict=False)
        return True
    except ValueError:
        return False


def validate_rule(body, partial=False):
    if not isinstance(body, dict):
        return "body must be a JSON object"

    proto = str(body.get("protocol", "tcp")).lower()
    action = str(body.get("action", "ACCEPT")).upper()
    direction = str(body.get("direction", "inbound")).lower()
    src = "" if body.get("src") is None else str(body.get("src", "")).strip()
    dst = "" if body.get("dst") is None else str(body.get("dst", "")).strip()
    port = str(body.get("port", "")).strip()
    name = str(body.get("name", "")).strip()
    enabled = body.get("enabled", True)

    raw_src_zone = body.get("src_zone")
    raw_dst_zone = body.get("dst_zone")
    if raw_src_zone in (None, ""):
        src_zone = "custom" if src and valid_ip_or_cidr(src) else "any"
    else:
        src_zone = str(raw_src_zone).lower()
    if raw_dst_zone in (None, ""):
        dst_zone = "custom" if dst and valid_ip_or_cidr(dst) else "any"
    else:
        dst_zone = str(raw_dst_zone).lower()

    if proto not in PROTOCOLS:
        return "invalid protocol"
    if action not in ACTIONS:
        return "invalid action"
    if direction not in DIRECTIONS:
        return "invalid direction"
    if src_zone not in ZONES or dst_zone not in ZONES:
        return "invalid zone"

    if src_zone == "custom":
        if not src:
            return "custom src zone requires src address (e.g. 192.168.1.0/24)"
        if not valid_ip_or_cidr(src):
            return "invalid src (use IPv4 or CIDR, e.g. 192.168.1.0/24)"
    else:
        src = ""

    if dst_zone == "custom":
        if not dst:
            return "custom dst zone requires dst address"
        if not valid_ip_or_cidr(dst):
            return "invalid dst (use IPv4 or CIDR)"
    else:
        dst = ""

    if proto == "icmp":
        port = ""
    elif not partial and (not port.isdigit() or not (1 <= int(port) <= 65535)):
        return "invalid port"

    zone_defaults = {
        "lan": "192.168.0.0/16",
        "wan": "0.0.0.0/0",
        "any": "0.0.0.0/0",
        "device": "127.0.0.1",
    }
    resolved_src = src if src_zone == "custom" else zone_defaults[src_zone]
    resolved_dst = dst if dst_zone == "custom" else zone_defaults[dst_zone]

    if action == "DROP" and resolved_src in ("0.0.0.0/0", "0.0.0.0") and resolved_dst in (
        "0.0.0.0/0",
        "0.0.0.0",
    ):
        return "refused: drop all traffic"

    return {
        "name": name,
        "direction": direction,
        "src_zone": src_zone,
        "dst_zone": dst_zone,
        "src": src,
        "dst": dst,
        "protocol": proto,
        "port": port,
        "action": action,
        "enabled": bool(enabled) if not isinstance(enabled, str) else enabled.lower() in ("1", "true", "yes"),
    }

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        path = urlparse(self.path).path.rstrip("/") or "/"

        if path == "/api/traffic":
            self.send_json(read_traffic())
        elif path == "/api/health":
            self.send_json({"ok": True, "stats_exists": os.path.isfile(STATS_PATH)})
        elif path == "/api/firewall/rules":
            self.send_json(run_script("fw_list.sh"))
        else:
            self.send_error(404, "Not Found")
    def do_OPTIONS(self):
        self.send_response(204)
        self._cors_headers()
        self.end_headers()

    def do_PUT(self):
        path = urlparse(self.path).path.rstrip("/") or "/"
        prefix = "/api/firewall/rules/"

        if path.startswith(prefix):
            rule_id = path[len(prefix):]
            if not rule_id.isdigit():
                self.send_json({"ok": False, "error": "invalid rule id"}, status=400)
                return
            body = self._read_json_body()
            validated = validate_rule(body)
            if isinstance(validated, str):
                self.send_json({"ok": False, "error": validated}, status=400)
                return
            payload = json.dumps(validated, ensure_ascii=False)
            self.send_json(run_script("fw_update.sh", rule_id, payload))
        else:
            self.send_error(404, "Not Found")

    def do_POST(self):
        path = urlparse(self.path).path.rstrip("/") or "/"
        body = self._read_json_body()

        if path == "/api/firewall/rules":
            validated = validate_rule(body)
            if isinstance(validated, str):
                self.send_json({"ok": False, "error": validated}, status=400)
                return
            payload = json.dumps(validated, ensure_ascii=False)
            self.send_json(run_script("fw_add.sh", payload))
        elif path == "/api/firewall/flush":
            self.send_json(run_script("fw_flush.sh"))
        else:
            prefix = "/api/firewall/rules/"
            if path.startswith(prefix) and path.endswith("/toggle"):
                rule_id = path[len(prefix): -len("/toggle")]
                if not rule_id.isdigit():
                    self.send_json({"ok": False, "error": "invalid rule id"}, status=400)
                    return
                enabled = body.get("enabled", True)
                self.send_json(
                    run_script("fw_toggle.sh", rule_id, "true" if enabled else "false")
                )
            else:
                self.send_error(404, "Not Found")

    def do_DELETE(self):
        path = urlparse(self.path).path.rstrip("/") or "/"
        prefix = "/api/firewall/rules/"

        if path.startswith(prefix):
            rule_id = path[len(prefix):]
            if not rule_id.isdigit():
                self.send_json({"ok": False, "error": "invalid rule id"}, status=400)
                return
            self.send_json(run_script("fw_del.sh", rule_id))
        else:
            self.send_error(404, "Not Found")

    def _read_json_body(self):
        length = int(self.headers.get("Content-Length", 0))
        if length <= 0:
            return {}
        try:
            return json.loads(self.rfile.read(length).decode("utf-8"))
        except (json.JSONDecodeError, UnicodeDecodeError):
            return {}

    def send_json(self, obj, status=200):
        body = json.dumps(obj, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self._cors_headers()
        self.end_headers()
        self.wfile.write(body)

    def _cors_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")

    def log_message(self, fmt, *args):
        print("[HTTP]", fmt % args)


def main():
    init = run_script("fw_init.sh")
    if not init.get("ok"):
        print("[WARN] fw_init:", init.get("error", init))
    server = HTTPServer((HOST, PORT), Handler)
    print(f"Listening on http://{HOST}:{PORT}")
    print(f"  GET /api/traffic  <- {STATS_PATH}")
    print(f"  GET /api/health")
    print(f"  GET    /api/firewall/rules")
    print(f"  POST   /api/firewall/rules")
    print(f"  PUT    /api/firewall/rules/{{id}}")
    print(f"  POST   /api/firewall/rules/{{id}}/toggle")
    print(f"  DELETE /api/firewall/rules/{{id}}")
    print(f"  POST   /api/firewall/flush")
    server.serve_forever()


if __name__ == "__main__":
    main()