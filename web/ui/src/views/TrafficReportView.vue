<template>
  <div class="page-card">
    <div class="page-card-title">流量报表</div>
    <el-alert v-if="error" type="error" :title="error" show-icon closable class="mb-16" />
    <el-alert v-else-if="!online" type="warning" title="netmon 未运行或 stats.json 不存在" show-icon class="mb-16" />

    <el-form inline class="mb-16">
      <el-form-item>
        <el-button type="primary" size="small" :loading="loading" @click="refresh">刷新数据</el-button>
      </el-form-item>
    </el-form>

    <el-row :gutter="16" class="mb-16">
      <el-col :span="8" v-for="c in summaryCards" :key="c.label">
        <div class="stat-card">
          <div class="label">{{ c.label }}</div>
          <div class="value">{{ c.value }}</div>
        </div>
      </el-col>
    </el-row>

    <div v-if="chartPoints.length" class="chart-wrap">
      <RateChart :points="chartPoints" />
    </div>
    <el-empty v-else description="暂无流量数据，请先启动 netmon" :image-size="80" />
  </div>
</template>

<script setup>
import { computed } from 'vue'
import RateChart from '@/components/RateChart.vue'
import { useTraffic } from '@/composables/useTraffic'

const {
  totals, loading, error, online, chartPoints, avgRxBps, avgTxBps,
  refresh, formatBytes, formatBps,
} = useTraffic(2000)

const summaryCards = computed(() => [
  { label: '总流量 RX', value: formatBytes(totals.value.rx_bytes) },
  { label: '总流量 TX', value: formatBytes(totals.value.tx_bytes) },
  {
    label: '平均速率',
    value: `${formatBps(avgRxBps.value)} / ${formatBps(avgTxBps.value)}`,
  },
])
</script>

<style scoped>
.mb-16 { margin-bottom: 16px; }
.stat-card .label { font-size: 13px; color: var(--text-secondary); }
.stat-card .value { font-size: 20px; font-weight: 700; margin-top: 4px; }
.chart-wrap { min-height: 280px; }
</style>
