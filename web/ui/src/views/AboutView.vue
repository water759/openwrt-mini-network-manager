<template>
  <div class="page-card about">
    <h2>MiniNetMon</h2>
    <p class="ver">版本 1.0.0</p>
    <p>基于 OpenWrt 的迷你网络管理系统，提供流量监控与防火墙管理 Web 界面。</p>
    <el-descriptions :column="1" border size="small" class="mt-16">
      <el-descriptions-item label="前端">Vue 3 + Element Plus + ECharts</el-descriptions-item>
      <el-descriptions-item label="后端">Python 3 HTTP Server</el-descriptions-item>
      <el-descriptions-item label="抓包">netmon (C + libpcap)</el-descriptions-item>
    </el-descriptions>
  </div>
</template>

<script setup>
</script>

<style scoped>
.about h2 { margin-bottom: 4px; }
.ver { color: var(--text-secondary); margin-bottom: 12px; }
.mt-16 { margin-top: 16px; }
</style>
