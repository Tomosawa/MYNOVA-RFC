import { createRouter, createWebHistory } from 'vue-router'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'RadioControl',
      component: () => import('../components/RadioControl.vue')
    },
    {
      path: '/manage',
      name: 'RadioDataManage',
      component: () => import('../components/RadioDataManage.vue')
    },
    {
      path: '/wificonfig',
      name: 'WIFIConfig',
      component: () => import('../components/WIFIConfig.vue')
    }
  ]
})

export default router
