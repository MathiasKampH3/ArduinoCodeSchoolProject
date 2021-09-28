#include <Servo.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include "DHT.h"
#include "arduino_secrets.h"

#define DHTPIN A1   	// what pin the DHT sensor is connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

int maxHum = 60;
int maxTemp = 40;

DHT dht(DHTPIN, DHTTYPE);

char ssid[] = SECRET_SSID;       
char pass[] = SECRET_PASS;   

int status = WL_IDLE_STATUS;
char server[] = "192.168.1.117"; // api
int serverPort = 5001;

WiFiClient client;
//Tools
Servo servo;
//pins
const char pirsensor = A5;
const char buttonPin = 0;
const char servopin = A2;
const char pinTemp = A1;
const char lightSensorPin = A6;
//lights
char bluel = 1;
char sensorLed = A4;

//status
int pirval = 0;
int pos = 90;
int ledState = LOW;
int servostatus = 0;
bool sensor = true;
int lightSensorValue = 0;

//listener

void setup() {
  //Initialize serial and wait for port to open:
  pinMode(buttonPin, INPUT);
  pinMode(pirsensor,INPUT);
  // sets interrupt on digital pin 0 where our button is
  attachInterrupt(digitalPinToInterrupt(buttonPin), UnlockWindows, RISING); // will trigger on falling so when we let go of the button
  pinMode(lightSensorPin, OUTPUT);
  servo.attach(servopin);
  pinMode(1,OUTPUT);
  Serial.begin(9600);
  servo.write(pos);
  ConnectToWifi();
  dht.begin();
}

void loop() {
  
  if (client.connect(server, serverPort)) {

    sensor = CheckSensor();

    if (sensor == true) {
       LockWindows(sensor);
    }

    PostRoomDataToApi();
  
    BlinkPostLed();    
  }
}

bool GetLightSensorValue(){
  lightSensorValue = analogRead (lightSensorPin);
  if( lightSensorValue <= 1000){

    return false;
  }

  else{
    return true;
  }
}

String ConvertBoolToString(bool val){

  if(val == true){

    Serial.println("converting true");

    return "true";
    
  }
  else{
    return "false";
  }
  
}

void LockWindows(bool sensor){
  if (sensor == true){
        if(servostatus == 0){
        servostatus = 1;
        servocontrole(1);
        Serial.println("servo: 1");
        }
    }
  }
 
void BlinkPostLed(){
  if (ledState == LOW){
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(bluel,ledState);
    
    delay(100);
    if (ledState == HIGH) {
     digitalWrite(bluel,LOW);
    }
    delay(200);
}



void PostRoomDataToApi(){
  
  client.println("GET /api/PostData/PostRoomData?roomName=B16&temperature=" + String(tempreader()) + "&windowLockState=" + String(ConvertBoolToString(CheckSensor()))+ "&lightSensorState=" + String(ConvertBoolToString(GetLightSensorValue())) +" HTTP/1.1");
  client.println("Host: 192.168.1.117:5001");
  client.println("Connection: close");
  client.println();  
}



void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void ConnectToWifi(){
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to WiFi");
  printWifiStatus();
}

  float tempreader(){
    float h = dht.readHumidity();
    float t = dht.readTemperature();
        if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else{
    return t;
  }
  }

void servocontrole(int status){
  if(status == 1){
  for (int i = 90; i >= 0; i--){
      servo.write(i);
      delay(20);
  }
  }else{
    for (int j = 0; j <= 90; j++){
        servo.write(j);
        delay(20);
     }
  }
}

bool CheckSensor(){
  pirval = digitalRead(pirsensor);
  if(pirval == HIGH){
    digitalWrite(sensorLed,HIGH);
      Serial.println("Motion detected!"); 
     return false;
  }
  else {
      digitalWrite(sensorLed, LOW);
        Serial.println("Motion stopped!");
        return true;           
  }
  delay(10);
}

void UnlockWindows(){
  if (servostatus == 1){
    servocontrole(0);
    servostatus = 0;
  }
  else{
      Serial.println("window is not locked");
    }
}

  
