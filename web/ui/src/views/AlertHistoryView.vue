<template>
  <div class="page-card">
    <div class="page-card-title">告警历史</div>
    <el-table v-if="rows.length" :data="rows" stripe size="small">
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
    <el-empty v-else description="暂无告警数据（后端未提供告警接口）" :image-size="80" />
  </div>
</template>

<script setup>
import { ref } from 'vue'

const rows = ref([])

function levelType(l) {
  if (l === 'OK') return 'success'
  if (l === 'WARNING') return 'warning'
  return 'danger'
}
</script>
