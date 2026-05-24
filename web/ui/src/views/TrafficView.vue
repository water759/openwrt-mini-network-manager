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
        <FlowTable :flows="filteredFlows" :loading="loading" v-model:page="page" v-model:page-size="pageSize" />
      </div>
    </div>

    <el-row :gutter="16" class="status-row">
      <el-col :xs="24" :md="8">
        <StatusTable title="主机状态" :columns="hostCols" :data="hostData" more-link="#" />
      </el-col>
      <el-col :xs="24" :md="8">
        <StatusTable title="服务状态" :columns="svcCols" :data="svcData" more-link="#" />
      </el-col>
      <el-col :xs="24" :md="8">
        <StatusTable title="最近告警" :columns="alarmCols" :data="alarmData" more-link="#" />
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
  { prop: 'name', label: '主机名', width: 100 },
  { prop: 'status', label: '状态', width: 90 },
  { prop: 'last', label: '最后检查', width: 100 },
  { prop: 'duration', label: '持续时间' },
]
const hostData = [
  { name: 'openwrt', status: 'UP', last: '15:30:40', duration: '2d 14h' },
  { name: 'gateway', status: 'UP', last: '15:30:38', duration: '2d 14h' },
  { name: 'dns-server', status: 'UP', last: '15:30:35', duration: '2d 14h' },
]

const svcCols = [
  { prop: 'service', label: '服务', width: 90 },
  { prop: 'host', label: '主机', width: 80 },
  { prop: 'status', label: '状态', width: 80 },
  { prop: 'output', label: '输出' },
]
const svcData = [
  { service: 'HTTP', host: 'openwrt', status: 'OK', output: 'HTTP OK: 200 OK' },
  { service: 'SSH', host: 'openwrt', status: 'OK', output: 'SSH OK - port 22 open' },
  { service: 'NetMon', host: 'openwrt', status: 'OK', output: 'netmon process running' },
]

const alarmCols = [
  { prop: 'time', label: '时间', width: 90 },
  { prop: 'type', label: '类型', width: 70 },
  { prop: 'target', label: '主机/服务' },
  { prop: 'level', label: '状态', width: 80 },
]
const alarmData = [
  { time: '15:28:10', type: '流量', target: 'openwrt/eth0', level: 'WARNING', info: 'TX 速率超过阈值' },
  { time: '15:15:00', type: '服务', target: 'openwrt/NetMon', level: 'OK', info: '服务恢复' },
  { time: '14:50:22', type: '主机', target: 'gateway', level: 'CRITICAL', info: 'ICMP 不可达' },
]

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
.flex-between { display: flex; justify-content: space-between; align-items: center; }
.chart-panels {
  display: grid;
  grid-template-columns: 0.9fr 1fr;
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
}
.flow-card :deep(.el-table) {
  flex: 1;
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
