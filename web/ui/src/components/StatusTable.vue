<template>
  <div class="page-card status-table">
    <div class="page-card-title flex-between">
      <span>{{ title }}</span>
      <a v-if="moreLink" :href="moreLink" class="more">更多 &gt;&gt;</a>
    </div>
    <el-table :data="data" size="small" stripe>
      <el-table-column
        v-for="col in columns"
        :key="col.prop"
        :prop="col.prop"
        :label="col.label"
        :width="col.width"
        :min-width="col.minWidth"
      >
        <template v-if="col.prop === 'status' || col.prop === 'level'" #default="{ row }">
          <el-tag :type="tagType(row[col.prop])" size="small">{{ row[col.prop] }}</el-tag>
        </template>
      </el-table-column>
      <el-table-column v-if="showInfo" prop="info" label="信息" min-width="120" />
    </el-table>
  </div>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({
  title: String,
  columns: Array,
  data: Array,
  moreLink: String,
})

const showInfo = computed(() => props.data?.some((r) => r.info))

function tagType(v) {
  if (v === 'UP' || v === 'OK') return 'success'
  if (v === 'WARNING') return 'warning'
  if (v === 'CRITICAL' || v === 'DOWN') return 'danger'
  return 'info'
}
</script>

<style scoped>
.flex-between { display: flex; justify-content: space-between; align-items: center; }
.more { font-size: 12px; color: var(--accent-blue); text-decoration: none; }
.status-table { min-height: 240px; }
</style>
