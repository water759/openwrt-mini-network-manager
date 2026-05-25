<template>
  <div class="flow-table-root">
  <el-table :data="paged" v-loading="loading" size="small" stripe class="fill-table">
    <el-table-column prop="src_ip" label="源 IP" min-width="110" />
    <el-table-column prop="src_port" label="源端口" width="72">
      <template #default="{ row }">{{ row.src_port || '-' }}</template>
    </el-table-column>
    <el-table-column prop="dst_ip" label="目的 IP" min-width="110" />
    <el-table-column prop="dst_port" label="目的端口" width="80">
      <template #default="{ row }">{{ row.dst_port || '-' }}</template>
    </el-table-column>
    <el-table-column prop="protocolName" label="协议" width="64" />
    <el-table-column label="接收" min-width="100">
      <template #default="{ row }">
        <div>{{ formatBytes(row.rx_bytes) }}</div>
        <div class="sub">{{ row.rx_pkts || 0 }} pkts</div>
      </template>
    </el-table-column>
    <el-table-column label="发送" min-width="100">
      <template #default="{ row }">
        <div>{{ formatBytes(row.tx_bytes) }}</div>
        <div class="sub">{{ row.tx_pkts || 0 }} pkts</div>
      </template>
    </el-table-column>
    <el-table-column label="总流量" min-width="120">
      <template #default="{ row }">
        <div class="bar-cell">
          <span>{{ formatBytes(row.total) }}</span>
          <el-progress :percentage="barPct(row.total)" :show-text="false" :stroke-width="6" color="#67c23a" />
        </div>
      </template>
    </el-table-column>
  </el-table>
  <div class="pager">
    <span class="total">共 {{ flows.length }} 条</span>
    <el-pagination
      v-model:current-page="pageModel"
      v-model:page-size="pageSizeModel"
      :page-sizes="[10, 20, 50]"
      layout="sizes, prev, pager, next"
      :total="flows.length"
      small
    />
  </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { formatBytes } from '@/utils/format'

const props = defineProps({
  flows: { type: Array, default: () => [] },
  loading: Boolean,
  page: { type: Number, default: 1 },
  pageSize: { type: Number, default: 10 },
})

const emit = defineEmits(['update:page', 'update:pageSize'])

const pageModel = computed({
  get: () => props.page,
  set: (v) => emit('update:page', v),
})
const pageSizeModel = computed({
  get: () => props.pageSize,
  set: (v) => emit('update:pageSize', v),
})

const maxTotal = computed(() => Math.max(1, ...props.flows.map((f) => f.total || 0)))

const paged = computed(() => {
  const start = (props.page - 1) * props.pageSize
  return props.flows.slice(start, start + props.pageSize)
})

function barPct(total) {
  return Math.min(100, Math.round(((total || 0) / maxTotal.value) * 100))
}
</script>

<style scoped>
.flow-table-root {
  width: 100%;
  flex: 1;
  display: flex;
  flex-direction: column;
  min-height: 0;
}
.fill-table {
  width: 100% !important;
  flex: 1;
}
.sub { font-size: 11px; color: var(--text-secondary); }
.bar-cell span { font-size: 12px; display: block; margin-bottom: 4px; }
.pager {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-top: 12px;
}
.total { font-size: 12px; color: var(--text-secondary); }
</style>
