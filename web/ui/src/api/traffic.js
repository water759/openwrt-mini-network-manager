const API_BASE = import.meta.env.VITE_API_BASE || ''

export async function fetchTraffic() {
  const res = await fetch(`${API_BASE}/api/traffic`)
  if (!res.ok) throw new Error(`HTTP ${res.status}`)
  return res.json()
}

export async function fetchHealth() {
  const res = await fetch(`${API_BASE}/api/health`)
  if (!res.ok) throw new Error(`HTTP ${res.status}`)
  return res.json()
}
