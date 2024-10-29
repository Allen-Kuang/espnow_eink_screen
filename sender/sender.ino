#include <Arduino.h>
#include <hp_BH1750.h>
#include "Seeed_Arduino_mmWave.h"
//esp now
#include <esp_now.h>
#include <WiFi.h>

// Set up serial communication depending on the board type
#ifdef ESP32
#  include <HardwareSerial.h>
HardwareSerial mmWaveSerial(0);
#else
#  define mmWaveSerial Serial1
#endif

SEEED_MR60BHA2 mmWave;
hp_BH1750 BH1750;

typedef struct struct_message {
    float breath_rate;
    float heart_rate;
    float lux;
} struct_message;

struct_message myData;

// 声明 peerAddr
uint8_t peerAddr[] = {0x54, 0x32, 0x04, 0x03, 0x36, 0x8C};  // 替换为接收方的 MAC 地址

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("数据发送状态: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "成功" : "失败");
}

void setup() {
    Serial.begin(115200);
    mmWave.begin(&mmWaveSerial);

    //-----------------light sensor-----------------------
    bool avail = BH1750.begin(BH1750_TO_GROUND);
    if (!avail) {
    Serial.println("No BH1750 sensor found!");
    while (true) {}
    }
    Serial.printf("conversion time: %dms\n", BH1750.getMtregTime());
    BH1750.start();

    //------------------esp now---------------------------
    // 设置为 Station 模式
    WiFi.mode(WIFI_STA);
    // 初始化 ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW 初始化失败");
        return;
    }

    // 设置发送回调函数
    esp_now_register_send_cb(OnDataSent);

    // 添加接收方的 peer 信息
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, peerAddr, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    // 添加接收方
    esp_now_add_peer(&peerInfo);
}

void loop() {

    //get light sensor data
    if (BH1750.hasValue()) {
      float lux = BH1750.getLux();
      myData.lux = lux;
      Serial.printf("lux: %.2f\n", lux);
      BH1750.start();
    }

    //get mmwave data in 10s
    if (mmWave.update(10000)) {
        float breath_rate;
        if (mmWave.getBreathRate(breath_rate)) {
          Serial.printf("breath_rate: %.2f\n", breath_rate);
        }
        float heart_rate;
        if (mmWave.getHeartRate(heart_rate)) {
          Serial.printf("heart_rate: %.2f\n", heart_rate);
        }

        // 判断 breath_rate 和 heart_rate 是否都不等于 0，esp now发送信息
        if (breath_rate != 0.0f && heart_rate != 0.0f) {
            Serial.println("get in");
            myData.breath_rate = breath_rate;
            myData.heart_rate = heart_rate;
            // 发送数据
            esp_err_t result = esp_now_send(peerAddr, (uint8_t *)&myData, sizeof(myData));
            if (result == ESP_OK) {
                Serial.println("发送成功");
            } else {
                Serial.println("发送失败");
            }
        }
    }
}