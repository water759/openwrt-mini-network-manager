<template>
  <div class="page-card">
    <div class="page-card-title">系统日志</div>
    <el-input v-model="filter" placeholder="过滤日志..." size="small" clearable class="mb-12" style="max-width: 320px" />
    <div v-if="filtered.length" class="log-box">
      <div v-for="(line, i) in filtered" :key="i" class="log-line" :class="line.level">{{ line.text }}</div>
    </div>
    <el-empty v-else description="暂无日志数据（后端未提供日志接口）" :image-size="80" />
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'

const filter = ref('')
const lines = ref([])

const filtered = computed(() => {
  const q = filter.value.trim().toLowerCase()
  if (!q) return lines.value
  return lines.value.filter((l) => l.text.toLowerCase().includes(q))
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
