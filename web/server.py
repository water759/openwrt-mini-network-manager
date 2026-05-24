import json
import os
from http.server import HTTPServer, BaseHTTPRequestHandler

STATS_PATH = "/tmp/netmon/stats.json"
HOST = "0.0.0.0"
PORT = 8080

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


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/api/traffic" or self.path == "/api/traffic/":
            self.send_json(read_traffic())
            #检测服务状态
        elif self.path == "/api/health":
            self.send_json({"ok": True, "stats_exists": os.path.isfile(STATS_PATH)})
        else:
            self.send_error(404, "Not Found")
    def send_json(self, obj):
        body = json.dumps(obj, ensure_ascii=False).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        # Vue 开发阶段若前后端不同端口，可保留；同域部署可删
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)
    def log_message(self, fmt, *args):
        print("[HTTP]", fmt % args)


def main():
    server = HTTPServer((HOST, PORT), Handler)
    print(f"Listening on http://{HOST}:{PORT}")
    print(f"  GET /api/traffic  <- {STATS_PATH}")
    print(f"  GET /api/health")
    server.serve_forever()


if __name__ == "__main__":
    main()