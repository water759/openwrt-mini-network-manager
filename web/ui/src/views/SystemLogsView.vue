<template>
  <div class="page-card">
    <div class="page-card-title">系统日志</div>
    <el-input v-model="filter" placeholder="过滤日志..." size="small" clearable class="mb-12" style="max-width: 320px" />
    <div class="log-box">
      <div v-for="(line, i) in filtered" :key="i" class="log-line" :class="line.level">{{ line.text }}</div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'

const filter = ref('')
const lines = [
  { level: 'info', text: '[INFO] pcap_open_live success on br-lan' },
  { level: 'info', text: '[INFO] filter: tcp' },
  { level: 'info', text: '[INFO] start capture loop...' },
  { level: 'warn', text: '[WARN] flow table 80% full' },
  { level: 'info', text: '[HTTP] GET /api/traffic 200' },
  { level: 'error', text: '[ERROR] pcap_lookupnet failed: eth0: no ipv4 (fallback OK)' },
]

const filtered = computed(() => {
  const q = filter.value.trim().toLowerCase()
  if (!q) return lines
  return lines.filter((l) => l.text.toLowerCase().includes(q))
})
</script>

<style scoped>
.mb-12 { margin-bottom: 12px; }
.log-box {
  background: #1e1e2d;
  color: #d1d5db;
  font-family: monospace;
  font-size: 13px;
  padding: 12px;
  border-radius: 6px;
  max-height: 480px;
  overflow-y: auto;
}
.log-line { padding: 2px 0; }
.log-line.warn { color: #e6a23c; }
.log-line.error { color: #f56c6c; }
</style>
