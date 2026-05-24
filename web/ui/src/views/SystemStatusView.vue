<template>
  <div>
    <el-row :gutter="16" class="mb-16">
      <el-col :span="6" v-for="s in statusCards" :key="s.label">
        <div class="page-card status-card">
          <el-icon :size="28" color="#67c23a"><CircleCheck /></el-icon>
          <div>
            <div class="label">{{ s.label }}</div>
            <div class="value ok">{{ s.value }}</div>
          </div>
        </div>
      </el-col>
    </el-row>

    <el-row :gutter="16" class="mb-16">
      <el-col :span="8" v-for="g in gauges" :key="g.label">
        <div class="page-card gauge-card">
          <el-progress type="dashboard" :percentage="g.pct" :color="g.color" :width="120" />
          <div class="gauge-label">{{ g.label }}</div>
        </div>
      </el-col>
    </el-row>

    <div class="page-card">
      <div class="page-card-title">网络接口状态</div>
      <el-table :data="ifaces" stripe size="small">
        <el-table-column prop="name" label="接口" width="100" />
        <el-table-column prop="status" label="状态" width="80">
          <template #default="{ row }"><el-tag type="success" size="small">{{ row.status }}</el-tag></template>
        </el-table-column>
        <el-table-column prop="rx_rate" label="接收速率" />
        <el-table-column prop="tx_rate" label="发送速率" />
        <el-table-column prop="rx_total" label="总接收" />
        <el-table-column prop="tx_total" label="总发送" />
      </el-table>
    </div>
  </div>
</template>

<script setup>
import { CircleCheck } from '@element-plus/icons-vue'

const statusCards = [
  { label: '主机状态', value: '正常' },
  { label: 'NetMon 状态', value: '运行中' },
  { label: '防火墙状态', value: '运行中' },
  { label: '系统负载', value: '正常' },
]



const ifaces = [
  { name: 'br-lan', status: 'UP', rx_rate: '1.2 Mbps', tx_rate: '856 Kbps', rx_total: '1.25 GB', tx_total: '856 MB' },
  { name: 'eth0', status: 'UP', rx_rate: '0 bps', tx_rate: '0 bps', rx_total: '0 B', tx_total: '0 B' },
  { name: 'lo', status: 'UP', rx_rate: '12 Kbps', tx_rate: '12 Kbps', rx_total: '45 MB', tx_total: '45 MB' },
]
</script>

<style scoped>
.mb-16 { margin-bottom: 16px; }
.status-card { display: flex; align-items: center; gap: 12px; }
.status-card .label { font-size: 13px; color: var(--text-secondary); }
.status-card .value.ok { color: var(--accent-green); font-weight: 600; }
.gauge-card { text-align: center; padding: 24px; }
.gauge-label { margin-top: 8px; font-size: 14px; color: var(--text-secondary); }
</style>
