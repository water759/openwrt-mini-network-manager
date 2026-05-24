<template>
  <div class="firewall-page">
    <div class="page-card">
      <div class="page-card-title">添加防火墙规则</div>
      <el-form :model="form" label-width="88px" inline class="rule-form">
        <el-form-item label="协议">
          <el-select v-model="form.protocol" style="width: 120px">
            <el-option label="TCP" value="tcp" />
            <el-option label="UDP" value="udp" />
            <el-option label="ICMP" value="icmp" />
          </el-select>
        </el-form-item>
        <el-form-item label="源地址">
          <el-input v-model="form.src" placeholder="192.168.1.0/24" style="width: 160px" />
        </el-form-item>
        <el-form-item label="目的地址">
          <el-input v-model="form.dst" placeholder="0.0.0.0/0" style="width: 160px" />
        </el-form-item>
        <el-form-item label="端口">
          <el-input v-model="form.port" placeholder="80" style="width: 100px" :disabled="form.protocol === 'icmp'" />
        </el-form-item>
        <el-form-item label="动作">
          <el-radio-group v-model="form.action">
            <el-radio label="ACCEPT">ACCEPT</el-radio>
            <el-radio label="DROP">DROP</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="addRule">添加规则</el-button>
          <el-button type="danger" plain @click="flushRules">清空全部</el-button>
        </el-form-item>
      </el-form>
      <el-alert v-if="msg" :title="msg" :type="msgType" show-icon closable class="mt-12" @close="msg = ''" />
    </div>

    <div class="page-card">
      <div class="page-card-title flex-between">
        <span>当前防火墙规则</span>
        <el-button size="small" :icon="Refresh" @click="loadRules">刷新</el-button>
      </div>
      <el-table :data="rules" stripe size="small">
        <el-table-column type="index" label="#" width="50" />
        <el-table-column prop="protocol" label="协议" width="80" />
        <el-table-column prop="src" label="源地址" min-width="120" />
        <el-table-column prop="dst" label="目的地址" min-width="120" />
        <el-table-column prop="port" label="端口" width="72" />
        <el-table-column prop="action" label="动作" width="90">
          <template #default="{ row }">
            <el-tag :class="row.action === 'ACCEPT' ? 'tag-accept' : 'tag-drop'" size="small">{{ row.action }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column label="操作" width="80">
          <template #default="{ row }">
            <el-button type="danger" link size="small" @click="removeRule(row.id)">删除</el-button>
          </template>
        </el-table-column>
      </el-table>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { Refresh } from '@element-plus/icons-vue'
import { ElMessageBox } from 'element-plus'

const form = ref({
  protocol: 'tcp',
  src: '192.168.1.0/24',
  dst: '0.0.0.0/0',
  port: '80',
  action: 'ACCEPT',
})

const rules = ref([
  { id: 1, protocol: 'TCP', src: '192.168.1.0/24', dst: '0.0.0.0/0', port: '80', action: 'ACCEPT' },
  { id: 2, protocol: 'TCP', src: '0.0.0.0/0', dst: '192.168.1.1', port: '22', action: 'DROP' },
  { id: 3, protocol: 'UDP', src: '192.168.1.0/24', dst: '8.8.8.8', port: '53', action: 'ACCEPT' },
])

const msg = ref('')
const msgType = ref('success')
let nextId = 4

function addRule() {
  if (!form.value.src || !form.value.dst) {
    msg.value = '请填写源地址和目的地址'
    msgType.value = 'error'
    return
  }
  rules.value.push({
    id: nextId++,
    protocol: form.value.protocol.toUpperCase(),
    src: form.value.src,
    dst: form.value.dst,
    port: form.value.protocol === 'icmp' ? '-' : (form.value.port || '-'),
    action: form.value.action,
  })
  msg.value = '规则已添加（本地演示，待接入后端脚本）'
  msgType.value = 'success'
}

async function flushRules() {
  try {
    await ElMessageBox.confirm('确定清空全部自定义规则？', '警告', { type: 'warning' })
    rules.value = []
    msg.value = '规则已清空'
    msgType.value = 'success'
  } catch { /* cancelled */ }
}

function removeRule(id) {
  rules.value = rules.value.filter((r) => r.id !== id)
}

function loadRules() {
  msg.value = '列表已刷新（待接入 /api/firewall/rules）'
  msgType.value = 'info'
}

onMounted(loadRules)
</script>

<style scoped>
.rule-form { flex-wrap: wrap; }
.flex-between { display: flex; justify-content: space-between; align-items: center; }
.mt-12 { margin-top: 12px; }
</style>
