<template>
  <div class="page-card status-table">
    <div class="page-card-title flex-between">
      <span>{{ title }}</span>
      <a v-if="moreLink" :href="moreLink" class="more">更多 &gt;&gt;</a>
    </div>
    <div class="table-wrap">
      <el-table :data="data" size="small" stripe class="fill-table">
        <el-table-column
          v-for="col in columns"
          :key="col.prop"
          :prop="col.prop"
          :label="col.label"
          :width="col.width"
          :min-width="col.minWidth || (col.width ? undefined : 100)"
        >
          <template v-if="col.prop === 'status' || col.prop === 'level'" #default="{ row }">
            <el-tag :type="tagType(row[col.prop])" size="small">{{ row[col.prop] }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column v-if="showInfo" prop="info" label="信息" min-width="120" />
      </el-table>
    </div>
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

.status-table {
  width: 100%;
  display: flex;
  flex-direction: column;
  margin-bottom: 0;
  box-sizing: border-box;
}

.table-wrap {
  flex: 1;
  width: 100%;
  min-width: 0;
}

.fill-table {
  width: 100% !important;
}

.status-table :deep(.el-table__header),
.status-table :deep(.el-table__body) {
  width: 100% !important;
}

.status-table :deep(.el-table__header-wrapper table),
.status-table :deep(.el-table__body-wrapper table) {
  width: 100% !important;
}
</style>
