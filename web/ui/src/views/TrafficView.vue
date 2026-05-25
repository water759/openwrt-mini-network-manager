<template>
  <div class="traffic-page">
    <el-alert v-if="error" type="error" :title="error" show-icon closable class="mb-16" />
    <el-alert v-else-if="!online" type="warning" title="netmon 未运行或 stats.json 不存在" show-icon class="mb-16" />

    <el-row :gutter="16" class="stats-row">
      <el-col :xs="24" :sm="12" :md="8" :lg="4" v-for="card in statCards" :key="card.label">
        <div class="stat-card">
          <div class="icon-wrap" :class="card.color"><el-icon><component :is="card.icon" /></el-icon></div>
          <div>
            <div class="label">{{ card.label }}</div>
            <div class="value">{{ card.value }}</div>
            <div class="sub" v-if="card.sub">{{ card.sub }}</div>
          </div>
        </div>
      </el-col>
    </el-row>

    <div class="chart-panels">
      <div class="page-card panel-card">
        <div class="page-card-title">实时速率趋势 (最近 60 秒)</div>
        <div class="chart-body">
          <RateChart :points="chartPoints" />
        </div>
        <div class="chart-summary">
          <span>平均速率 (近60s): RX <b class="green">{{ formatBps(avgRxBps) }}</b></span>
          <span>TX <b class="blue">{{ formatBps(avgTxBps) }}</b></span>
          <span>峰值: RX <b class="blue">{{ formatBps(peakRxBps) }}</b> TX <b class="orange">{{ formatBps(peakTxBps) }}</b></span>
        </div>
      </div>
      <div class="page-card panel-card flow-card">
        <div class="page-card-title flex-between">
          <span>连接流量列表</span>
          <el-input v-model="search" placeholder="搜索 IP..." size="small" clearable style="width: 160px" :prefix-icon="Search" />
        </div>
        <div class="flow-table-body">
          <FlowTable :flows="filteredFlows" :loading="loading" v-model:page="page" v-model:page-size="pageSize" />
        </div>
      </div>
    </div>

    <el-row :gutter="16" class="status-row">
      <el-col :span="24">
        <StatusTable title="服务状态" :columns="svcCols" :data="svcData" />
      </el-col>
      <el-col :span="24">
        <StatusTable title="活跃主机" :columns="hostCols" :data="hostData" />
      </el-col>
    </el-row>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { Search, Download, Upload, TrendCharts, Timer, Clock } from '@element-plus/icons-vue'
import { useTraffic } from '@/composables/useTraffic'
import RateChart from '@/components/RateChart.vue'
import FlowTable from '@/components/FlowTable.vue'
import StatusTable from '@/components/StatusTable.vue'
import { formatTime } from '@/utils/format'

const {
  totals, flows, loading, error, online, rxBps, txBps,
  peakRxBps, peakTxBps, chartPoints, avgRxBps, avgTxBps, lastUpdate,
  refresh, startPolling, stopPolling, formatBytes, formatBps,
} = useTraffic(2000)

const search = ref('')
const page = ref(1)
const pageSize = ref(10)

const filteredFlows = computed(() => {
  const q = search.value.trim()
  let list = [...flows.value].sort((a, b) => b.total - a.total)
  if (q) {
    list = list.filter((f) =>
      f.src_ip.includes(q) || f.dst_ip.includes(q) ||
      String(f.src_port).includes(q) || String(f.dst_port).includes(q),
    )
  }
  return list
})

const statCards = computed(() => [
  {
    label: '总接收 (RX)',
    value: formatBytes(totals.value.rx_bytes),
    sub: `${totals.value.rx_pkts} 包`,
    icon: Download,
    color: 'green',
  },
  {
    label: '总发送 (TX)',
    value: formatBytes(totals.value.tx_bytes),
    sub: `${totals.value.tx_pkts} 包`,
    icon: Upload,
    color: 'blue',
  },
  {
    label: '实时 RX 速率',
    value: formatBps(rxBps.value),
    sub: `${(rxBps.value / 8 / 1024).toFixed(2)} KB/s`,
    icon: TrendCharts,
    color: 'green',
  },
  {
    label: '实时 TX 速率',
    value: formatBps(txBps.value),
    sub: `${(txBps.value / 8 / 1024).toFixed(2)} KB/s`,
    icon: TrendCharts,
    color: 'blue',
  },
  {
    label: '峰值速率',
    value: formatBps(Math.max(peakRxBps.value, peakTxBps.value)),
    sub: `RX ${formatBps(peakRxBps.value)} / TX ${formatBps(peakTxBps.value)}`,
    icon: Timer,
    color: 'orange',
  },
  {
    label: '最后更新时间',
    value: lastUpdateStr.value,
    sub: '自动刷新',
    icon: Clock,
    color: 'gray',
  },
])

const lastUpdateStr = computed(() => {
  if (!lastUpdate.value) return '--'
  return formatTime(lastUpdate.value)
})

const hostCols = [
  { prop: 'name', label: 'IP', minWidth: 140 },
  { prop: 'status', label: '状态', minWidth: 100 },
  { prop: 'last', label: '最后更新', minWidth: 160 },
  { prop: 'duration', label: '流量', minWidth: 120 },
]

const svcCols = [
  { prop: 'service', label: '服务', minWidth: 100 },
  { prop: 'host', label: '主机', minWidth: 100 },
  { prop: 'status', label: '状态', minWidth: 100 },
  { prop: 'output', label: '输出', minWidth: 200 },
]

const hostData = computed(() => {
  const bytesByIp = new Map()
  for (const f of flows.value) {
    for (const ip of [f.src_ip, f.dst_ip]) {
      if (!ip) continue
      const add = (f.rx_bytes || 0) + (f.tx_bytes || 0)
      bytesByIp.set(ip, (bytesByIp.get(ip) || 0) + add)
    }
  }
  return [...bytesByIp.entries()]
    .sort((a, b) => b[1] - a[1])
    .slice(0, 20)
    .map(([name, bytes]) => ({
      name,
      status: online.value ? 'UP' : 'DOWN',
      last: lastUpdateStr.value,
      duration: formatBytes(bytes),
    }))
})

const svcData = computed(() => [
  {
    service: 'NetMon',
    host: 'local',
    status: online.value ? 'OK' : 'DOWN',
    output: online.value ? 'stats.json 可用' : 'stats.json 不可用',
  },
  {
    service: 'API',
    host: 'local',
    status: error.value ? 'DOWN' : 'OK',
    output: error.value || 'HTTP 后端正常',
  },
])

function onPollChange(e) {
  stopPolling()
  startPolling(e.detail * 1000)
}

function onManualRefresh() {
  refresh()
}

onMounted(() => {
  window.addEventListener('netmon-poll-change', onPollChange)
  window.addEventListener('netmon-refresh', onManualRefresh)
})

onUnmounted(() => {
  window.removeEventListener('netmon-poll-change', onPollChange)
  window.removeEventListener('netmon-refresh', onManualRefresh)
})
</script>

<style scoped>
.traffic-page { min-height: 100%; }
.mb-16 { margin-bottom: 16px; }
.stats-row { margin-bottom: 16px; }
.stats-row .stat-card { margin-bottom: 12px; height: 100%; }
.status-row { margin-top: 0; }
.status-row .el-col { width: 100%; max-width: 100%; }
.status-row .el-col + .el-col { margin-top: 16px; }
.status-row :deep(.status-table) { width: 100%; }
.flex-between { display: flex; justify-content: space-between; align-items: center; }
.chart-panels {
  display: grid;
  grid-template-columns: 1fr 1.4fr;
  gap: 16px;
  align-items: stretch;
  margin-bottom: 16px;
}
@media (max-width: 1200px) {
  .chart-panels {
    grid-template-columns: 1fr;
  }
}
.panel-card {
  height: 100%;
  display: flex;
  flex-direction: column;
  margin-bottom: 0;
  min-height: 280px;
  box-sizing: border-box;
}
.chart-body {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
  min-height: 0;
  width: 100%;
}
.chart-body :deep(.rate-chart) {
  width: 100%;
  height: 100%;
  min-height: 300px;
}
.flow-card {
  display: flex;
  flex-direction: column;
}
.flow-table-body {
  flex: 1;
  width: 100%;
  min-width: 0;
  display: flex;
  flex-direction: column;
}
.flow-card :deep(.el-table) {
  width: 100% !important;
  flex: 1;
}
.flow-card :deep(.pager) {
  width: 100%;
}
.chart-summary {
  display: flex;
  flex-wrap: wrap;
  gap: 16px;
  margin-top: 12px;
  font-size: 13px;
  color: var(--text-secondary);
}
.chart-summary b.green { color: var(--accent-green); }
.chart-summary b.blue { color: var(--accent-blue); }
.chart-summary b.orange { color: var(--accent-orange); }
</style>
