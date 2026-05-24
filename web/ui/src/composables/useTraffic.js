import { ref, computed, onMounted, onUnmounted } from 'vue'
import { fetchTraffic, fetchHealth } from '@/api/traffic'

function formatBytes(n) {
  if (n == null) return '0 B'
  if (n < 1024) return `${n} B`
  if (n < 1024 * 1024) return `${(n / 1024).toFixed(2)} KB`
  if (n < 1024 * 1024 * 1024) return `${(n / 1024 / 1024).toFixed(2)} MB`
  return `${(n / 1024 / 1024 / 1024).toFixed(2)} GB`
}

function formatBps(bps) {
  if (bps == null || bps <= 0) return '0 bps'
  if (bps < 1000) return `${bps.toFixed(0)} bps`
  if (bps < 1_000_000) return `${(bps / 1000).toFixed(2)} Kbps`
  return `${(bps / 1_000_000).toFixed(2)} Mbps`
}

function protoName(p) {
  if (p === 6) return 'TCP'
  if (p === 17) return 'UDP'
  if (p === 1) return 'ICMP'
  return String(p)
}

export function useTraffic(pollMs = 2000) {
  const totals = ref({ rx_bytes: 0, tx_bytes: 0, rx_pkts: 0, tx_pkts: 0 })
  const flows = ref([])
  const loading = ref(false)
  const error = ref(null)
  const lastUpdate = ref(null)
  const online = ref(false)

  const rxBps = ref(0)
  const txBps = ref(0)
  const peakRxBps = ref(0)
  const peakTxBps = ref(0)
  const chartPoints = ref([])

  let timer = null
  let prevRx = 0
  let prevTx = 0
  let prevTs = 0

  async function refresh() {
    loading.value = true
    error.value = null
    try {
      const [data, health] = await Promise.all([fetchTraffic(), fetchHealth()])
      online.value = health.ok && health.stats_exists
      totals.value = data.totals || totals.value
      flows.value = (data.flows || []).map((f) => ({
        ...f,
        total: (f.rx_bytes || 0) + (f.tx_bytes || 0),
        protocolName: protoName(f.protocol),
      }))

      const now = Date.now()
      if (prevTs > 0) {
        const dt = (now - prevTs) / 1000
        if (dt > 0) {
          const rx = ((totals.value.rx_bytes - prevRx) * 8) / dt
          const tx = ((totals.value.tx_bytes - prevTx) * 8) / dt
          rxBps.value = Math.max(0, rx)
          txBps.value = Math.max(0, tx)
          peakRxBps.value = Math.max(peakRxBps.value, rxBps.value)
          peakTxBps.value = Math.max(peakTxBps.value, txBps.value)

          const label = new Date(now).toLocaleTimeString('zh-CN', { hour12: false })
          const pts = [...chartPoints.value, { time: label, rx: rxBps.value, tx: txBps.value }]
          chartPoints.value = pts.slice(-60)
        }
      }
      prevRx = totals.value.rx_bytes
      prevTx = totals.value.tx_bytes
      prevTs = now
      lastUpdate.value = new Date()
    } catch (e) {
      error.value = e.message || '无法连接后端'
      online.value = false
    } finally {
      loading.value = false
    }
  }

  const avgRxBps = computed(() => {
    const pts = chartPoints.value
    if (!pts.length) return 0
    return pts.reduce((s, p) => s + p.rx, 0) / pts.length
  })

  const avgTxBps = computed(() => {
    const pts = chartPoints.value
    if (!pts.length) return 0
    return pts.reduce((s, p) => s + p.tx, 0) / pts.length
  })

  function startPolling(ms) {
    stopPolling()
    refresh()
    timer = setInterval(refresh, ms)
  }

  function stopPolling() {
    if (timer) clearInterval(timer)
  }

  onMounted(() => startPolling(pollMs))
  onUnmounted(stopPolling)

  return {
    totals,
    flows,
    loading,
    error,
    lastUpdate,
    online,
    rxBps,
    txBps,
    peakRxBps,
    peakTxBps,
    chartPoints,
    avgRxBps,
    avgTxBps,
    refresh,
    startPolling,
    stopPolling,
    formatBytes,
    formatBps,
    protoName,
  }
}
