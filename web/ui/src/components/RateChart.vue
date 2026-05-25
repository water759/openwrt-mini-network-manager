<template>
  <v-chart class="rate-chart" :option="option" autoresize />
</template>

<script setup>
import { computed } from 'vue'
import VChart from 'vue-echarts'
import { use } from 'echarts/core'
import { CanvasRenderer } from 'echarts/renderers'
import { LineChart } from 'echarts/charts'
import { GridComponent, TooltipComponent, LegendComponent } from 'echarts/components'

use([CanvasRenderer, LineChart, GridComponent, TooltipComponent, LegendComponent])

const props = defineProps({
  points: { type: Array, default: () => [] },
})

const option = computed(() => ({
  tooltip: { trigger: 'axis' },
  legend: { data: ['接收速率 (bps)', '发送速率 (bps)'], bottom: 0 },
  grid: { left: 48, right: 16, top: 24, bottom: 40 },
  xAxis: {
    type: 'category',
    boundaryGap: false,
    data: props.points.map((p) => p.time),
    axisLabel: { fontSize: 10 },
  },
  yAxis: {
    type: 'value',
    name: 'bps',
    axisLabel: {
      formatter: (v) => (v >= 1_000_000 ? `${(v / 1_000_000).toFixed(1)}M` : v >= 1000 ? `${(v / 1000).toFixed(0)}K` : v),
    },
  },
  series: [
    {
      name: '接收速率 (bps)',
      type: 'line',
      smooth: true,
      showSymbol: false,
      lineStyle: { color: '#67c23a', width: 2 },
      itemStyle: { color: '#67c23a' },
      areaStyle: { color: 'rgba(103,194,58,.08)' },
      data: props.points.map((p) => Math.round(p.rx)),
    },
    {
      name: '发送速率 (bps)',
      type: 'line',
      smooth: true,
      showSymbol: false,
      lineStyle: { color: '#409eff', width: 2, type: 'dashed' },
      itemStyle: { color: '#409eff' },
      data: props.points.map((p) => Math.round(p.tx)),
    },
  ],
}))
</script>

<style scoped>
.rate-chart {
  width: 100%;
  height: 100%;
  min-height: 300px;
}
</style>
