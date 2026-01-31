<template>
  <v-container fluid>
    <v-row>
      <v-col cols="12">
        <v-card>
          <v-toolbar flat dense color="primary">
            <v-toolbar-title>
              <span class="text-h6">遥控器</span>
            </v-toolbar-title>
            <template v-slot:append>
              <v-btn icon="refresh" size="small" @click="fetchData" :loading="loading"></v-btn>
            </template>
          </v-toolbar>

          <!-- 搜索框 -->
          <v-card-text>
            <v-text-field
              v-model="searchQuery"
              label="搜索遥控数据"
              prepend-inner-icon="search"
              variant="outlined"
              density="compact"
              clearable
              hide-details
              @input="filterData"
            ></v-text-field>
          </v-card-text>

          <!-- 按钮网格 -->
          <v-card-text>
            <v-row v-if="filteredDataList.length > 0">
              <v-col
                v-for="item in filteredDataList"
                :key="item.index"
                cols="6"
                sm="4"
                md="3"
              >
                <v-card
                  :color="sendingIndex === item.index ? 'primary' : 'default'"
                  :loading="sendingIndex === item.index"
                  elevation="2"
                  class="control-button"
                  @click="sendSignal(item)"
                >
                  <v-card-text class="text-center pa-4">
                    <v-icon size="48" :color="sendingIndex === item.index ? 'white' : 'primary'">
                      radio_button_checked
                    </v-icon>
                    <div class="text-h6 mt-2" :class="sendingIndex === item.index ? 'text-white' : ''">
                      {{ item.name || '未命名' }}
                    </div>
                    <div class="text-caption mt-1" :class="sendingIndex === item.index ? 'text-white' : 'text-grey'">
                      {{ item.freqType === 0 ? '315MHz' : '433MHz' }} | 协议{{ item.protocol }}
                    </div>
                  </v-card-text>
                </v-card>
              </v-col>
            </v-row>

            <!-- 无数据提示 -->
            <v-card v-else class="text-center pa-8" variant="outlined">
              <v-icon size="64" color="grey">inbox</v-icon>
              <p class="text-grey mt-2">暂无遥控数据</p>
              <p class="text-caption text-grey">请先在数据管理页面添加遥控数据</p>
            </v-card>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- 发送成功提示 -->
    <v-snackbar
      v-model="snackbar"
      :color="snackbarColor"
      :timeout="2000"
      location="top"
    >
      {{ snackbarText }}
    </v-snackbar>
  </v-container>
</template>

<script>
import axios from 'axios';

export default {
  name: 'RadioControl',
  
  data: () => ({
    dataList: [],
    filteredDataList: [],
    searchQuery: '',
    loading: false,
    sendingIndex: null,
    snackbar: false,
    snackbarText: '',
    snackbarColor: 'success'
  }),
  
  mounted() {
    this.fetchData();
  },
  
  methods: {
    async fetchData() {
      this.loading = true;
      try {
        const response = await axios.get('/api/radiodata/list');
        this.dataList = response.data.data || [];
        this.filterData();
      } catch (error) {
        console.error('获取数据失败:', error);
        this.showMessage('获取数据失败', 'error');
      } finally {
        this.loading = false;
      }
    },
    
    filterData() {
      if (!this.searchQuery) {
        this.filteredDataList = [...this.dataList];
      } else {
        const query = this.searchQuery.toLowerCase();
        this.filteredDataList = this.dataList.filter(item => 
          item.name.toLowerCase().includes(query) ||
          item.index.toString().includes(query)
        );
      }
    },
    
    async sendSignal(item) {
      if (this.sendingIndex !== null) {
        return; // 防止重复点击
      }
      
      this.sendingIndex = item.index;
      
      try {
        const response = await axios.post('/api/radiodata/send', {
          index: item.index
        });
        
        if (response.data.result === 'OK') {
          this.showMessage(`已发送: ${item.name}`, 'success');
        } else {
          this.showMessage('发送失败: ' + (response.data.message || '未知错误'), 'error');
        }
      } catch (error) {
        console.error('发送信号失败:', error);
        this.showMessage('发送失败: ' + error.message, 'error');
      } finally {
        this.sendingIndex = null;
      }
    },
    
    showMessage(text, color = 'success') {
      this.snackbarText = text;
      this.snackbarColor = color;
      this.snackbar = true;
    }
  }
}
</script>

<style scoped>
.control-button {
  cursor: pointer;
  transition: all 0.3s ease;
}

.control-button:hover {
  transform: translateY(-4px);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
}

.control-button:active {
  transform: translateY(-2px);
}
</style>
