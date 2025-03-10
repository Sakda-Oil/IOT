/*
 Publish
*/
const char *topic_temp = "temp";
const char *topic_light = "light";
const char *topic_Mode = "Modes";
const char *topic_ModeAuto = "Modes/ligh/temp";
/*
 subscribe
*/   
const char *topic_setupmode = "setup/mode";
const char *topic_setupautomode = "setup/automode";
const char *topic_opencurtain = "slider/curtain";
const char *topic_sliderlight = "slider/light";
const char *topic_slidertemp = "slider/temp";
const char *topic_Open = "open";
const char *topic_Close = "close";
void connectMQTT() {
  while (!client.connected()) {
    String client_id = "ESP32-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT broker as %s\n", client_id.c_str());

    // เชื่อมต่อ MQTT พร้อม Username และ Password
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker!");
      // สมัครรับข้อมูลจาก topics
      client.subscribe(topic_setupmode);
      client.subscribe(topic_setupautomode);
      client.subscribe(topic_opencurtain);
      client.subscribe(topic_Open); 
      client.subscribe(topic_Close);
      client.subscribe(topic_sliderlight); 
      client.subscribe(topic_slidertemp);  
      client.publish(topic_Mode, Mode.c_str());
      StatusLed("Connecting MQTT",500);
    } 
    else 
    {
      Serial.print("Failed to connect, state: ");
      Serial.println(client.state());
      StatusLed("Connecting MQTT",500);
      delay(2000);
    }
  }
}
void callback(char *topic, byte *payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    // Serial.print("Received topic: ");
    // Serial.println(topic); // เพิ่มบรรทัดนี้
    // client.subscribe(topic);
    Serial.print("Message received from topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    Serial.println(message);
    if (strcmp(topic, topic_setupmode) == 0) {
       handleSetupMode(message);
    } else if (strcmp(topic, topic_setupautomode) == 0) {
        handleAutoMode(message);
    } else if (strcmp(topic, topic_opencurtain) == 0) {
        handleCurtain(message);
    } else if (strcmp(topic, topic_Open) == 0) {
        handleStart(message);
    } else if (strcmp(topic, topic_Close) == 0) {
        handleStop(message);
    } else if (strcmp(topic, topic_sliderlight) == 0) {
        handleSliderLight(message);
    } else if (strcmp(topic, topic_slidertemp) == 0) {
        handleSliderTemp(message);
    } 
}
void pubTempValue(){
  float temp = GetTemperature();
  if (temp != -1) { // ตรวจสอบว่าอ่านค่าอุณหภูมิสำเร็จหรือไม่
    String tempStr = String(temp); // แปลง float เป็น String
    client.publish(topic_temp, tempStr.c_str());
  }
}
void pubLightValue(){
  int lux = Getlux_meter();
  String luxStr = String(lux); // แปลง int เป็น String
  client.publish(topic_light, luxStr.c_str());
}
void handleSetupMode(String message) {
    Mode = message;
    Serial.print("Setup Mode: ");
    Serial.println(Mode);
}
void handleAutoMode(String message) {
    AutoTempAndLight = message;
    Serial.print("Auto Mode: ");
    Serial.println(AutoTempAndLight);
}
void handleCurtain(String message) {
    OpenCurtain = message.toInt();
    Serial.print("Curtain Value: ");
    Serial.println(OpenCurtain);
}
void handleSliderLight(String message) {
    SliderLight = message.toInt();
    Serial.print("SliderLight Value: ");
    Serial.println(SliderLight);
}
void handleSliderTemp(String message) {
    SliderTemp = message.toInt();
    Serial.print("SliderTemp Value: ");
    Serial.println(SliderTemp);
}
void handleStart(String message) {
    Open = message.toInt();
    Serial.print("Open: ");
    Serial.println(Close);
}
void handleStop(String message) {
    Close = message.toInt();
    Serial.print("Close: ");
    Serial.println(Close);
}             

void RremoteControl(){
  Key21 command = irController.getKey();
  if (command != Key21::NONE) {
    switch (command) {
      case Key21::KEY_CH_MINUS:
        Serial.println("CH-");
        Mode = "Manual";
        client.publish(topic_Mode, Mode.c_str());
        break;
      case Key21::KEY_CH_PLUS:
        Serial.println("CH+");
        Mode = "Auto";
        client.publish(topic_Mode, Mode.c_str());
        client.publish(topic_ModeAuto, AutoTempAndLight.c_str());
        break;
      case Key21::KEY_1:
        Serial.println("1");
        AutoTempAndLight = "Light";
        client.publish(topic_ModeAuto, AutoTempAndLight.c_str());
        break;

      case Key21::KEY_2:
        Serial.println("2");
        AutoTempAndLight = "Temp";
        client.publish(topic_ModeAuto, AutoTempAndLight.c_str());
        break;
      case Key21::KEY_PREV:
        Serial.println("<<");
        if (countOpen >=1){
          SetMotorstepper(-maxdis);
          countOpen = 0;
          maxdis = 0;
          maxdisRealtime = 0;
          statusCurtain = false;
        }
        else if(countclose >=1)
        {
          SetMotorstepper(-maxdisRealtime);
          countclose = 0;
          maxdis = 0;
          maxdisRealtime = 0;
          statusCurtain = false;
        }
        else if(statusCurtain == true){
          SetMotorstepper(-135);
          maxdis = 0;
          maxdisRealtime = 0;
          statusCurtain = false;
        }
        break;

      case Key21::KEY_NEXT:
        Serial.println(">>");
        if (countOpen >=1)
        {
          SetMotorstepper(-maxdis);
          countOpen = 0;
          maxdis = 135;
          maxdisRealtime = 0;
          statusCurtain = false;
        }
        else if(countclose >=1)
        {
          SetMotorstepper(-maxdisRealtime);
          countclose = 0;
          maxdis = 135;
          maxdisRealtime = 0;
          statusCurtain = false;
        }
        else if(statusCurtain == false){
          SetMotorstepper(135);
          maxdis = 0;
          maxdisRealtime = 0;
          statusCurtain = true;
        }
        break;
      default:
        Serial.println("WARNING: undefined command:");
        break;
    }
  }
}
    
    