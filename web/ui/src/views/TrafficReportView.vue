<template>
  <div class="page-card">
    <div class="page-card-title">流量报表</div>
    <el-form inline class="mb-16">
      <el-form-item label="时间范围">
        <el-date-picker type="datetimerange" start-placeholder="开始" end-placeholder="结束" size="small" />
      </el-form-item>
      <el-form-item label="粒度">
        <el-select model-value="1min" size="small" style="width: 100px">
          <el-option label="1 分钟" value="1min" />
          <el-option label="5 分钟" value="5min" />
        </el-select>
      </el-form-item>
      <el-form-item>
        <el-button type="primary" size="small">查询</el-button>
        <el-button size="small">导出 CSV</el-button>
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
    <RateChart :points="demoPoints" />
  </div>
</template>

<script setup>
import RateChart from '@/components/RateChart.vue'

const summaryCards = [
  { label: '总流量 RX', value: '1.25 GB' },
  { label: '总流量 TX', value: '856 MB' },
  { label: '平均速率', value: '2.4 Mbps' },
]

const demoPoints = Array.from({ length: 30 }, (_, i) => ({
  time: `15:${String(i).padStart(2, '0')}`,
  rx: 800000 + Math.random() * 2_000_000,
  tx: 400000 + Math.random() * 1_000_000,
}))
</script>

<style scoped>
.mb-16 { margin-bottom: 16px; }
.stat-card .label { font-size: 13px; color: var(--text-secondary); }
.stat-card .value { font-size: 20px; font-weight: 700; margin-top: 4px; }
</style>
