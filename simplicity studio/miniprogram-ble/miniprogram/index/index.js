const app = getApp()

function inArray(arr, key, val) {
  for (let i = 0; i < arr.length; i++) {
    if (arr[i][key] === val) {
      return i;
    }
  }
  return -1;
}

/* ArrayBuffer转16进度字符串示例 */
function ab2hex(buffer) {
  var hexArr = Array.prototype.map.call(
    new Uint8Array(buffer),
    function (bit) {
      return ('00' + bit.toString(16)).slice(-2)
    }
  )
  return hexArr.join('');
}

Page({
   data: {
   connectedDeviceId: "", //已连接设备uuid
   services: "", // 连接设备的服务
   characteristics: "",   // 连接设备的状态值
   writeServicweId: "", // 可写服务uuid
   writeCharacteristicsId: "",//可写特征值uuid
   readServicweId: "", // 可读服务uuid
   readCharacteristicsId: "",//可读特征值uuid
   notifyServicweId: "", //通知服务UUid
   current_set_temp: 25, // 当前设定温度初始值
   led_warn_state: 0, // led报警状态
   tempertureCharId: '00002A6E-0000-1000-8000-00805F9B34FB',
   ledCharId: '00002A56-0000-1000-8000-00805F9B34FB',
   },

   /**
      * 生命周期函数--监听页面加载
      */
   onLoad: function (options) {
      // 初始化蓝牙适配器
      this.initBluetoothAdapter();
      // 本机蓝牙适配器状态
      this.getBluetoothAdapterState();
      // 开始搜索外围设备
      this.startBluetoothDevicesDiscovery();
      // this.onBluetoothDeviceFound();
   },
    
   /**
      * 生命周期函数--监听页面初次渲染完成
      */
   onReady: function () {
   },
    
   /**
      * 生命周期函数--监听页面显示
      */
   onShow: function () {
   },
    
   /**
      * 生命周期函数--监听页面隐藏
      */
   onHide: function () {
   },
    
   /**
      * 生命周期函数--监听页面卸载
      */
   onUnload: function () {
   },
    
   /**
      * 页面相关事件处理函数--监听用户下拉动作
      */
   onPullDownRefresh: function () {
   },
    
   /**
      * 页面上拉触底事件的处理函数
      */
   onReachBottom: function () {
   },
    
   /**
      * 用户点击右上角分享
      */
   onShareAppMessage: function () {
   },

   slidertest: function(e) {
      this.setData({
         current_set_temp: e.detail.value,
      })
   },
   /* 初始化蓝牙适配器 */
   initBluetoothAdapter: function () {
      wx.openBluetoothAdapter({
         success: function(res) {
            console.log('初始化蓝牙适配器成功：' + JSON.stringify(res));
         },
         fail: function(res) {
            console.log('初始化蓝牙适配器失败：' + JSON.stringify(res));
         },
         complete: function(res){
            console.log('初始化蓝牙适配器完成：' + JSON.stringify(res));
         }
      })
   },

   /* 本机蓝牙适配器状态 */
   getBluetoothAdapterState: function() {
      wx.getBluetoothAdapterState({
         success: function(res) {
            console.log( '本机蓝牙适配器状态获取成功：' + JSON.stringify(res));
         },
         fail: function(res) {
            console.log( '本机蓝牙适配器状态获取失败：' + JSON.stringify(res));
         },
         complete: function(res) {
            console.log( '本机蓝牙适配器状态获取完成：' + JSON.stringify(res));
         }
      })
   },

   /* 开始搜寻附近的蓝牙外围设备 */
   startBluetoothDevicesDiscovery: function() {
      wx.startBluetoothDevicesDiscovery({
         success: function(res) {
            console.log('搜索设备成功返回：' + JSON.stringify(res));
         },
         fail: function(res) {
            console.log('搜索设备失败返回：' + JSON.stringify(res));
         },
         complete: function(res) {
            console.log('搜索设备完成返回：' + JSON.stringify(res));
         }
      })
   },

   /* 监听寻找到新设备的事件 */
   onBluetoothDeviceFound: function() {
      wx.onBluetoothDeviceFound(function(res) {
         console.log('新设备列表已发现');
         console.log( res.devices);
      })
   },

   /* 获取在蓝牙模块生效期间所有已发现的蓝牙设备。包括已经和本机处于连接状态的设备。*/
   getBluetoothDevices: function() {
      var that = this;
      wx.getBluetoothDevices({
         success: function(res) {
            console.log('搜到的蓝牙设备数目：' + res.devices.length);
            console.log(res.devices);
            that.setData({
               devices: res.devices,
               /* 显示可用设备开关 */
               show_available_devices_switch: 0,
            });
         },
         fail: function(res) {
            console.log('搜索蓝牙设备失败');
         },
         complete: function(res) {
            console.log('搜索蓝牙设备完成');
         }
      })
   },

   /* 连接设备 */
   connectTO: function(e) {
      var that = this;
      /* 打印当前已连接设备name和id*/
      console.log('deviceName:', e.currentTarget.dataset.deviceName)
      console.log('deviceId:', e.currentTarget.id);
      /* 先停止搜索周边设备 */
      that.stopBluetoothDevicesDiscovery();
      that.data.device_id = e.currentTarget.id;
      that.data.device_name = e.currentTarget.dataset.deviceName;
      that.setData({
         device_id: that.data.device_id,
         device_name: that.data.device_name,
      })
      /* 连接低功耗蓝牙设备 */
      that.createBLEConnection();
   },

   /* 连接低功耗蓝牙设备 */
   createBLEConnection: function() {
      var that = this;
      wx.showLoading({
         title: '连接蓝牙设备中...',
      });
      wx.createBLEConnection({
         deviceId: that.data.device_id,
         success: function(res) {
            wx.showLoading({
               title: '连接成功',
            })
            console.log('连接设备成功返回：'+res.errMsg);
            /* 连接成功后延时500ms */
            setTimeout(function(){
               wx.hideLoading()
            }, 500) 
            /* 连接成功后读设备服务uuid */
            that.getBLEDeviceServices();
            /* 定时2s去读取蓝牙开发板的温湿度数据 */
            setInterval(function() {
  
               /* 读取温度与设定温度比较，如果超过设定温度，客户端发送消息给服务端并告知报警，否则继续读取读取当前温度。*/
               if(that.data.current_temperture > that.data.current_set_temp) {
                  /* 开启报警状态 */
                  that.data.led_warn_state = 1;
                  console.log('已超温度设定值');
                  /* 定义一个写入蓝牙开发板的buf，buf长度为4 */
                  var buf = new ArrayBuffer(4);
                  var dataView = new DataView(buf);
                  /* 将buf[0]数据设定为'2'作为蓝牙开发板点亮LED的指令 */
                  dataView.setUint8(0, 2);
                  /* 向低功耗蓝牙设备特征值中写入二进制数据 */
                  that.writeBLECharacteristicValue(buf);
               }
               else {
                  /* 关闭报警状态 */
                  that.data.led_warn_state = 0;
                  console.log('未超温度设定值');
                  /* 定义一个写入蓝牙开发板的buf，buf长度为4 */
                  var buf = new ArrayBuffer(4);
                  /* 将buf[0]数据设定为'4'作为蓝牙开发板关闭LED的指令 */
                  var dataView = new DataView(buf);
                  dataView.setUint8(0, 4);
                  /* 向低功耗蓝牙设备特征值中写入二进制数据 */
                  that.writeBLECharacteristicValue(buf);
               }
               /* 监听读取到温湿度数据 */
               that.onBLECharacteristicValueChange();
               /* 读取低功耗蓝牙设备的特征值的二进制数据值 */
               that.readBLECharacteristicValue();
            }, 2000); 
            that.setData({
               /* 更新显示可用设备开关 */
               show_available_devices_switch: 1,
               /* 已连接设备开关 */
               connected_device_switch: 0,
            })
         },
         fail: function() {
            console.log("连接设备失败");
         },
         complete: function() {
            console.log("连接设备完成");
         }
      })
   },

   /* 监听低功耗蓝牙设备的特征值变化事件 */
   onBLECharacteristicValueChange: function() {
      var that = this;
      wx.onBLECharacteristicValueChange(function(res) {
         var int16View = new Int16Array(res.value)
         console.log(int16View[0]);
         /* 接收的当前温度数据并转化实际温度显示值 */
         that.data.current_temperture = int16View[0] / 100;
         that.setData({
            /* 定义一个当前温度变量 */
            current_temperture: that.data.current_temperture,
         })
      })
   },

   /* 读取低功耗蓝牙设备的特征值的二进制数据值 */
   readBLECharacteristicValue: function() {
      var that = this;
      wx.readBLECharacteristicValue({
         // 这里的 deviceId 需要已经通过 createBLEConnection 与对应设备建立链接
         deviceId: that.data.device_id,
         // 这里的 serviceId 需要在 getBLEDeviceServices 接口中获取
         serviceId: that.data.services[6].uuid,
         // 这里的 characteristicId 需要在 getBLEDeviceCharacteristics 接口中获取
         characteristicId:  that.data.tempertureCharId,
         success(res) {
            console.log('readBLECharacteristicValue suceess:', res.errCode)
         }
      })
   },

   /* 向低功耗蓝牙设备特征值中写入二进制数据 */
   writeBLECharacteristicValue: function(buf) {
      var that = this;
      wx.writeBLECharacteristicValue({
         deviceId: that.data.device_id,
         serviceId: that.data.services[1].uuid,
         characteristicId: that.data.ledCharId,
         value: buf,
         success: function (res) {
            console.log('writeBLECharacteristicValue success:', res.errCode)
            that.setData({
               led_warn_state: that.data.led_warn_state,
            })
         }
      })
   },

   /* 停止搜索周边设备 */
   stopBluetoothDevicesDiscovery: function() {
      wx.stopBluetoothDevicesDiscovery({ 
         success: function(res){
            console.log('连接设备前，先停止搜索周边设备:');
            console.log(res);
         },
         fail: function(res){
            console.log('停止搜索蓝牙设备失败');
         },
         complete: function(res){
            console.log('停止搜索蓝牙设备完成');
         }
      });
   },

   /* 获取连接设备的service服务UUID */
   getBLEDeviceServices: function() {
      var that = this;
      wx.getBLEDeviceServices({
         /* 这里的 deviceId 需要在上面的 getBluetoothDevices 或 onBluetoothDeviceFound 接口中获取 */
         deviceId: that.data.device_id,
         success: function(res) {
            console.log('device services:', JSON.stringify(res.services));
            for(var i=0;i<res.services.length;i++){
               console.log(i+"--UUID:------"+res.services[i].uuid)
            }
            that.setData({
               services: res.services,
               msg: JSON.stringify(res.services),
            })
         }
      })
   },

   /* 获取连接设备的所有特征值 */
   getBLEDeviceCharacteristics: function() {
      var that = this;
      var myuuid = that.data.services[6].uuid //具有读写通知属性的服务uuid
      //var myuuid = that.data.ALLUUID//具有读、写、通知、属性的服务uuid
      console.log('myuuid' + myuuid)
      wx.getBLEDeviceCharacteristics({
         // 这里的 deviceId 需要在上面的 getBluetoothDevices 或 onBluetoothDeviceFound 接口中获取
         deviceId: that.data.connectedDeviceId,
         // 这里的 serviceId 需要在上面的 getBLEDeviceServices 接口中获取
         serviceId: myuuid,
         success: function (res) {
            for (var i = 0; i < res.characteristics.length; i++) {
               console.log("i =", i);
               console.log('特征值：' + res.characteristics[i].uuid)
               if (res.characteristics[i].properties.notify) {
                  console.log("获取开启notify的ServicweId：", myuuid);
                  console.log("获取开启notify的CharacteristicsId：", res.characteristics[i].uuid);
                  that.setData({
                     notifyServicweId: myuuid,
                     notifyCharacteristicsId: res.characteristics[i].uuid,   
                  })
               }
               if (res.characteristics[i].properties.write) {
                  console.log("获取开启write的ServicweId：", myuuid);
                  console.log("获取开启write的CharacteristicsId：", res.characteristics[i].uuid);
                  that.setData({
                     writeServicweId: myuuid,
                     writeCharacteristicsId: res.characteristics[i].uuid,
                  })
               } else if (res.characteristics[i].properties.read) {
                  console.log("读read操作readServicweId：", myuuid);
                  console.log("读read操作：readCharacteristicsId", res.characteristics[i].uuid);
                  that.setData({
                     readServicweId: myuuid,
                     readCharacteristicsId: res.characteristics,
                  })   
               }
            }
            console.log('device getBLEDeviceCharacteristics:', res.characteristics);
            console.log('read test', res.characteristics[3].uuid);
            that.setData({
               msg: JSON.stringify(res.characteristics),
            })
         },
         fail: function () {
            console.log("fail");
         },
         complete: function () {
            console.log("complete");
         }
      })    
   },
})
    
    
 