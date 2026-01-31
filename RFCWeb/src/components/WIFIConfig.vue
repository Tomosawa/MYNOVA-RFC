<template>
  <v-row justify="center">
    <v-dialog v-model="dialog" persistent width="auto">
      <v-card>
        <v-card-title class="text-h5">
          保存完成
        </v-card-title>
        <v-card-text>WIFI配置保存成功，需要立即进行连接吗？</v-card-text>
        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn color="green-darken-1" variant="text" @click="dialog = false">
            否
          </v-btn>
          <v-btn color="green-darken-1" variant="text" @click="connectWifi">
            是
          </v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>
  </v-row>

  <v-row justify="center">
    <v-dialog v-model="cleardialog" persistent width="auto">
      <v-card>
        <v-card-title class="text-h5">
          清除WI-FI设置？
        </v-card-title>
        <v-card-text>清除WIFI设置后将断开网络连接，确认清除？</v-card-text>
        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn color="green-darken-1" variant="text" @click="cleardialog = false">
            否
          </v-btn>
          <v-btn color="green-darken-1" variant="text" @click="deleteConfig">
            是
          </v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>
  </v-row>

  <v-card class="my-5" />

  <v-card class="mx-auto pa-12 pb-0" max-width="448">
    <v-form ref="form" v-model="form" @submit.prevent="onSubmit">
      <div class="text-subtitle-1 text-medium-emphasis">WIFI网络</div>

      <v-combobox v-model="ST_SSID" :items="wifi_items" item-title="SSID" density="compact" placeholder="WI-FI网络名称"
        variant="outlined" :readonly="loading" :rules="[required]" prepend-inner-icon="wifi">
      </v-combobox>

      <div class="text-subtitle-1 text-medium-emphasis d-flex align-center justify-space-between">
        WIFI密码
      </div>

      <v-text-field v-model="ST_PASSWORD" :type="WIFI_visible ? 'text' : 'password'" :readonly="loading" :rules="[required]"
        density="compact" placeholder="输入WIFI密码" variant="outlined" prepend-inner-icon="lock" :append-inner-icon="WIFI_visible ? 'visibility_off' : 'visibility'" @click:append-inner="WIFI_visible = !WIFI_visible">
      </v-text-field>

      <v-btn block variant="elevated" :disabled="!form" :loading="loading" color="success" size="large" type="submit">
        保存配置
      </v-btn>

      <v-btn color="error" class="mt-4" block @click="clearConfig" :disabled="!form" :loading="loading" size="large">
        删除配置
      </v-btn>
    </v-form>
    <v-switch color="success" v-model="ST_ENABLE" inset @change="onWifiEnableChanged" label="启用WIFI连接" style="height: 56px;"></v-switch>
  </v-card>


  <v-card class="mx-auto pa-12 pb-8" max-width="448">
    <v-form ref="AP_form" v-model="AP_form" @submit.prevent="onAPSubmit">
      <div class="text-subtitle-1 text-medium-emphasis">WIFI热点</div>
      <v-text-field v-model="AP_SSID" type="text" density="compact" placeholder="WIFI热点名称"
        variant="outlined" :readonly="loading" :rules="[required]" prepend-inner-icon="wifi_tethering">
      </v-text-field>

      <div class="text-subtitle-1 text-medium-emphasis d-flex align-center justify-space-between">
        热点密码
      </div>

      <v-text-field v-model="AP_PASSWORD" :type="AP_visible ? 'text' : 'password'" :readonly="loading" :rules="[required]"
        density="compact" placeholder="输入热点密码" variant="outlined" prepend-inner-icon="lock" :append-inner-icon="AP_visible ? 'visibility_off' : 'visibility'"  @click:append-inner="AP_visible = !AP_visible">
      </v-text-field>
      <v-btn block variant="elevated" :disabled="!AP_form" :loading="loading" color="success" size="large" type="submit">
        保存配置
      </v-btn>
      <v-switch color="success" v-model="AP_ENABLE" inset label="启用AP热点" style="height: 56px;" @change="onAPEnable"></v-switch>
    </v-form>
  </v-card>

  <v-card class="mx-auto pa-12 pb-8" max-width="448">
    <v-form ref="HA_form" v-model="HA_form" @submit.prevent="onHASubmit">
      <div class="text-subtitle-1 text-medium-emphasis">Home Assistant 配置</div>
      
      <v-radio-group v-model="HA_MODE" inline>
        <v-radio label="自动发现" value="auto" color="success"></v-radio>
        <v-radio label="手动配置" value="manual" color="success"></v-radio>
      </v-radio-group>

      <v-text-field v-if="HA_MODE === 'manual'" v-model="HA_SERVER" type="text" density="compact" 
        placeholder="Home Assistant IP地址" variant="outlined" :readonly="loading" 
        :rules="HA_MODE === 'manual' ? [required] : []" prepend-inner-icon="dns">
      </v-text-field>

      <v-text-field v-if="HA_MODE === 'manual'" v-model="HA_PORT" type="number" density="compact" 
        placeholder="MQTT端口 (默认1883)" variant="outlined" :readonly="loading" prepend-inner-icon="settings_ethernet">
      </v-text-field>

      <div class="text-subtitle-1 text-medium-emphasis d-flex align-center justify-space-between">
        MQTT 用户名
      </div>

      <v-text-field v-model="HA_USERNAME" type="text" density="compact" 
        placeholder="MQTT用户名（可选）" variant="outlined" :readonly="loading" prepend-inner-icon="person">
      </v-text-field>

      <div class="text-subtitle-1 text-medium-emphasis d-flex align-center justify-space-between">
        MQTT 密码
      </div>

      <v-text-field v-model="HA_PASSWORD" :type="HA_visible ? 'text' : 'password'" :readonly="loading"
        density="compact" placeholder="MQTT密码（可选）" variant="outlined" prepend-inner-icon="lock" 
        :append-inner-icon="HA_visible ? 'visibility_off' : 'visibility'" @click:append-inner="HA_visible = !HA_visible">
      </v-text-field>

      <v-btn block variant="elevated" :disabled="!HA_form" :loading="loading" color="success" size="large" type="submit">
        保存并连接
      </v-btn>

      <v-btn color="warning" class="mt-4" block @click="testHAConnection" :disabled="loading" size="large">
        测试连接
      </v-btn>

      <v-btn color="error" class="mt-2" block @click="clearHAConfig" :disabled="loading" size="large">
        清除配置
      </v-btn>

      <v-alert v-if="HA_status" :type="HA_status_type" class="mt-4" density="compact">
        {{ HA_status_message }}
      </v-alert>
    </v-form>
  </v-card>
  <v-dialog
      v-model="AP_dialog"
      max-width="400"
      persistent
    >
      <v-card
        prepend-icon="warning"
        text-color="red"
        text="警告：如果修改热点并忘记了密码，将导致再也无法连上设备进行设置，请谨慎操作！"
        title="确认修改热点?"
      >
      <v-chip color="red" class="ml-6 mr-6" variant="flat" rounded="0">
        请输入文字：我确认风险并修改
      </v-chip>
        <v-text-field v-model="AP_confirm" class="ml-6 mr-6 mt-2" :rules="[required]" density="compact" placeholder="请输入确认文字" variant="outlined"></v-text-field>
        <template v-slot:actions>
          <v-spacer></v-spacer>

          <v-btn rounded="0" variant="flat" @click="onAPConfirm" :disabled="!isConfirmValid" :color="isConfirmValid ? 'red-darken-1' : ''">
            确认修改
          </v-btn>

          <v-btn color="blue-darken-4" rounded="0" variant="flat" @click="AP_dialog = false">
            取消返回
          </v-btn>
        </template>
      </v-card>
    </v-dialog>
</template>
<script>
  import axios from 'axios';
  export default {
    components: {
    },
    data: () => ({
      form: false,
      AP_form: false,
      HA_form: false,
      ST_SSID: '',
      ST_PASSWORD: '',
      ST_ENABLE: false,
      AP_SSID: '',
      AP_PASSWORD: '',
      AP_ENABLE: false,
      HA_MODE: 'auto',
      HA_SERVER: '',
      HA_PORT: 1883,
      HA_USERNAME: '',
      HA_PASSWORD: '',
      HA_status: false,
      HA_status_type: 'info',
      HA_status_message: '',
      loading: false,
      WIFI_visible: false,
      AP_visible: false,
      HA_visible: false,
      dialog: false,
      cleardialog: false,
      AP_dialog: false,
      AP_confirm: '',
      wifi_items: [{
        SSID: "Network1"
      }, ],
      scanTimer: null, // 扫描轮询定时器
    }),
    mounted() {
      console.log("WIFI config mounted");
      this.getWifiScan();
      this.getAPInfo();
      this.getHAConfig();
    },
    beforeUnmount() {
      // 组件销毁前清理定时器
      if (this.scanTimer) {
        clearTimeout(this.scanTimer);
        this.scanTimer = null;
      }
    },
    methods: {
      onSubmit() {
        if (!this.form) return

        this.loading = true
        this.dialog = true

        var wifissid = "";
        if (typeof this.ST_SSID === 'object')
          wifissid = this.ST_SSID.SSID;
        else
          wifissid = this.ST_SSID;

        var data = {
          SSID: wifissid,
          PWD: this.ST_PASSWORD
        };
        console.log(data);
        axios.post('/api/wifisave', data).then(
          response => {
            console.log(response.data);
            this.loading = false
          },
          error => {
            console.log(error);
            this.loading = false
          }
        );
        //setTimeout(() => (this.loading = false), 2000)
      },
      onAPSubmit(){
        if (!this.AP_form) return
        this.AP_confirm = '';
        this.AP_dialog = true;
      },
      required(v) {
        return !!v || '不能为空'
      },
      getWifiScan(){
        // 清除之前的定时器
        if (this.scanTimer) {
          clearInterval(this.scanTimer);
          this.scanTimer = null;
        }
        
        // 开始扫描
        this.pollWifiScan();
      },
      pollWifiScan(){
        axios.get('/api/wifiscan').then(
          response => {
            console.log('WiFi扫描响应:', response.data)
            
            // 检查是否还在扫描中
            if (response.data.scanning === true) {
              console.log('WiFi扫描中，1秒后重试...')
              // 如果还在扫描，1秒后再次请求
              this.scanTimer = setTimeout(() => {
                this.pollWifiScan();
              }, 1000);
            } else {
              // 扫描完成，更新列表
              console.log('WiFi扫描完成，找到', response.data.count, '个网络')
              this.wifi_items = response.data.WIFI_SCAN || [];
              
              // 清除定时器
              if (this.scanTimer) {
                clearTimeout(this.scanTimer);
                this.scanTimer = null;
              }
            }
          },
          error => {
            console.log('WiFi扫描请求失败:', error.message)
            // 出错也清除定时器
            if (this.scanTimer) {
              clearTimeout(this.scanTimer);
              this.scanTimer = null;
            }
          }
        );
      },
      connectWifi() {
        this.dialog = false
        this.loading = true
        axios.get('/api/wificonnect').then(
          // 成功
          response => {
            console.log('请求成功了', response.data)
            this.loading = false
          },
          // 失败
          error => {
            console.log('请求失败了', error.message)
            this.loading = false
          }
        );
      },
      clearConfig() {
        this.cleardialog = true;
      },
      deleteConfig() {
        axios.delete('/api/wificlear').then(
          // 成功
          response => {
            console.log('请求成功了', response.data)
            this.$refs.form.reset();
            this.cleardialog = false;
          },
          // 失败
          error => {
            console.log('请求失败了', error.message)
            this.cleardialog = false;
          }
        );
      },
      onWifiEnableChanged(){
        //发送请求到/api/wifienable
        var data = {
          WIFI_ENABLE: this.ST_ENABLE
        };
        console.log(data);
        axios.post('/api/wifienable', data).then(
          response => {
            console.log(response.data);
          },
          error => {
            console.log('请求失败了', error.message)
          }
        );
      },
      //获取AP热点信息
      getAPInfo(){
        axios.get('/api/apwifiinfo').then(
          response => {
            console.log('请求成功了', response.data)
            this.AP_SSID = response.data.AP_SSID;
            this.AP_PASSWORD = response.data.AP_PWD;
            this.AP_ENABLE = response.data.AP_ENABLE;
            this.ST_SSID = response.data.WIFI_SSID;
            this.ST_PASSWORD = response.data.WIFI_PASSWORD;
            this.ST_ENABLE = response.data.WIFI_ENABLE;
          },
          error => {
            console.log('请求失败了', error.message)
          } 
        );
      },
      //保存AP热点信息
      saveAPInfo(){
        var data = {
          AP_SSID: this.AP_SSID,
          AP_PWD: this.AP_PASSWORD
        };
        console.log(data);
        axios.post('/api/apsave', data).then(
          response => {
            console.log(response.data);
          },
          error => {
            console.log('请求失败了', error.message)
          }
        );
      },
      //发送AP热点开关
      onAPEnable(){
        var data = {
          AP_ENABLE: this.AP_ENABLE
        };
        console.log(data);
        axios.post('/api/apenable', data).then(
          response => {
            console.log(response.data);
          },
          error => {
            console.log('请求失败了', error.message)
          }
        );
      },
      onAPConfirm(){
        this.AP_dialog = false;
        this.saveAPInfo();
      },
      // Home Assistant 配置相关方法
      onHASubmit(){
        if (!this.HA_form) return;
        this.loading = true;
        this.HA_status = false;

        var data = {
          mode: this.HA_MODE,
          server: this.HA_MODE === 'manual' ? this.HA_SERVER : '',
          port: this.HA_MODE === 'manual' ? parseInt(this.HA_PORT) : 1883,
          username: this.HA_USERNAME,
          password: this.HA_PASSWORD
        };
        
        console.log('保存HA配置:', data);
        
        axios.post('/api/mqtt/config', data).then(
          response => {
            console.log('HA配置保存成功:', response.data);
            this.HA_status = true;
            this.HA_status_type = 'success';
            this.HA_status_message = 'HA配置保存成功！正在连接...';
            
            // 保存成功后尝试连接
            this.connectHA();
          },
          error => {
            console.log('HA配置保存失败:', error.message);
            this.loading = false;
            this.HA_status = true;
            this.HA_status_type = 'error';
            this.HA_status_message = '保存失败: ' + error.message;
          }
        );
      },
      connectHA(){
        axios.get('/api/mqtt/connect').then(
          response => {
            console.log('HA连接响应:', response.data);
            this.loading = false;
            
            if (response.data.connected) {
              this.HA_status = true;
              this.HA_status_type = 'success';
              this.HA_status_message = '✓ 已成功连接到 Home Assistant！';
            } else {
              this.HA_status = true;
              this.HA_status_type = 'warning';
              this.HA_status_message = response.data.message || '连接失败，请检查配置';
            }
          },
          error => {
            console.log('HA连接失败:', error.message);
            this.loading = false;
            this.HA_status = true;
            this.HA_status_type = 'error';
            this.HA_status_message = '连接失败: ' + error.message;
          }
        );
      },
      testHAConnection(){
        this.loading = true;
        this.HA_status = false;
        
        axios.get('/api/mqtt/status').then(
          response => {
            console.log('HA状态:', response.data);
            this.loading = false;
            
            if (response.data.connected) {
              this.HA_status = true;
              this.HA_status_type = 'success';
              this.HA_status_message = '✓ 当前已连接到 Home Assistant';
            } else if (response.data.hasConfig) {
              this.HA_status = true;
              this.HA_status_type = 'warning';
              this.HA_status_message = '有配置但未连接，尝试重新连接...';
              this.connectHA();
            } else {
              this.HA_status = true;
              this.HA_status_type = 'info';
              this.HA_status_message = '未配置 Home Assistant';
            }
          },
          error => {
            console.log('查询状态失败:', error.message);
            this.loading = false;
            this.HA_status = true;
            this.HA_status_type = 'error';
            this.HA_status_message = '查询失败: ' + error.message;
          }
        );
      },
      getHAConfig(){
        axios.get('/api/mqtt/config').then(
          response => {
            console.log('获取HA配置:', response.data);
            
            if (response.data.hasConfig) {
              if (response.data.server && response.data.server.length > 0) {
                this.HA_MODE = 'manual';
                this.HA_SERVER = response.data.server;
                this.HA_PORT = response.data.port || 1883;
              } else {
                this.HA_MODE = 'auto';
              }
              
              this.HA_USERNAME = response.data.username || '';
              this.HA_PASSWORD = response.data.password || '';
            }
          },
          error => {
            console.log('获取HA配置失败:', error.message);
          }
        );
      },
      clearHAConfig(){
        if (!confirm('确认清除 Home Assistant 配置？')) {
          return;
        }
        
        this.loading = true;
        this.HA_status = false;
        
        axios.delete('/api/mqtt/config').then(
          response => {
            console.log('清除HA配置成功:', response.data);
            this.loading = false;
            this.HA_MODE = 'auto';
            this.HA_SERVER = '';
            this.HA_PORT = 1883;
            this.HA_USERNAME = '';
            this.HA_PASSWORD = '';
            this.HA_status = true;
            this.HA_status_type = 'info';
            this.HA_status_message = 'HA配置已清除';
          },
          error => {
            console.log('清除HA配置失败:', error.message);
            this.loading = false;
            this.HA_status = true;
            this.HA_status_type = 'error';
            this.HA_status_message = '清除失败: ' + error.message;
          }
        );
      }
    },
    computed: {
      isConfirmValid() {
        return this.AP_confirm === '我确认风险并修改'
      }
    }
  }
</script>


<style>
</style>
