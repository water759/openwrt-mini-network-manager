#!/usr/bin/env python3
"""Shared firewall rule persistence and iptables helpers."""
import json
import os
import subprocess
import sys

RULES_FILE = os.environ.get("FW_RULES_FILE", "/tmp/netmon/fw_rules.json")
CHAIN_IN = "MININETMON_IN"
CHAIN_FWD = "MININETMON_FWD"

ZONES = {
    "lan": "192.168.0.0/16",
    "wan": "0.0.0.0/0",
    "any": "0.0.0.0/0",
    "device": "127.0.0.1",
}

DIRECTIONS = {"inbound": CHAIN_IN, "forward": CHAIN_FWD}


def chain_for(direction):
    return DIRECTIONS.get(direction, CHAIN_IN)


def resolve_addr(zone, custom=""):
    z = (zone or "any").lower()
    if z == "custom":
        return (custom or "").strip()
    return ZONES.get(z, ZONES["any"])


def ensure_chains():
    for chain in (CHAIN_IN, CHAIN_FWD):
        subprocess.run(["iptables", "-N", chain], capture_output=True)
    for hook, chain in (("INPUT", CHAIN_IN), ("FORWARD", CHAIN_FWD)):
        proc = subprocess.run(
            ["iptables", "-C", hook, "-j", chain],
            capture_output=True,
        )
        if proc.returncode != 0:
            subprocess.run(["iptables", "-I", hook, "1", "-j", chain], check=True)


def load_rules():
    if not os.path.isfile(RULES_FILE):
        return []
    with open(RULES_FILE, "r", encoding="utf-8") as f:
        return json.load(f)


def save_rules(rules):
    os.makedirs(os.path.dirname(RULES_FILE) or ".", exist_ok=True)
    with open(RULES_FILE, "w", encoding="utf-8") as f:
        json.dump(rules, f, ensure_ascii=False, indent=2)


def normalize_rule(rule_in, rule_id=None):
    direction = (rule_in.get("direction") or "inbound").lower()
    if direction not in DIRECTIONS:
        direction = "inbound"

    src_zone = (rule_in.get("src_zone") or "any").lower()
    dst_zone = (rule_in.get("dst_zone") or "any").lower()
    src = resolve_addr(src_zone, rule_in.get("src", ""))
    dst = resolve_addr(dst_zone, rule_in.get("dst", ""))

    proto = (rule_in.get("protocol") or "tcp").lower()
    port = str(rule_in.get("port") or "").strip()
    action = (rule_in.get("action") or "ACCEPT").upper()
    enabled = rule_in.get("enabled", True)
    if isinstance(enabled, str):
        enabled = enabled.lower() in ("1", "true", "yes")

    name = (rule_in.get("name") or "").strip()
    if not name:
        verb = "允许" if action == "ACCEPT" else "禁止"
        port_s = f"-{port}" if port and proto != "icmp" else ""
        name = f"{verb}-{src_zone}-{proto.upper()}{port_s}"

    rule = {
        "id": rule_id,
        "name": name,
        "direction": direction,
        "src_zone": src_zone,
        "dst_zone": dst_zone,
        "src": src,
        "dst": dst,
        "protocol": proto,
        "port": port if port and proto != "icmp" else "-",
        "action": action,
        "enabled": bool(enabled),
    }
    return rule


def iptables_cmd(rule, op="-A"):
    chain = chain_for(rule["direction"])
    proto = rule["protocol"]
    cmd = ["iptables", op, chain, "-p", proto, "-s", rule["src"], "-d", rule["dst"]]
    if proto in ("tcp", "udp") and rule.get("port") not in ("", "-", None):
        cmd += ["--dport", str(rule["port"])]
    cmd += ["-j", rule["action"]]
    return cmd


def apply_rule(rule):
    subprocess.run(iptables_cmd(rule, "-A"), check=True)


def remove_rule(rule):
    cmd = iptables_cmd(rule, "-D")
    proc = subprocess.run(cmd, capture_output=True)
    if proc.returncode != 0:
        subprocess.run(cmd, capture_output=True)


def flush_chains():
    ensure_chains()
    for chain in (CHAIN_IN, CHAIN_FWD):
        subprocess.run(["iptables", "-F", chain], check=True)


def replay_all():
    flush_chains()
    for rule in load_rules():
        if rule.get("enabled", True):
            apply_rule(rule)


def migrate_legacy(rule):
    """Old rules without direction/zones."""
    if "direction" in rule and "src_zone" in rule:
        return rule
    src = rule.get("src", "0.0.0.0/0")
    dst = rule.get("dst", "0.0.0.0/0")

    def infer_zone(addr):
        if addr in ("127.0.0.1", "127.0.0.0/8"):
            return "device"
        if addr.startswith("192.168.") or addr.startswith("10."):
            return "lan"
        if addr in ("0.0.0.0/0", "0.0.0.0"):
            return "wan"
        return "custom"

    def pick_zone(addr):
        z = infer_zone(addr)
        if z == "custom":
            return z, addr
        if resolve_addr(z) != addr:
            return "custom", addr
        return z, addr

    sz, _ = pick_zone(src)
    dz, _ = pick_zone(dst)
    rule.setdefault("direction", "inbound")
    rule["src_zone"] = sz
    rule["dst_zone"] = dz
    rule["src"] = src
    rule["dst"] = dst
    rule.setdefault("name", f"规则-{rule.get('id', '')}")
    rule.setdefault("enabled", True)
    return rule


def find_by_id(rules, rule_id):
    rid = int(rule_id)
    for i, r in enumerate(rules):
        if r.get("id") == rid:
            return i, r
    return None, None


def cmd_init():
    ensure_chains()
    rules = [migrate_legacy(r) for r in load_rules()]
    save_rules(rules)
    replay_all()
    return {"ok": True, "message": "firewall initialized", "rules": rules}


def cmd_list():
    rules = [migrate_legacy(r) for r in load_rules()]
    save_rules(rules)
    return {"ok": True, "rules": rules}


def cmd_add(rule_in):
    ensure_chains()
    rules = [migrate_legacy(r) for r in load_rules()]
    new_id = max((r.get("id", 0) for r in rules), default=0) + 1
    rule = normalize_rule(rule_in, new_id)
    if rule.get("enabled", True):
        apply_rule(rule)
    rules.append(rule)
    save_rules(rules)
    return {"ok": True, "rule": rule, "rules": rules}


def cmd_del(rule_id):
    rules = [migrate_legacy(r) for r in load_rules()]
    idx, rule = find_by_id(rules, rule_id)
    if idx is None:
        return {"ok": False, "error": "rule not found"}
    if rule.get("enabled", True):
        remove_rule(rule)
    rules.pop(idx)
    save_rules(rules)
    return {"ok": True, "rules": rules}


def cmd_update(rule_id, rule_in):
    ensure_chains()
    rules = [migrate_legacy(r) for r in load_rules()]
    idx, old = find_by_id(rules, rule_id)
    if idx is None:
        return {"ok": False, "error": "rule not found"}
    if old.get("enabled", True):
        remove_rule(old)
    rule = normalize_rule(rule_in, int(rule_id))
    if rule.get("enabled", True):
        apply_rule(rule)
    rules[idx] = rule
    save_rules(rules)
    return {"ok": True, "rule": rule, "rules": rules}


def cmd_toggle(rule_id, enabled):
    ensure_chains()
    rules = [migrate_legacy(r) for r in load_rules()]
    idx, rule = find_by_id(rules, rule_id)
    if idx is None:
        return {"ok": False, "error": "rule not found"}
    want = str(enabled).lower() in ("1", "true", "yes")
    if rule.get("enabled") and not want:
        remove_rule(rule)
    elif not rule.get("enabled") and want:
        apply_rule(rule)
    rule["enabled"] = want
    rules[idx] = rule
    save_rules(rules)
    return {"ok": True, "rule": rule, "rules": rules}


def cmd_flush():
    flush_chains()
    save_rules([])
    return {"ok": True, "message": "all rules flushed", "rules": []}


def main():
    if len(sys.argv) < 2:
        print(json.dumps({"ok": False, "error": "missing command"}))
        sys.exit(1)

    cmd = sys.argv[1]
    try:
        if cmd == "init":
            out = cmd_init()
        elif cmd == "list":
            out = cmd_list()
        elif cmd == "add":
            out = cmd_add(json.loads(sys.argv[2]))
        elif cmd == "del":
            out = cmd_del(sys.argv[2])
        elif cmd == "update":
            out = cmd_update(sys.argv[2], json.loads(sys.argv[3]))
        elif cmd == "toggle":
            out = cmd_toggle(sys.argv[2], sys.argv[3])
        elif cmd == "flush":
            out = cmd_flush()
        else:
            out = {"ok": False, "error": f"unknown command: {cmd}"}
            print(json.dumps(out, ensure_ascii=False))
            sys.exit(1)
        print(json.dumps(out, ensure_ascii=False))
    except subprocess.CalledProcessError:
        print(json.dumps({"ok": False, "error": "iptables command failed"}, ensure_ascii=False))
        sys.exit(1)
    except Exception as e:
        print(json.dumps({"ok": False, "error": str(e)}, ensure_ascii=False))
        sys.exit(1)


if __name__ == "__main__":
    main()
