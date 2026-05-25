import { createRouter, createWebHashHistory } from 'vue-router'
import AppLayout from '@/layouts/AppLayout.vue'

const routes = [
  {
    path: '/',
    component: AppLayout,
    redirect: '/traffic',
    children: [
      { path: 'traffic', name: 'traffic', component: () => import('@/views/TrafficView.vue'), meta: { title: '流量监控' } },
      { path: 'firewall', name: 'firewall', component: () => import('@/views/FirewallView.vue'), meta: { title: '防火墙管理' } },
      { path: 'report', name: 'report', component: () => import('@/views/TrafficReportView.vue'), meta: { title: '流量报表' } },
      { path: 'system', name: 'system', component: () => import('@/views/SystemStatusView.vue'), meta: { title: '系统状态' } },
      { path: 'config', name: 'config', component: () => import('@/views/ConfigView.vue'), meta: { title: '配置管理' } },
      { path: 'about', name: 'about', component: () => import('@/views/AboutView.vue'), meta: { title: '关于系统' } },
    ],
  },
]

export default createRouter({
  history: createWebHashHistory(),
  routes,
})
