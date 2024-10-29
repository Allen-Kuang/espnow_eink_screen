#include <SPI.h>
//EPD
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "Ap_29demo.h"  
#include "GUI_Paint.h"
#include "fonts.h"
//esp now
#include <esp_now.h>
#include <WiFi.h>

#if 1 
   unsigned char BlackImage[EPD_ARRAY];//Define canvas space  
#endif

typedef struct struct_message {
    float heart_rate;
    float breath_rate;
    float lux;
} struct_message;

struct_message receivedData;

// 接收数据的回调函数
void OnDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    memcpy(&receivedData, data, sizeof(receivedData));
    Serial.print("接收到数据 - heart rate: ");
    Serial.print(receivedData.heart_rate);
    Serial.print(", breath rate: ");
    Serial.println(receivedData.breath_rate);

    /***********eink display data***************************/
    //transfer float to const char*, as eink input
    String hearts = String(receivedData.heart_rate);
    String breaths = String(receivedData.breath_rate);
    String luxs = String(receivedData.lux);
    const char* lux = luxs.c_str();
    const char* heart = hearts.c_str();
    const char* breath = breaths.c_str();

		Paint_NewImage(BlackImage, EPD_WIDTH, EPD_HEIGHT, 180, WHITE); //Set canvas parameters, GUI image rotation, please change 0 to 0/90/180/270.
    Paint_SelectImage(BlackImage); //Select current settings.
		
		EPD_Init(); //Full screen refresh initialization.
		Paint_Clear(WHITE); //Clear canvas.
		Paint_DrawString_EN(250, 120, "mmWave Sensor Data", &Font24, WHITE, BLACK); //17*24
    Paint_DrawLine(180, 150, 600, 150, BLACK, LINE_STYLE_SOLID, DOT_PIXEL_1X1); //1x1line 1.
		Paint_DrawString_EN(210, 200, "lux data:", &Font24, WHITE, BLACK); //17*24
		Paint_DrawString_EN(460, 200, lux, &Font24, WHITE, BLACK); //17*24
		Paint_DrawString_EN(210, 250, "breath rate:", &Font24, WHITE, BLACK); //17*24
		Paint_DrawString_EN(460, 250, breath, &Font20, WHITE, BLACK); //14*20.
		Paint_DrawString_EN(210, 300, "heart rate:", &Font24, WHITE, BLACK); //17*24
		Paint_DrawString_EN(460, 300, heart, &Font24, WHITE, BLACK); //17*24
    EPD_Display(BlackImage);//Display GUI image.
		EPD_DeepSleep(); //EPD_DeepSleep,Sleep instruction is necessary, please do not delete!!!
    delay(10000); //Delay for 2s.
}

void setup() {
    pinMode(D5, INPUT);  //BUSY
    pinMode(D0, OUTPUT); //RES 
    pinMode(D3, OUTPUT); //DC   
    pinMode(D1, OUTPUT); //CS  

    //SPI
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0)); 
    SPI.begin ();

    Serial.begin(115200);
    
    // 设置为 Station 模式
    WiFi.mode(WIFI_STA);
    // 初始化 ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW 初始化失败");
        return;
    }

    // 注册接收回调函数
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataReceived));
}

void loop() {
			
}




