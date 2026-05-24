<template>
  <div class="page-card">
    <div class="page-card-title">告警历史</div>
    <el-form inline class="mb-16">
      <el-form-item label="状态">
        <el-select model-value="all" size="small" style="width: 100px">
          <el-option label="全部" value="all" />
          <el-option label="CRITICAL" value="critical" />
          <el-option label="WARNING" value="warning" />
        </el-select>
      </el-form-item>
      <el-form-item label="类型">
        <el-select model-value="all" size="small" style="width: 100px">
          <el-option label="全部" value="all" />
        </el-select>
      </el-form-item>
      <el-form-item>
        <el-button type="primary" size="small">查询</el-button>
      </el-form-item>
    </el-form>
    <el-table :data="rows" stripe size="small">
      <el-table-column prop="time" label="时间" width="160" />
      <el-table-column prop="level" label="级别" width="100">
        <template #default="{ row }">
          <el-tag :type="levelType(row.level)" size="small">{{ row.level }}</el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="target" label="对象" min-width="140" />
      <el-table-column prop="status" label="状态" width="90" />
      <el-table-column prop="message" label="信息" min-width="200" />
    </el-table>
  </div>
</template>

<script setup>
const rows = [
  { time: '2025-05-19 15:28:10', level: 'WARNING', target: 'openwrt/eth0', status: '告警', message: 'TX 速率超过阈值 2Mbps' },
  { time: '2025-05-19 15:15:00', level: 'OK', target: 'openwrt/NetMon', status: '恢复', message: 'netmon 进程恢复运行' },
  { time: '2025-05-19 14:50:22', level: 'CRITICAL', target: 'gateway', status: '告警', message: 'ICMP 不可达' },
  { time: '2025-05-19 14:30:00', level: 'WARNING', target: 'openwrt/br-lan', status: '告警', message: '连接数超过 200' },
]

function levelType(l) {
  if (l === 'OK') return 'success'
  if (l === 'WARNING') return 'warning'
  return 'danger'
}
</script>

<style scoped>
.mb-16 { margin-bottom: 16px; }
</style>
