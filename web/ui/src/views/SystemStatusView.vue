<template>
  <div>
    <el-alert v-if="error" type="error" :title="error" show-icon closable class="mb-16" />

    <el-row :gutter="16" class="mb-16">
      <el-col :span="6" v-for="s in statusCards" :key="s.label">
        <div class="page-card status-card">
          <el-icon :size="28" :color="s.ok ? '#67c23a' : '#f56c6c'">
            <CircleCheck v-if="s.ok" />
            <CircleClose v-else />
          </el-icon>
          <div>
            <div class="label">{{ s.label }}</div>
            <div class="value" :class="s.ok ? 'ok' : 'bad'">{{ s.value }}</div>
          </div>
        </div>
      </el-col>
    </el-row>

    <el-row v-if="gauges.length" :gutter="16" class="mb-16">
      <el-col :span="8" v-for="g in gauges" :key="g.label">
        <div class="page-card gauge-card">
          <el-progress type="dashboard" :percentage="g.pct" :color="g.color" :width="120" />
          <div class="gauge-label">{{ g.label }}</div>
          <div class="gauge-sub">{{ g.sub }}</div>
        </div>
      </el-col>
    </el-row>

    <div class="page-card">
      <div class="page-card-title">流量连接（来自 /api/traffic）</div>
      <el-table v-loading="loading" :data="flowRows" stripe size="small">
        <el-table-column prop="src_ip" label="源 IP" min-width="110" />
        <el-table-column prop="dst_ip" label="目的 IP" min-width="110" />
        <el-table-column prop="protocolName" label="协议" width="72" />
        <el-table-column prop="rx_bytes" label="接收" width="100">
          <template #default="{ row }">{{ formatBytes(row.rx_bytes) }}</template>
        </el-table-column>
        <el-table-column prop="tx_bytes" label="发送" width="100">
          <template #default="{ row }">{{ formatBytes(row.tx_bytes) }}</template>
        </el-table-column>
        <template #empty>
          <el-empty description="暂无连接数据" :image-size="64" />
        </template>
      </el-table>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { CircleCheck, CircleClose } from '@element-plus/icons-vue'
import { fetchHealth } from '@/api/traffic'
import { fetchFirewallRules } from '@/api/firewall'
import { useTraffic } from '@/composables/useTraffic'

const { flows, loading, error, online, rxBps, txBps, formatBytes, formatBps } = useTraffic(3000)

const apiOk = ref(false)
const firewallOk = ref(false)
const ruleCount = ref(0)

const statusCards = computed(() => [
  { label: 'HTTP 后端', value: apiOk.value ? '正常' : '不可用', ok: apiOk.value },
  { label: 'NetMon', value: online.value ? '运行中' : '未就绪', ok: online.value },
  { label: '防火墙 API', value: firewallOk.value ? '正常' : '不可用', ok: firewallOk.value },
  { label: '防火墙规则', value: String(ruleCount.value), ok: firewallOk.value },
])

const gauges = computed(() => {
  if (!online.value) return []
  const rxPct = Math.min(100, Math.round((rxBps.value / 10_000_000) * 100))
  const txPct = Math.min(100, Math.round((txBps.value / 10_000_000) * 100))
  return [
    { label: 'RX 速率', pct: rxPct, color: '#67c23a', sub: formatBps(rxBps.value) },
    { label: 'TX 速率', pct: txPct, color: '#409eff', sub: formatBps(txBps.value) },
    { label: '活跃连接', pct: Math.min(100, flows.value.length * 5), color: '#e6a23c', sub: `${flows.value.length} 条` },
  ]
})

const flowRows = computed(() =>
  [...flows.value].sort((a, b) => (b.rx_bytes + b.tx_bytes) - (a.rx_bytes + a.tx_bytes)).slice(0, 50),
)

async function loadStatus() {
  try {
    const health = await fetchHealth()
    apiOk.value = !!health.ok
  } catch {
    apiOk.value = false
  }

  try {
    const fw = await fetchFirewallRules()
    firewallOk.value = !!fw.ok
    ruleCount.value = (fw.rules || []).length
  } catch {
    firewallOk.value = false
    ruleCount.value = 0
  }
}

let timer = null

onMounted(() => {
  loadStatus()
  timer = setInterval(loadStatus, 5000)
  window.addEventListener('netmon-refresh', loadStatus)
})

onUnmounted(() => {
  if (timer) clearInterval(timer)
  window.removeEventListener('netmon-refresh', loadStatus)
})
</script>

<style scoped>
.mb-16 { margin-bottom: 16px; }
.status-card { display: flex; align-items: center; gap: 12px; }
.status-card .label { font-size: 13px; color: var(--text-secondary); }
.status-card .value.ok { color: var(--accent-green); font-weight: 600; }
.status-card .value.bad { color: #f56c6c; font-weight: 600; }
.gauge-card { text-align: center; padding: 24px; }
.gauge-label { margin-top: 8px; font-size: 14px; color: var(--text-secondary); }
.gauge-sub { margin-top: 4px; font-size: 12px; color: var(--text-secondary); }
</style>
