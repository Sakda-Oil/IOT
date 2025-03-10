#include <Stepper.h>
#include "pitches.h"
#include "DHT.h"
#include <DIYables_IRcontroller.h> 
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// IR Remote 
#define IR_RECEIVER_PIN 22 // The Arduino pin connected to IR controller
DIYables_IRcontroller_21 irController(IR_RECEIVER_PIN, 200); // debounce time is 200ms

//LDR Sensor
#define LDR  A0 

//DHT11   
#define DHTPIN 4 
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// ULN2003 Motor Driver Pins
#define IN1 19
#define IN2 23
#define IN3 18
#define IN4 5

// initialize the stepper library
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// LED
#define PIN_R 33
#define PIN_G 25
#define PIN_B 26

// Buzzer
#define BUZZZER_PIN  21 


char ssid[] = "OIL"; // Username Wifi
char pass[] = "sakda2845"; // Password Wifi

const char *mqtt_broker = "56824c794ef94d4bbf19f1bc738d03ad.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883;
const char* mqtt_username = "IOTARIS";  // ใส่ Username ของ MQTT
const char* mqtt_password = "Sakda-2845oil";  // ใส่ Password ของ MQTT


WiFiClientSecure espClient;
PubSubClient client(espClient);


String AutoTempAndLight = "Light"; // รับค่าจาก node red
String Mode = "Manual"; // รับค่าจาก node red
int OpenCurtain = 0 ; // รับค่าจาก node red
int Open = 0 ; // รับค่าจาก node red
int Close = 0 ; // รับค่าจาก node red
int SliderTemp = 30; // รับค่าจาก node red
int SliderLight = 150; // รับค่าจาก node red
int disManual = 0; // รับค่าจาก node red

int maxdis = 135 ; // ระยะสูงสุดที่เคลื่อนที่ได้
int maxdisRealtime = 0 ; // ใช้เก็บระยะที่เคลื่อนที่ไปแล้ว

int countOpen = 0; // นับจำนวนครั้งการงานปิดผ้าม่าน
int countclose = 0; // นับจำนวนครั้งการงานเปิดผ้าม่าน
bool statusCurtain = false; // สถานะการของผ้านม่าน ถ้า false แปลว่ายังไม่มีการสั่งปิด
bool off = false; // ตัวบ่งชี้ ว่าม่านได้ถูกปิดสนิทแล้วหรือไม่ ถ้า true แปลว่าปิดสนิทแล้ว
unsigned long lastMsg = 0;  // เก็บเวลาสำหรับ millis()
unsigned long lastMsgled = 0;  // เก็บเวลาสำหรับ millis()

void setup() {
  pinMode(PIN_R, OUTPUT);     // ตั้งค่า PIN_R เป็นขา OUTPUT สำหรับไฟ LED สีแดง
  pinMode(PIN_G, OUTPUT);     // ตั้งค่า PIN_G เป็นขา OUTPUT สำหรับไฟ LED สีเขียว
  pinMode(PIN_B, OUTPUT);     // ตั้งค่า PIN_B เป็นขา OUTPUT สำหรับไฟ LED สีน้ำเงิน
  Serial.begin(115200);       // เริ่มต้นการสื่อสารผ่าน Serial ที่ความเร็ว 115200 baud
  myStepper.setSpeed(15);     // ตั้งค่าความเร็วของมอเตอร์ Stepper ที่ 15 รอบต่อนาที (RPM)
  SetBuzzer();                // เรียกใช้ฟังก์ชัน SetBuzzer() เพื่อตั้งค่าหรือเริ่มต้นใช้งาน Buzzer
  dht.begin();                // เริ่มต้นการทำงานของเซ็นเซอร์วัดอุณหภูมิและความชื้น DHT
  irController.begin();       // เริ่มต้นการทำงานของ IR Controller
  setup_wifi();               // เรียกใช้ฟังก์ชัน setup_wifi() เพื่อตั้งค่า Wi-Fi และเชื่อมต่ออินเทอร์เน็ต
  espClient.setInsecure();    // ตั้งค่าให้ ESP32 ยอมรับการเชื่อมต่อ MQTT แบบไม่มี SSL Certificate ตรวจสอบ
  client.setServer(mqtt_broker, mqtt_port); // ตั้งค่าเซิร์ฟเวอร์ MQTT โดยใช้ broker และ port ที่กำหนด
  client.setCallback(callback); // กำหนดฟังก์ชัน callback() ให้ทำงานเมื่อมีข้อความ MQTT เข้ามา
  connectMQTT();              // เรียกใช้ฟังก์ชัน connectMQTT() เพื่อเชื่อมต่อกับ MQTT Broker
}

void loop() {
  unsigned long now = millis();
  client.loop();
  RremoteControl();
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  else if (!client.connected()) {
    
    connectMQTT();
  }
  if(now - lastMsg > 10000){
    pubTempValue();
    pubLightValue();
  }
  if (Mode == "Auto") {
    StatusLed("Auto",500);
    if (countOpen >=1){
      SetMotorstepper(-maxdis);
      Serial.println(maxdis);
      countOpen = 0;
      maxdis = 135;
      maxdisRealtime = 0;
      statusCurtain = false;
    }
    else if(countclose >=1){
      Serial.println(maxdisRealtime);
      SetMotorstepper(-maxdisRealtime);
      countclose = 0;
      maxdis = 135;
      maxdisRealtime = 0;
      statusCurtain = false;
    }
    else if (AutoTempAndLight == "Light"){
      StatusLed("RunLight",500);
      if (Getlux_meter() >= SliderLight && statusCurtain == false){ 
        Serial.println("Close");
        SetMotorstepper(135);
        statusCurtain = true;
        maxdis = 0;
        maxdisRealtime = 0;
      }
      else if (Getlux_meter() < SliderLight && statusCurtain == true){
        Serial.println("Open");
        SetMotorstepper(-135);
        statusCurtain = false;
        maxdis = 0;
        maxdisRealtime = 0;
      }
      else{
        SetMotorstepper(0);
      }
    }
    else if (AutoTempAndLight == "Temp"){
    StatusLed("RunTemp",500);
      if (GetTemperature() >= SliderTemp && statusCurtain == false){ 
        Serial.println("Close");
        SetMotorstepper(135);
        statusCurtain = true;
        maxdis = 0;
        maxdisRealtime = 135;
      }
      else if (GetTemperature() < SliderTemp && statusCurtain == true){
        Serial.println("Open");
        SetMotorstepper(-135);
        statusCurtain = false;
        maxdis = 0;
        maxdisRealtime = 135;
      }
      else{
          SetMotorstepper(0);
        }
    }
  }
  else if (Mode == "Manual") {
      ModeManual();
  }
}
