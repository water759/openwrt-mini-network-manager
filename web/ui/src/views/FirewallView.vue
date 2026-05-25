<template>
  <div class="firewall-page">
    <div class="page-card">
      <div class="page-card-title flex-between">
        <span>防火墙 — 通信规则</span>
        <div class="toolbar">
          <el-button type="primary" size="small" @click="openAddDialog">添加</el-button>
          <el-button size="small" :icon="Refresh" :loading="loading" @click="loadRules">刷新</el-button>
          <el-button size="small" type="danger" plain :loading="submitting" @click="flushRules">清空全部</el-button>
        </div>
      </div>

      <el-alert v-if="msg" :title="msg" :type="msgType" show-icon closable class="mb-12" @close="msg = ''" />

      <div v-loading="loading" class="rule-list-wrap">
        <div class="rule-header">
          <span class="col-name">名称</span>
          <span class="col-dir">匹配</span>
          <span class="col-proto">协议</span>
          <span class="col-src">源区域</span>
          <span class="col-dst">目标</span>
          <span class="col-action">动作</span>
          <span class="col-enable">启用</span>
          <span class="col-btns">操作</span>
        </div>

        <div
          v-for="row in rules"
          :key="row.id"
          class="rule-row"
          :class="{
            'rule-row--drop': row.action === 'DROP',
            'rule-row--disabled': !row.enabled,
          }"
        >
          <span class="col-name rule-name" :title="row.name">{{ row.name }}</span>
          <span class="col-dir">{{ directionLabel(row.direction) }}</span>
          <span class="col-proto proto-text">{{ protoLabel(row) }}</span>
          <span class="col-src">
            <el-tag size="small" effect="plain" :class="zoneTagClass(row.src_zone)">
              {{ row.src_zone }}
            </el-tag>
          </span>
          <span class="col-dst">
            <span v-if="row.dst_zone === 'device'" class="dst-text">此设备</span>
            <el-tag v-else size="small" effect="plain" :class="zoneTagClass(row.dst_zone)">
              {{ row.dst_zone }}
            </el-tag>
          </span>
          <span class="col-action">
            <span :class="row.action === 'ACCEPT' ? 'act-accept' : 'act-drop'">{{ actionLabel(row) }}</span>
          </span>
          <span class="col-enable">
            <el-checkbox
              :model-value="row.enabled"
              :disabled="submitting"
              @change="(v) => toggleRule(row, v)"
            />
          </span>
          <span class="col-btns">
            <el-button size="small" type="primary" :disabled="submitting" @click="openEditDialog(row)">编辑</el-button>
            <el-button size="small" plain type="danger" :disabled="submitting" @click="removeRule(row.id)">删除</el-button>
          </span>
        </div>

        <el-empty v-if="!loading && !rules.length" description="暂无防火墙规则，点击「添加」创建" :image-size="72" />
      </div>
    </div>

    <el-dialog
      v-model="dialogVisible"
      :title="dialogMode === 'edit' ? '编辑规则' : '添加规则'"
      width="560px"
      destroy-on-close
      @closed="resetDialogForm"
    >
      <el-form :model="form" label-width="96px">
        <el-form-item label="规则名称">
          <el-input v-model="form.name" placeholder="留空则自动生成" />
        </el-form-item>
        <el-form-item label="匹配">
          <el-radio-group v-model="form.direction">
            <el-radio label="inbound">入站</el-radio>
            <el-radio label="forward">转发</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item label="协议">
          <el-select v-model="form.protocol" style="width: 100%">
            <el-option label="TCP" value="tcp" />
            <el-option label="UDP" value="udp" />
            <el-option label="ICMP" value="icmp" />
          </el-select>
        </el-form-item>
        <el-form-item label="源区域">
          <el-select v-model="form.src_zone" style="width: 100%">
            <el-option v-for="z in zoneOptions" :key="z.value" :label="z.label" :value="z.value" />
          </el-select>
        </el-form-item>
        <el-form-item v-if="form.src_zone === 'custom'" label="源地址">
          <el-input v-model="form.src" placeholder="192.168.1.0/24" />
        </el-form-item>
        <el-form-item label="目标">
          <el-select v-model="form.dst_zone" style="width: 100%">
            <el-option v-for="z in dstZoneOptions" :key="z.value" :label="z.label" :value="z.value" />
          </el-select>
        </el-form-item>
        <el-form-item v-if="form.dst_zone === 'custom'" label="目的地址">
          <el-input v-model="form.dst" placeholder="0.0.0.0/0" />
        </el-form-item>
        <el-form-item label="端口">
          <el-input v-model="form.port" placeholder="80" :disabled="form.protocol === 'icmp'" />
        </el-form-item>
        <el-form-item label="动作">
          <el-radio-group v-model="form.action">
            <el-radio label="ACCEPT">接受</el-radio>
            <el-radio label="DROP">丢弃</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item label="启用">
          <el-switch v-model="form.enabled" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="dialogVisible = false">取消</el-button>
        <el-button type="primary" :loading="submitting" @click="submitDialog">保存</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, watch, onMounted } from 'vue'
import { Refresh } from '@element-plus/icons-vue'
import { ElMessageBox } from 'element-plus'
import {
  fetchFirewallRules,
  addFirewallRule,
  updateFirewallRule,
  toggleFirewallRule,
  deleteFirewallRule,
  flushFirewallRules,
} from '@/api/firewall'

const zoneOptions = [
  { label: 'LAN (192.168.0.0/16)', value: 'lan' },
  { label: 'WAN (任意)', value: 'wan' },
  { label: '任意', value: 'any' },
  { label: '自定义地址', value: 'custom' },
]

const dstZoneOptions = [
  { label: '此设备', value: 'device' },
  { label: 'LAN', value: 'lan' },
  { label: 'WAN', value: 'wan' },
  { label: '任意', value: 'any' },
  { label: '自定义地址', value: 'custom' },
]

const emptyForm = () => ({
  name: '',
  direction: 'inbound',
  protocol: 'tcp',
  src_zone: 'lan',
  dst_zone: 'device',
  src: '',
  dst: '',
  port: '',
  action: 'ACCEPT',
  enabled: true,
})

const form = ref(emptyForm())
const rules = ref([])
const msg = ref('')
const msgType = ref('success')
const loading = ref(false)
const submitting = ref(false)
const dialogVisible = ref(false)
const dialogMode = ref('add')
const editingId = ref(null)

function displayProtocol(p) {
  return String(p || '').toUpperCase()
}

function normalizeRules(list) {
  return (list || []).map((r) => ({
    ...r,
    protocol: String(r.protocol || '').toLowerCase(),
    direction: r.direction || 'inbound',
    src_zone: r.src_zone || 'any',
    dst_zone: r.dst_zone || 'any',
    port: r.port === '' || r.port == null || r.port === '-' ? '-' : String(r.port),
    enabled: r.enabled !== false,
    name: r.name || `规则-${r.id}`,
  }))
}

function directionLabel(dir) {
  return dir === 'forward' ? '转发' : '入站'
}

function protoLabel(row) {
  const p = displayProtocol(row.protocol)
  if (row.protocol === 'icmp') return 'ICMP'
  if (row.port && row.port !== '-') return `${p} (端口 ${row.port})`
  return p
}

function actionLabel(row) {
  const dir = row.direction === 'forward' ? '转发' : '入站'
  if (row.action === 'ACCEPT') return `接受${dir}`
  return `拒绝${dir}`
}

function zoneTagClass(zone) {
  if (zone === 'lan') return 'zone-tag zone-lan'
  if (zone === 'wan') return 'zone-tag zone-wan'
  if (zone === 'device') return 'zone-tag zone-device'
  return 'zone-tag'
}

const IP_RE = /^(\d{1,3}\.){3}\d{1,3}(\/(\d|[12]\d|3[0-2]))?$/

function isValidIpOrCidr(s) {
  const t = String(s || '').trim()
  if (!IP_RE.test(t)) return false
  return t.split('/')[0].split('.').every((o) => {
    const n = Number(o)
    return n >= 0 && n <= 255
  })
}

function buildPayload() {
  if (form.value.src_zone === 'custom') {
    if (!form.value.src?.trim()) throw new Error('源区域为「自定义」时请填写源地址（如 192.168.1.0/24）')
    if (!isValidIpOrCidr(form.value.src)) throw new Error('源地址格式无效，请使用 IPv4 或 CIDR')
  }
  if (form.value.dst_zone === 'custom') {
    if (!form.value.dst?.trim()) throw new Error('目标为「自定义」时请填写目的地址')
    if (!isValidIpOrCidr(form.value.dst)) throw new Error('目的地址格式无效')
  }
  if (form.value.protocol !== 'icmp' && !form.value.port?.trim()) {
    throw new Error('请填写端口')
  }
  const payload = {
    name: form.value.name.trim(),
    direction: form.value.direction,
    protocol: form.value.protocol,
    src_zone: form.value.src_zone,
    dst_zone: form.value.dst_zone,
    action: form.value.action,
    enabled: form.value.enabled,
  }
  if (form.value.src_zone === 'custom') {
    payload.src = form.value.src.trim()
  }
  if (form.value.dst_zone === 'custom') {
    payload.dst = form.value.dst.trim()
  }
  if (form.value.protocol !== 'icmp') {
    payload.port = form.value.port.trim()
  }
  return payload
}

watch(
  () => form.value.src_zone,
  (z) => {
    if (z !== 'custom') form.value.src = ''
  },
)
watch(
  () => form.value.dst_zone,
  (z) => {
    if (z !== 'custom') form.value.dst = ''
  },
)

function openAddDialog() {
  dialogMode.value = 'add'
  editingId.value = null
  form.value = emptyForm()
  dialogVisible.value = true
}

function openEditDialog(row) {
  dialogMode.value = 'edit'
  editingId.value = row.id
  form.value = {
    name: row.name,
    direction: row.direction,
    protocol: row.protocol,
    src_zone: row.src_zone,
    dst_zone: row.dst_zone,
    src: row.src_zone === 'custom' ? row.src : '',
    dst: row.dst_zone === 'custom' ? row.dst : '',
    port: row.port === '-' ? '' : row.port,
    action: row.action,
    enabled: row.enabled,
  }
  dialogVisible.value = true
}

function resetDialogForm() {
  form.value = emptyForm()
  editingId.value = null
}

async function loadRules() {
  loading.value = true
  try {
    const data = await fetchFirewallRules()
    rules.value = normalizeRules(data.rules)
    msg.value = ''
  } catch (e) {
    rules.value = []
    msg.value = e.message || '加载规则失败'
    msgType.value = 'error'
  } finally {
    loading.value = false
  }
}

async function submitDialog() {
  submitting.value = true
  try {
    const payload = buildPayload()
    const data =
      dialogMode.value === 'edit' && editingId.value != null
        ? await updateFirewallRule(editingId.value, payload)
        : await addFirewallRule(payload)
    rules.value = normalizeRules(data.rules)
    msg.value = dialogMode.value === 'edit' ? '规则已更新' : '规则已添加'
    msgType.value = 'success'
    dialogVisible.value = false
  } catch (e) {
    msg.value = e.message || '保存失败'
    msgType.value = 'error'
  } finally {
    submitting.value = false
  }
}

async function toggleRule(row, enabled) {
  submitting.value = true
  try {
    const data = await toggleFirewallRule(row.id, enabled)
    rules.value = normalizeRules(data.rules)
    msg.value = enabled ? '规则已启用' : '规则已禁用'
    msgType.value = 'success'
  } catch (e) {
    msg.value = e.message || '切换失败'
    msgType.value = 'error'
    await loadRules()
  } finally {
    submitting.value = false
  }
}

async function flushRules() {
  try {
    await ElMessageBox.confirm('确定清空全部自定义规则？', '警告', { type: 'warning' })
  } catch {
    return
  }

  submitting.value = true
  try {
    const data = await flushFirewallRules()
    rules.value = normalizeRules(data.rules)
    msg.value = data.message || '规则已清空'
    msgType.value = 'success'
  } catch (e) {
    msg.value = e.message || '清空失败'
    msgType.value = 'error'
  } finally {
    submitting.value = false
  }
}

async function removeRule(id) {
  try {
    await ElMessageBox.confirm('确定删除该规则？', '提示', { type: 'warning' })
  } catch {
    return
  }

  submitting.value = true
  try {
    const data = await deleteFirewallRule(id)
    rules.value = normalizeRules(data.rules)
    msg.value = '规则已删除'
    msgType.value = 'success'
  } catch (e) {
    msg.value = e.message || '删除失败'
    msgType.value = 'error'
  } finally {
    submitting.value = false
  }
}

onMounted(loadRules)
</script>

<style scoped>
.flex-between {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
  gap: 8px;
}
.toolbar {
  display: flex;
  gap: 8px;
}
.mb-12 { margin-bottom: 12px; }

.rule-list-wrap {
  min-height: 120px;
}

.rule-header,
.rule-row {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 10px 14px;
  font-size: 13px;
}

.rule-header {
  color: var(--text-secondary);
  font-weight: 600;
  border-bottom: 1px solid #ebeef5;
  margin-bottom: 4px;
}

.rule-row {
  border: 1px solid #dcdfe6;
  border-radius: 3px;
  margin-bottom: 8px;
  background: #fff;
  transition: border-color 0.15s, opacity 0.15s;
}

.rule-row:hover {
  border-color: #e6a23c;
  box-shadow: 0 0 0 1px rgba(230, 162, 60, 0.35);
}

.rule-row--disabled {
  opacity: 0.55;
  background: #fafafa;
}

.col-name { flex: 1.4; min-width: 160px; }
.col-dir { width: 48px; flex-shrink: 0; color: #303133; }
.col-proto { width: 130px; flex-shrink: 0; }
.col-src { width: 72px; flex-shrink: 0; }
.col-dst { width: 72px; flex-shrink: 0; }
.col-action { width: 88px; flex-shrink: 0; }
.col-enable { width: 44px; flex-shrink: 0; text-align: center; }
.col-btns {
  width: 148px;
  flex-shrink: 0;
  display: flex;
  gap: 6px;
  justify-content: flex-end;
  margin-left: auto;
}

.rule-name {
  color: #0069d4;
  font-weight: 500;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.proto-text {
  color: #0069d4;
}

.dst-text {
  color: #303133;
}

.act-accept {
  color: #0069d4;
  font-weight: 500;
}

.act-drop {
  color: #c0392b;
  font-weight: 500;
}

.zone-tag {
  border: none;
  text-transform: lowercase;
}

.zone-lan {
  --el-tag-bg-color: #e8f5e9;
  --el-tag-border-color: #c8e6c9;
  --el-tag-text-color: #2e7d32;
}

.zone-wan {
  --el-tag-bg-color: #ffebee;
  --el-tag-border-color: #ffcdd2;
  --el-tag-text-color: #c62828;
}

.zone-device {
  --el-tag-bg-color: #e3f2fd;
  --el-tag-border-color: #bbdefb;
  --el-tag-text-color: #1565c0;
}

@media (max-width: 1100px) {
  .rule-header { display: none; }
  .rule-row {
    flex-wrap: wrap;
    gap: 8px 16px;
  }
  .col-btns {
    width: 100%;
    margin-left: 0;
    justify-content: flex-start;
  }
}
</style>
