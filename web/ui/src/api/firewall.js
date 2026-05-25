const API_BASE = import.meta.env.VITE_API_BASE || ''

async function parseJson(res) {
  const data = await res.json()
  if (!res.ok || data.ok === false) {
    throw new Error(data.error || `HTTP ${res.status}`)
  }
  return data
}

export async function fetchFirewallRules() {
  const res = await fetch(`${API_BASE}/api/firewall/rules`)
  return parseJson(res)
}

export async function addFirewallRule(rule) {
  const res = await fetch(`${API_BASE}/api/firewall/rules`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(rule),
  })
  return parseJson(res)
}

export async function updateFirewallRule(id, rule) {
  const res = await fetch(`${API_BASE}/api/firewall/rules/${id}`, {
    method: 'PUT',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(rule),
  })
  return parseJson(res)
}

export async function toggleFirewallRule(id, enabled) {
  const res = await fetch(`${API_BASE}/api/firewall/rules/${id}/toggle`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ enabled }),
  })
  return parseJson(res)
}

export async function deleteFirewallRule(id) {
  const res = await fetch(`${API_BASE}/api/firewall/rules/${id}`, { method: 'DELETE' })
  return parseJson(res)
}

export async function flushFirewallRules() {
  const res = await fetch(`${API_BASE}/api/firewall/flush`, { method: 'POST' })
  return parseJson(res)
}
