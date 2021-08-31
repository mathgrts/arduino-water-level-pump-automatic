//menggunakan arduino mini pro dengan LCD_i2c, ultrasonic, button, water level konduktor, dan LEDs
//setelah di pertimbangkan, fungsi koneksi ke wifi dimatikan karena mengurangi kestabilan kinerja device
// Last edited by ILMI on 20 Feb 2021

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
/*
  #include <WiFiEsp.h>
  #include <WiFiEspClient.h>
  #include <WiFiEspUdp.h>
  #include <SoftwareSerial.h>
  #include <PubSubClient.h>
  
  //--------------KONFIGURASI WIFI ESP8266 DAN MQTT--------------
  IPAddress server(192, 168, 2, 100);
  char ssid[] = "ssid_name";
  char pass[] = "ssid_pass";
  int status = WL_IDLE_STATUS;



  //Callback function header
  void callback(char* topic, byte* payload, unsigned int length);

  WiFiEspClient espClient;
  PubSubClient client(server, 1883, callback, espClient);

  SoftwareSerial espSerial(16, 17); //ArduinoRX, ArduinoTX
  //connect esp8266 TX to Arduino RX, dan sebaliknya

  long lastReconnectAttempt = 0;


  //reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--reco--
  boolean reconnect() {

  if (client.connect("minipro1")) {
    //      Serial.println("connected");
    digitalWrite(ledServer, HIGH);
    client.subscribe("minipro1/#");
  }
  return client.connected();
  }
*/

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
int baris1;
const int buzzer = 7;
#define ledServer 13

//-----------------Button-------------
#define buttPin 15
int buttValue =0;
int buttState = 0;

//-----------------LEDs-------------
const int led1 = 6;
const int led2 = 8;
const int led3 = 9;
const int led4 = 12;

//--------------Konfigurasi Bak SENSOR-------------
#define sensorAtas 5
#define sensorBawah 4
int bakPenuh = 0;
int bakKosong = 0;

//--------------Konfigurasi Ultrasonic SENSOR-------------
// defines pins numbers
const int trigPin = 3;
const int echoPin = 2;

// defines variables
long duration;
int distance;
int water;

const int pump  = 10;
const int stopper  = 11;

long lastMsg2 = 0;

boolean errorSensor = 0;
boolean overridePump = 0;

//--------------Konfigurasi Thermistor SENSOR-------------
//connect thermistor sensor to thermistor pin, and to vcc 5v
int ThermistorPin = A0;
int Vo;
float R1 = 6000;
float logR2, R2, T, Tc, Tf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

long lastMsg = 0;

//stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--stup--
void setup() {
  //  Serial.begin(9600);//initialize serial for debugging

  lcd.init();                      // initialize the lcd
  lcd.init();
  lcd.backlight();

  pinMode(echoPin, INPUT); // Ultrasonic echoPin as an Input
  pinMode(trigPin, OUTPUT); // Ultrasonic trigPin as an Output

  pinMode(pump, OUTPUT);
  digitalWrite(pump, HIGH);
  pinMode(stopper, OUTPUT);
  digitalWrite(stopper, HIGH);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(buzzer, OUTPUT);
  pinMode(sensorAtas, INPUT);
  pinMode(sensorBawah, INPUT);
  pinMode(buttPin, INPUT_PULLUP);

  /*
    espSerial.begin(9600);//initialize serial for ESP8266 module
    WiFi.init(&espSerial); //initialize ESP8266 module


    //mengecek keberadaan shield
    if (WiFi.status() == WL_NO_SHIELD) {
      lcd.setCursor(0, 0);
      lcd.print("WiFi shield Eror");
      while (true);
    }

    //pin LED ready, jika terkoneksi ke mqtt server
    pinMode(ledServer, OUTPUT);
    digitalWrite(ledServer, LOW);

    //konfigurasi esp8266
    while (status != WL_CONNECTED) {
      lcd.setCursor(0, 0);
      lcd.print("Connecting to:");
      lcd.setCursor(0, 1);
      lcd.print(ssid);
      //connect to WPA/WPA2 network
      status = WiFi.begin(ssid, pass);
    }

    lcd.clear();
  */
  delay(1000);
}

//loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--loop--
void loop() {
  /*
    if (!client.connected())
    {
      if (sedangMengisi == 0) {
        digitalWrite(ledServer, LOW);
        lcd.setCursor(0, 0);
        lcd.print("-Device Offline-");
      }

      long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;

        if (reconnect()) {
          lastReconnectAttempt = 0;
          if (sedangMengisi == 0) {
            lcd.setCursor(0, 0);
            lcd.print("IP:");
            lcd.print(WiFi.localIP());
            lcd.print(" ");
          }
        }
      }
    } else {
      client.loop();
    }
  */
  sensorBak();
  pressButt();
  sensorUltrasonic();
  waterLevel();
  buzzerTone();
  sensorSuhu();

  delay(3000);
}

/*
  //call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--call--
  void callback(char* topic, byte* payload, unsigned int length) {

  // ini adalah algoritma untuk pump
  if (strcmp(topic, "minipro1/pump/com") == 0) {
    if (payload[0] == '0') {
      digitalWrite(pump, HIGH);
      delay(100);
      //      client.publish("minipro1/pump/state", "0");
    } else if (payload[0] == '1') {
      digitalWrite(pump, LOW);
      delay(100);
      //      client.publish("minipro1/pump/state", "1");
    }
  }
  }
*/

//ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--ultr--
void sensorUltrasonic() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;
  // we assume that the water tank heigh is 100cm
  water = 100 - distance;

  if (distance > 100 || distance < 0) {
    errorSensor = 1;
    
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Sensors Error...");
//    lcd.noBacklight();

    //turn off all relays
    digitalWrite(pump, HIGH);
    digitalWrite(stopper, HIGH);
  }
  else {
    errorSensor = 0;

    lcd.setCursor(13, 1);
    lcd.print(water);
    lcd.print("%");
    lcd.print(" ");

    /*
            long now = millis();
            if (now - lastMsg2 > 10000) {
              lastMsg2 = now;

              char charWater[50];
              dtostrf(water, 5, 2, charWater);

              client.publish("minipro1/ultrasonic/state", charWater);
            }
    */
    // ini untuk matikan pompa jika dinyalakan menggunakan saklar manual(konvensional)
    // jika air lebih 95cm nyalakan relay untuk stop pompa
    if (water > 95) {
      digitalWrite(stopper, LOW);
    }
    if (water <= 85) {
      digitalWrite(stopper, HIGH);
    }

    if (water <= 60 || overridePump == 1) {
      buttState = 1;
      if (bakKosong == LOW) { // jika bak tidak kosong boleh lakukan otomatis
        digitalWrite(pump, LOW);
        lcd.setCursor(11, 0);
        lcd.print("P:ON ");
        //      client.publish("minipro1/pump/state", "1");
      }
    } else if (water >= 90 || bakKosong == HIGH) { //jika bak kosong pompa mati
      buttState = 0;
      digitalWrite(pump, HIGH);
      lcd.setCursor(11, 0);
      lcd.print("P:OFF");
      //      client.publish("minipro1/pump/state", "0");
    }
  }
}


//lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--lvel--
void waterLevel() {

  //water tank 75%
  if (water > 75 && water <= 100 ) {
    digitalWrite(led4, HIGH);
  } else {
    digitalWrite(led4, LOW);
  }

  //water tank 50%
  if (water > 50 && water <= 75) {
    digitalWrite(led3, HIGH);
  } else {
    digitalWrite(led3, LOW);
  }

  //water tank 25%
  if (water > 25 && water <= 50) {
    digitalWrite(led2, HIGH);
  } else {
    digitalWrite(led2, LOW);
  }

  //water tank 5%
  if (water > 5 && water <= 25) {
    digitalWrite(led1, HIGH);
  } else {
    digitalWrite(led1, LOW);
  }


}
//leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--leak--
void sensorBak() {
  bakKosong = digitalRead(sensorBawah); // detect sensor bawah bak
  bakPenuh = digitalRead(sensorAtas);

  if (bakPenuh == LOW && water < 80) {
    overridePump = 1;
    tone(buzzer, 500); // Send 1KHz sound signal...
    delay(100);
    tone(buzzer, 1000); // Send 1KHz sound signal...
    delay(150);
    tone(buzzer, 500); // Send 1KHz sound signal...
    delay(100);
    tone(buzzer, 1000); // Send 1KHz sound signal...
    delay(150);
  } else {
    overridePump = 0;
    noTone(buzzer);     // Stop sound...
  }
  
  if (bakPenuh == LOW) {
    lcd.setCursor(0, 0);
    lcd.print("Bak Penuh ");

  }

  if (bakKosong == HIGH) {
    lcd.setCursor(0, 0);
    lcd.print("Bak Kosong");
  }

  if (bakKosong == LOW && bakPenuh == HIGH) {
    lcd.setCursor(0, 0);
    lcd.print("Bak Ready ");
  }

////activate for debugging
//if (bakKosong == HIGH){
//    lcd.setCursor(0, 0);
//    lcd.print("BH");
//}else{
//  
//    lcd.setCursor(0, 0);
//    lcd.print("BL");
//}
//if(bakPenuh == HIGH){
//  
//    lcd.setCursor(3, 0);
//    lcd.print("AH");
//}else{
//    lcd.setCursor(3, 0);
//    lcd.print("AL");
//}
}

//butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--butt--
void pressButt(){
  buttValue = digitalRead(buttPin);

  if (buttValue == LOW && buttState == 0) {//if pressed
    digitalWrite(pump, LOW);
    buttState = 1;
  } else if (buttValue == LOW && buttState == 1) {
    digitalWrite(pump, HIGH);
    buttState = 0;
  }
}

//tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--tone--
void buzzerTone() {
  if (water > 92) {
    tone(buzzer, 2000); // Send 2KHz sound signal...
    lcd.noBacklight();
    delay(100);
    noTone(buzzer);     // Stop sound...
    lcd.backlight();
    delay(200);
  } else if (water < 25) {
    tone(buzzer, 1000);
    delay(200);
    noTone(buzzer);
    delay(200);
  } else if (water > 100) {
    noTone(buzzer);
  } else {
    noTone(buzzer);
    lcd.backlight();
  }
}

//suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--suhu--
void sensorSuhu() {
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc = T - 273.15;
  //  Tf = (Tc * 9.0)/ 5.0 + 32.0;

  if (errorSensor == 0) {
    lcd.setCursor(0, 1);
    lcd.print("Toren ");
    lcd.print(Tc,1);
    lcd.setCursor(10, 1);
    lcd.print("\337C ");

  }

  /*
      long now = millis();
      if (now - lastMsg > 10000) {
        lastMsg = now;

        char charTc[50];
        dtostrf(Tc, 5, 2, charTc);

        client.publish("minipro1/temp/state", charTc);
      }
  */
}
