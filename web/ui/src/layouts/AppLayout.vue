<template>
  <div class="layout">
    <aside class="sidebar">
      <div class="brand">
        <div class="brand-sub">MiniNetMon</div>
      </div>

      <nav class="nav">
        <div class="nav-group" v-for="group in navGroups" :key="group.label">
          <div class="nav-group-label">{{ group.label }}</div>
          <router-link
            v-for="item in group.items"
            :key="item.path + item.title"
            :to="item.path"
            class="nav-item"
            :class="{ active: isActive(item) }"
          >
            <el-icon><component :is="item.icon" /></el-icon>
            <span>{{ item.title }}</span>
          </router-link>
        </div>
      </nav>

    </aside>

    <div class="main">
      <header class="topbar">
        <div class="topbar-left">
          <el-icon class="menu-icon"><Fold /></el-icon>
          <h1>迷你网络管理系统</h1>
        </div>
        <div class="topbar-right">
          <span class="clock">{{ nowStr }}</span>
          <span class="status-ok" :class="{ 'status-bad': !apiOk }">
            <span class="dot"></span>
            系统状态: {{ apiOk ? 'OK' : '异常' }}
          </span>
          <el-select v-model="pollSec" size="small" style="width: 88px" @change="onPollChange">
            <el-option label="1秒" :value="1" />
            <el-option label="2秒" :value="2" />
            <el-option label="5秒" :value="5" />
          </el-select>
          <el-button :icon="Refresh" circle size="small" @click="onRefresh" />
        </div>
      </header>

      <div class="content">
        <router-view />
      </div>

      <footer class="footer-bar">
        MiniNetMon v1.0.0 | © 2026|by.白开水
      </footer>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { useRoute } from 'vue-router'
import {
  Fold, Refresh,
  Connection, Lock,
  DataLine, Monitor, Tools, InfoFilled,
} from '@element-plus/icons-vue'
import { formatTime } from '@/utils/format'
import { fetchHealth } from '@/api/traffic'

const route = useRoute()
const nowStr = ref(formatTime())
const pollSec = ref(2)
const apiOk = ref(false)

let clockTimer = null
let healthTimer = null

async function checkHealth() {
  try {
    const h = await fetchHealth()
    apiOk.value = !!h.ok
  } catch {
    apiOk.value = false
  }
}

const navGroups = [
  {
    label: 'General',
    items: [
      { path: '/traffic', title: '流量监控', icon: Connection },
      { path: '/firewall', title: '防火墙管理', icon: Lock },
    ],
  },
  {
    label: 'Reports',
    items: [
      { path: '/report', title: '流量报表', icon: DataLine },
    ],
  },
  {
    label: 'System',
    items: [
      { path: '/system', title: '系统状态', icon: Monitor },
      { path: '/config', title: '配置管理', icon: Tools },
      { path: '/about', title: '关于系统', icon: InfoFilled },
    ],
  },
]

function isActive(item) {
  return route.path === item.path
}

function onPollChange() {
  window.dispatchEvent(new CustomEvent('netmon-poll-change', { detail: pollSec.value }))
}

function onRefresh() {
  checkHealth()
  window.dispatchEvent(new CustomEvent('netmon-refresh'))
}

onMounted(() => {
  clockTimer = setInterval(() => { nowStr.value = formatTime() }, 1000)
  checkHealth()
  healthTimer = setInterval(checkHealth, 5000)
})

onUnmounted(() => {
  if (clockTimer) clearInterval(clockTimer)
  if (healthTimer) clearInterval(healthTimer)
})
</script>

<style scoped>
.layout {
  display: flex;
  height: 100vh;
  overflow: hidden;
}

.sidebar {
  width: 220px;
  background: var(--sidebar-bg);
  color: #c0c4cc;
  display: flex;
  flex-direction: column;
  flex-shrink: 0;
}

.brand {
  padding: 20px 16px 16px;
  border-bottom: 1px solid rgba(255,255,255,.06);
}

.brand-title {
  font-size: 22px;
  font-weight: 700;
  color: #fff;
}

.brand-sub {
  font-size: 13px;
  color: var(--nagios-yellow);
  margin-top: 2px;
}

.nav {
  flex: 1;
  overflow-y: auto;
  padding: 8px 0;
}

.nav-group-label {
  font-size: 11px;
  text-transform: uppercase;
  letter-spacing: .5px;
  color: #6b7280;
  padding: 12px 16px 6px;
}

.nav-item {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 10px 16px;
  color: #b0b3bb;
  text-decoration: none;
  font-size: 14px;
  border-left: 3px solid transparent;
  transition: background .15s;
}

.nav-item:hover {
  background: var(--sidebar-hover);
  color: #fff;
}

.nav-item.active {
  background: var(--sidebar-active);
  color: #fff;
  border-left-color: var(--sidebar-active-border);
}

.main {
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  background: var(--content-bg);
}

.topbar {
  height: 52px;
  background: var(--header-bg);
  color: #e5e7eb;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 20px;
  flex-shrink: 0;
}

.topbar-left {
  display: flex;
  align-items: center;
  gap: 12px;
}

.topbar-left h1 {
  font-size: 16px;
  font-weight: 600;
}

.menu-icon {
  font-size: 18px;
  cursor: pointer;
}

.topbar-right {
  display: flex;
  align-items: center;
  gap: 14px;
  font-size: 13px;
}

.clock { color: #d1d5db; }

.status-ok {
  display: flex;
  align-items: center;
  gap: 6px;
  color: var(--nagios-green);
}

.status-ok .dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: var(--nagios-green);
}

.status-ok.status-bad {
  color: #f56c6c;
}

.status-ok.status-bad .dot {
  background: #f56c6c;
}

.content {
  flex: 1;
  overflow-y: auto;
  padding: 16px 20px;
}
</style>
