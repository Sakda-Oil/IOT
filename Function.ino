void ModeManual() {
    StatusLed("Run", 500);
    disManual = map(OpenCurtain, 0, 100, 0, 135);
    if (Close == 1) {
        countclose++;
        countOpen = 0;
        Defensesystemstepper(countclose);
        maxdisRealtime = 135 - maxdis;
        Serial.print("countclose = "); Serial.print(countclose);
        Serial.print(" maxdisRealtime = "); Serial.print(maxdisRealtime);
        Serial.print(" maxdis = "); Serial.print(maxdis);
        Serial.print(" disManual = "); Serial.println(disManual);
        SetMotorstepper(abs(disManual));
        Open = 0;
        Close = 0;
    } else if (Open == 1 && statusCurtain == true) {
        countOpen++;
        countclose = 0;
        Defensesystemstepper(countOpen);
        maxdisRealtime = 135 - maxdis;
        Serial.print("countOpen = "); Serial.print(countOpen);
        Serial.print(" maxdisRealtime = "); Serial.print(maxdisRealtime);
        Serial.print(" maxdis = "); Serial.print(maxdis);
        Serial.print(" disManual = "); Serial.println(disManual);
        SetMotorstepper(-abs(disManual));
        Open = 0;
        Close = 0;
    } else {
        SetMotorstepper(0);
    }
}
void Defensesystemstepper(int Counts){
        if (Counts == 1) {
            off = false;
            if (statusCurtain == false) {
              if(maxdisRealtime == 135)
              {
                disManual = 0;
              }
              else
              {
                maxdis -= disManual;
              }
            } else {
                Serial.println("maxdisRealtime");
                maxdis = maxdisRealtime;
                maxdis -= disManual;
                if (maxdis < 0) {
                    if (off == false) {
                        Serial.println("off");
                        maxdis += disManual;
                        disManual = maxdis;
                        maxdis = 0;
                        off = true;
                    } else {
                        disManual = 0;
                        maxdis = 0;
                    }
                }else {
                    disManual = disManual;
                }
            }
            statusCurtain = true;
        } else {
            maxdis -= disManual;
            if (maxdis < 0) {
                if (off == false) {
                    Serial.println("off");
                    maxdis += disManual;
                    disManual = maxdis;
                    maxdis = 0;
                    off = true;
                } else {
                    disManual = 0;
                    maxdis = 0;
                }
            } else {
                disManual = disManual;
            }
        }
}
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};
void SetBuzzer(){
    for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZZER_PIN, melody[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZZER_PIN);
  }
}
void SetBuzzerrun(){
    int noteDuration = 1000 / noteDurations[0];
  tone(BUZZZER_PIN, melody[0], 4);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZZER_PIN);
}
int Getlux_meter(){    //ฟังก์ชั่นแปลงค่าดิจิทัลเป็นค่า Lux
  int LDR_Val = analogRead(LDR);
  float Vout = float(LDR_Val) * (3.3 / float(4096));
  float RLDR = (10000 * (3.3 - Vout))/Vout; 
  int lux = 500/(RLDR/1000); 
  return lux;
}
float GetTemperature() {
  float t = dht.readTemperature(); 
  if (isnan(t)) {  // ตรวจสอบ NaN และ 0
    Serial.println(F("Failed to read from DHT sensor!"));
    return -1; 
  }
  return t; 
}
void Prinlux_meter(){
  float LDR_Val = analogRead(LDR);
  Serial.print("LUX Meter : ");
  Serial.print(Getlux_meter());   //เรียกใช้ฟังก์ชั่น lux_meter()
  Serial.println("  lux"); 
  delay(1000);
}
void PrintTemperature(){
  delay(1000);
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
}
void SetMotorstepper (double distance){
  SetBuzzerrun();
  // max 135 min -135
  // คำนวณจำนวนสเต็ปที่ต้องการให้มอเตอร์หมุนจากระยะทาง
  int steps = (abs(distance) / (PI * 12.5)) * stepsPerRevolution; 
  if (distance == 0)
  {
    myStepper.step(0);
  }
  else if (distance > 0)
  {
    myStepper.step(steps);
    delay(500);
  }
  else if (distance < 0)
  {
    myStepper.step(-steps);
    delay(500);
  }
}
void setColor(int R, int G, int B) {
  analogWrite(PIN_R, R);
  analogWrite(PIN_G, G);
  analogWrite(PIN_B, B);
}
void StatusLed (String Status,int time) {
    unsigned long nowled = millis();
    if (Status == "Connecting WIFI"){
      if (nowled - lastMsgled > time) {  // ส่งข้อมูลทุกๆ 3 วินาที
          setColor(255, 0, 0);//Red
          delay(500); 
          setColor(0, 0, 0);
          delay(500);
          setColor(0, 255, 0);//Green 
          delay(500); 
          setColor(0, 0, 0);
          delay(500); 
          setColor(0, 0, 255); //Blue
          delay(500);
          setColor(0, 0, 0);
      }
    }
    if (Status == "Connecting MQTT"){
      if (nowled - lastMsgled > time) {
          setColor(255, 0, 0);//Red
          delay(500); 
          setColor(0, 0, 0);
          delay(500); 
      }
    }
    if (Status == "Run"){
      if (nowled - lastMsgled > time) {
          setColor(0, 255, 0);//Green 
          delay(500); 
          setColor(0, 0, 0);
          delay(500);
      }
    }
    if (Status == "RunTemp"){
      if (nowled - lastMsgled > time) {
          setColor(0, 0, 255);//Green 
          delay(500); 
          setColor(0, 0, 0);
          delay(500);
      }
    }
    if (Status == "RunLight"){
      if (nowled - lastMsgled > time) { 
          setColor(255, 255, 255);//Green 
          delay(500); 
          setColor(0, 0, 0);
          delay(500); 
      }
    }
    if (Status == "Auto"){
      if (nowled - lastMsgled > time) {
          setColor(191, 0, 255);//Purple 
          delay(500); 
          setColor(0, 0, 0);
          delay(500);
      }
    }
}
void setup_wifi() {
  StatusLed("Connecting WIFI",500);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    StatusLed("Connecting WIFI",500);
    RremoteControl();
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}