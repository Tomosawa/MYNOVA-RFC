<template>
  <v-card class="ma-2">
    <v-toolbar flat dense>
      <v-toolbar-title>
        <span class="text-subheading">无线遥控数据管理</span>
      </v-toolbar-title>
      <template v-slot:append>
        <v-btn icon="add" size="small" variant="tonal" @click="showAddDialog"></v-btn>
      </template>
    </v-toolbar>

    <!-- 搜索框 -->
    <v-card-text>
      <v-text-field
        v-model="searchQuery"
        label="搜索数据"
        prepend-inner-icon="search"
        variant="outlined"
        density="compact"
        clearable
        @input="filterData"
      ></v-text-field>
    </v-card-text>

    <!-- 数据列表 -->
    <v-card-text>
      <v-list v-if="filteredDataList.length > 0">
        <v-virtual-scroll
          :items="displayedData"
          height="500"
          item-height="72"
        >
          <template v-slot:default="{ item }">
            <v-list-item>
              <template v-slot:prepend>
                <v-avatar color="primary">
                  <span class="text-h6">{{ item.index }}</span>
                </v-avatar>
              </template>
              
              <v-list-item-title>{{ item.name || '未命名' }}</v-list-item-title>
              <v-list-item-subtitle>
                数据: {{ item.data }} | 位长: {{ item.bitLength }} | 协议: {{ item.protocol }}
              </v-list-item-subtitle>
              <v-list-item-subtitle>
                脉冲: {{ item.pulseLength }}μs | 频率: {{ item.freqType === 0 ? '315MHz' : '433MHz' }}
              </v-list-item-subtitle>

              <template v-slot:append>
                <v-btn icon="edit" size="small" variant="text" @click="showEditDialog(item)"></v-btn>
                <v-btn icon="delete" size="small" variant="text" color="error" @click="showDeleteDialog(item)"></v-btn>
              </template>
            </v-list-item>
            <v-divider></v-divider>
          </template>
        </v-virtual-scroll>
      </v-list>
      
      <v-card v-else class="text-center pa-5" variant="outlined">
        <v-icon size="64" color="grey">inbox</v-icon>
        <p class="text-grey mt-2">暂无数据</p>
      </v-card>

      <!-- 加载更多按钮 -->
      <div v-if="hasMore" class="text-center mt-4">
        <v-btn @click="loadMore" variant="outlined" :loading="loading">
          加载更多
        </v-btn>
      </div>
    </v-card-text>
  </v-card>

  <!-- 添加/编辑对话框 -->
  <v-dialog v-model="editDialog" max-width="600" persistent>
    <v-form ref="form" v-model="formValid" @submit.prevent="saveData">
      <v-card>
        <v-card-title>
          <span class="text-h5">{{ isEditMode ? '编辑数据' : '添加数据' }}</span>
        </v-card-title>

        <v-card-text>
          <v-row dense>
            <v-col cols="12">
              <v-text-field
                v-model="editItem.name"
                label="数据名称"
                :rules="[rules.required, rules.nameFormat, rules.nameLength]"
                variant="outlined"
                density="compact"
                placeholder="仅支持数字和英文，最多10个字符"
                counter="10"
                @input="filterNameInput"
              ></v-text-field>
            </v-col>

            <v-col cols="6">
              <v-select
                v-model="editItem.freqType"
                label="频率类型"
                :items="freqTypes"
                item-title="label"
                item-value="value"
                :rules="[rules.required]"
                variant="outlined"
                density="compact"
              ></v-select>
            </v-col>

            <v-col cols="6">
              <v-select
                v-model="editItem.protocol"
                label="协议"
                :items="protocolOptions"
                :rules="[rules.required]"
                variant="outlined"
                density="compact"
              ></v-select>
            </v-col>

            <v-col cols="6">
              <v-select
                v-model="editItem.bitLength"
                label="位长度"
                :items="bitLengthOptions"
                :rules="[rules.required]"
                variant="outlined"
                density="compact"
                @update:modelValue="onBitLengthChange"
              ></v-select>
            </v-col>

            <v-col cols="6">
              <v-text-field
                v-model.number="editItem.pulseLength"
                label="脉冲长度(μs)"
                type="number"
                :rules="[rules.required, rules.positive, rules.maxPulseLength]"
                variant="outlined"
                density="compact"
                hint="0-999"
              ></v-text-field>
            </v-col>

            <v-col cols="12">
              <v-text-field
                v-model="editItem.dataHex"
                label="数据值(十六进制)"
                :rules="[rules.required, rules.hexFormat, rules.hexLength]"
                variant="outlined"
                density="compact"
                prefix="0x"
                :hint="`最多${maxHexDigits}位十六进制`"
                @input="onDataHexInput"
              ></v-text-field>
            </v-col>
          </v-row>
        </v-card-text>

        <v-divider></v-divider>

        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn text="取消" variant="plain" @click="editDialog = false"></v-btn>
          <v-btn
            color="primary"
            text="保存"
            variant="tonal"
            type="submit"
            :disabled="!formValid"
            :loading="saving"
          ></v-btn>
        </v-card-actions>
      </v-card>
    </v-form>
  </v-dialog>

  <!-- 删除确认对话框 -->
  <v-dialog v-model="deleteDialog" persistent width="auto">
    <v-card>
      <v-card-title class="text-h5">确认删除</v-card-title>
      <v-card-text>您确定要删除数据 "{{ deleteItem?.name }}" 吗？</v-card-text>
      <v-card-actions>
        <v-spacer></v-spacer>
        <v-btn color="green-darken-1" variant="text" @click="deleteDialog = false">
          取消
        </v-btn>
        <v-btn color="red-lighten-1" variant="flat" @click="confirmDelete" :loading="deleting">
          删除
        </v-btn>
      </v-card-actions>
    </v-card>
  </v-dialog>
</template>

<script>
import axios from 'axios';

export default {
  data: () => ({
    dataList: [],
    filteredDataList: [],
    displayedData: [],
    searchQuery: '',
    loading: false,
    saving: false,
    deleting: false,
    pageSize: 10,
    currentPage: 1,
    
    editDialog: false,
    deleteDialog: false,
    formValid: false,
    isEditMode: false,
    
    editItem: {
      index: null,
      name: '',
      data: 0,
      dataHex: '',
      bitLength: 24,
      protocol: 1,
      pulseLength: 100,
      freqType: 0
    },
    
    deleteItem: null,
    
    freqTypes: [
      { label: '315MHz', value: 0 },
      { label: '433MHz', value: 1 }
    ],
    
    protocolOptions: [1, 2, 3, 4, 5, 6],
    
    bitLengthOptions: [4, 8, 12, 16, 20, 24, 28, 32, 36, 40],
    
    maxHexDigits: 6,
    
    rules: {
      required: v => !!v || v === 0 || v === '' || '此字段为必填项',
      positive: v => v > 0 || '必须大于0',
      maxPulseLength: v => v <= 999 || '最大值为999',
      nameFormat: v => {
        if (!v) return true;
        return /^[0-9A-Za-z]*$/.test(v) || '只能输入数字和英文字母';
      },
      nameLength: v => {
        if (!v) return true;
        return v.length <= 10 || '最多10个字符';
      },
      hexFormat: v => {
        if (!v) return true;
        return /^[0-9A-Fa-f]*$/.test(v) || '只能输入十六进制字符(0-9,A-F)';
      },
      hexLength: function(v) {
        if (!v) return true;
        return v.length <= this.maxHexDigits || `最多${this.maxHexDigits}位十六进制`;
      }
    }
  }),
  
  computed: {
    hasMore() {
      return this.displayedData.length < this.filteredDataList.length;
    }
  },
  
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
          item.data.toString().includes(query) ||
          item.index.toString().includes(query)
        );
      }
      this.currentPage = 1;
      this.updateDisplayedData();
    },
    
    updateDisplayedData() {
      const endIndex = this.currentPage * this.pageSize;
      this.displayedData = this.filteredDataList.slice(0, endIndex);
    },
    
    loadMore() {
      this.currentPage++;
      this.updateDisplayedData();
    },
    
    filterNameInput() {
      // 过滤掉非数字和非英文字母的字符
      if (this.editItem.name) {
        const filtered = this.editItem.name.replace(/[^0-9A-Za-z]/g, '');
        // 限制长度为10
        this.editItem.name = filtered.substring(0, 10);
      }
    },
    
    showAddDialog() {
      this.isEditMode = false;
      this.editItem = {
        index: null,
        name: '',
        data: 0,
        dataHex: '',
        bitLength: 24,
        protocol: 1,
        pulseLength: 100,
        freqType: 0
      };
      this.updateMaxHexDigits();
      this.editDialog = true;
    },
    
    showEditDialog(item) {
      this.isEditMode = true;
      this.editItem = { ...item };
      // 将数据值转换为十六进制字符串
      this.editItem.dataHex = this.editItem.data.toString(16).toUpperCase();
      this.updateMaxHexDigits();
      this.editDialog = true;
    },
    
    showDeleteDialog(item) {
      this.deleteItem = item;
      this.deleteDialog = true;
    },
    
    async saveData() {
      if (!this.formValid) return;
      
      this.saving = true;
      try {
        const endpoint = this.isEditMode 
          ? '/api/radiodata/update' 
          : '/api/radiodata/add';
        
        // 将十六进制字符串转换为数值
        const dataToSend = {
          ...this.editItem,
          data: parseInt(this.editItem.dataHex || '0', 16)
        };
        delete dataToSend.dataHex;
        
        const response = await axios.post(endpoint, dataToSend);
        
        if (response.data.result === 'OK') {
          this.editDialog = false;
          await this.fetchData();
        }
      } catch (error) {
        console.error('保存数据失败:', error);
      } finally {
        this.saving = false;
      }
    },
    
    onBitLengthChange() {
      this.updateMaxHexDigits();
      // 如果当前十六进制值超过新的最大长度，截断它
      if (this.editItem.dataHex.length > this.maxHexDigits) {
        this.editItem.dataHex = this.editItem.dataHex.substring(0, this.maxHexDigits);
      }
      // 验证当前值是否超过位长限制
      const maxValue = this.getMaxValueForBitLength(this.editItem.bitLength);
      const currentValue = parseInt(this.editItem.dataHex || '0', 16);
      if (currentValue > maxValue) {
        this.editItem.dataHex = maxValue.toString(16).toUpperCase();
      }
    },
    
    updateMaxHexDigits() {
      // 每4位二进制=1位十六进制
      this.maxHexDigits = Math.ceil(this.editItem.bitLength / 4);
    },
    
    getMaxValueForBitLength(bitLength) {
      if (bitLength >= 32) {
        return 0xFFFFFFFF;
      }
      return (1 << bitLength) - 1;
    },
    
    onDataHexInput(event) {
      // 只允许输入十六进制字符
      let value = event.target.value;
      value = value.replace(/[^0-9A-Fa-f]/g, '');
      // 转换为大写
      value = value.toUpperCase();
      // 限制长度
      if (value.length > this.maxHexDigits) {
        value = value.substring(0, this.maxHexDigits);
      }
      this.editItem.dataHex = value;
      
      // 验证是否超过位长限制
      if (value) {
        const maxValue = this.getMaxValueForBitLength(this.editItem.bitLength);
        const currentValue = parseInt(value, 16);
        if (currentValue > maxValue) {
          this.editItem.dataHex = maxValue.toString(16).toUpperCase();
        }
      }
    },
    
    async confirmDelete() {
      this.deleting = true;
      try {
        const response = await axios.post('/api/radiodata/delete', {
          index: this.deleteItem.index
        });
        
        if (response.data.result === 'OK') {
          this.deleteDialog = false;
          await this.fetchData();
        }
      } catch (error) {
        console.error('删除数据失败:', error);
      } finally {
        this.deleting = false;
      }
    }
  },
  
  watch: {
    'editItem.bitLength'(newVal) {
      this.updateMaxHexDigits();
    }
  }
}
</script>

<style scoped>
.v-list-item {
  border-bottom: 1px solid rgba(0, 0, 0, 0.12);
}
</style>
