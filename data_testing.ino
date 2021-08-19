#include <ESP8266WiFi.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Wire.h> 
#include <LCD.h>
#include <SPI.h>;
//#include <SimpleTimer.h>

#include <WiFiClientSecure.h>

fgfg

#define DHTPIN 14     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define I2C_ADDR 0x27
#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
WidgetLCD lcd1(V3);
SimpleTimer timer;
 
char auth[] = "NZM8Zs8_GfqApxtDdkC06JtFicdEveP9"; //Enter Authentication code sent by Blynk
char ssid[] = "Hotspot@PUTRA-2.4G"; //Enter WIFI Name
char pass[] = ""; //Enter WIFI Password

int flame_sensor = D7; //Pin for IR Flame sensor
int flame_detected = HIGH;

int chk;
//float hum;  //Stores humidity value
//float temp; //Stores temperature value

int buzzer = D6; //buzzer
int redLED = D4; //redLED
int greenLED = D8; //GreenLED

int sensor = A0; //Pin for MQ2 sensor
int sensor_limit = 520; //Threshold limit of MQ2
int tem_limit = 50; //Threshold limit for DHT22

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//--------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String GAS_ID = "AKfycbyEowDfcgSde38Kl3YRdpmEZpvdoFKMrgqkJcolTNu4uBkPk4Pz6YadQsWFj71hL5YV2A"; // spreadsheet script ID
 
void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);  

  Serial.println("");
  Serial.println("WiFi connected");  //show wifi is connected
  Serial.println("IP address: "); //show the IP address
  Serial.println(WiFi.localIP()); //IP address of the shield

  client.setInsecure();

  lcd.begin(16,2); //begin LCD display
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0); //goto first column (column 0) and first line (Line 0)
  lcd.print("Temp C = "); //Print at cursor Location
  lcd.setCursor(0,1); //goto first column (column 0) and first line (Line 0)
  lcd.print("Humi % = "); //Print at cursor Location

  pinMode(sensor, INPUT); 
  pinMode(flame_sensor, INPUT);
  pinMode(redLED,OUTPUT);
  pinMode(greenLED,OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  dht.begin(); //start dht
  timer.setInterval(15000, sendUptime); //ttime interval of 15 seconds
  

}

void sendUptime(){

  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  int sensor_value = analogRead(sensor);

  flame_detected = digitalRead(flame_sensor);

  String Temp = "Temperature : " + String(temp) + " °C  ";
  String Humi = "Humidity : " + String(hum) + " %";
  String Gas = "Gas Value : " + String(sensor_value) + " ppm";
  
  Serial.print(Temp);
  Serial.print(Humi);
  Serial.println();

 
  Serial.println(Gas); //print gas level

  Serial.println(flame_detected); //print detection status

  Blynk.virtualWrite(V10, temp); //write to blynk application
  Blynk.virtualWrite(V9, hum); //
  
  Blynk.virtualWrite(V2, sensor_value); //
  
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" ");
  lcd.print("C");
  
  lcd.setCursor(0,1);
  lcd.print("Humi: ");
  lcd.print(hum);
  lcd.print(" %");
  
   if (temp > tem_limit)
  {
    digitalWrite(buzzer, LOW); //buzzer high, redLED high, send alert
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    Blynk.notify("Alert: High Temperature");    
    delay(2000);
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);

  }
  else
  {
    digitalWrite(redLED, LOW); //buzzer low, greenLED high
    digitalWrite(greenLED, HIGH);
  }
  
  if (sensor_value > sensor_limit)
  {
    digitalWrite(buzzer, LOW); // buzzer high, redLED high, send alert
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    Blynk.notify("Alert: Gas Leakage Detected");    
    delay(2000);
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);

  }
  else
  {
    digitalWrite(redLED, LOW); //buzzer low, greenLED high
    digitalWrite(greenLED, HIGH);
  }

  
  if (flame_detected == LOW)
  {
    digitalWrite(buzzer, LOW); //buzzer high, redLED high, send alert
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    Serial.println("Flame detected...! take action immediately.");

    lcd1.clear(); //Use it to clear the LCD Widget
    lcd1.print(4, 0, "Detected!"); //display in blynk dashboard
    Blynk.notify("Alert: Fire Detected!"); //notification
    delay(2000);
    digitalWrite(buzzer, HIGH);
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    delay(2000);

  }
  else
  {
    Serial.println("No flame detected.");
    digitalWrite(buzzer, HIGH);
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    lcd1.clear(); //Use it to clear the LCD Widget
    lcd1.print(4, 0, "Not Detected");
  }
  
  sendData(temp, hum, sensor_value, flame_detected); //--> Calls the sendData Subroutine
 // delay(30000);
  //Blynk.run(); // Initiates Blynk
}
 
void loop()
{
  Blynk.run(); //loopback 
  timer.run();

}

void sendData(float tem, float humi, int gas, int flame) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_temperature =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(humi); 
  String string_gas =  String(gas); 
  String string_flame =  String(flame); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + "&gas=" + string_gas + "&flame=" + string_flame;;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
  
  //Blynk.run(); // Initiates Blynk
} 
