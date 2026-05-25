<template>
  <div class="page-card">
    <div class="page-card-title">配置管理</div>
    <p class="hint">以下为当前部署相关路径与接口（只读，修改需在设备上调整 netmon / server.py）。</p>
    <el-form label-width="120px" style="max-width: 520px">
      <el-form-item label="流量数据路径">
        <el-input :model-value="statsPath" readonly />
      </el-form-item>
      <el-form-item label="防火墙规则文件">
        <el-input :model-value="rulesPath" readonly />
      </el-form-item>
      <el-form-item label="iptables 链">
        <el-input model-value="MININETMON_IN / MININETMON_FWD" readonly />
      </el-form-item>
      <el-form-item label="API 基址">
        <el-input :model-value="apiBaseDisplay" readonly />
      </el-form-item>
      <el-form-item label="后端端口">
        <el-input model-value="8080" readonly />
      </el-form-item>
    </el-form>
  </div>
</template>

<script setup>
import { computed } from 'vue'

const statsPath = '/tmp/netmon/stats.json'
const rulesPath = '/tmp/netmon/fw_rules.json'

const apiBaseDisplay = computed(() => {
  const base = import.meta.env.VITE_API_BASE
  if (base) return base
  if (typeof window !== 'undefined') return window.location.origin
  return '(同域)'
})
</script>

<style scoped>
.hint {
  font-size: 13px;
  color: var(--text-secondary);
  margin: 0 0 16px;
}
</style>
