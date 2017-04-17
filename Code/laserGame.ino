#include <Servo.h>
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266Influxdb.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards
const char* ssid = "yourssid";
const char* password = "yourpass";

const char* INFLUXDB_HOST = "xxx.xxx.xxx.xxx";
const uint16_t INFLUXDB_PORT = 8086;
const char* DATABASE = "dbname";
const char* DB_USER = "dbuser";
const char* DB_PASSWORD = "dbpassword";
Influxdb influxdb(INFLUXDB_HOST, INFLUXDB_PORT);
 
int ledPin = D5;
ESP8266WiFiMulti WiFiMulti;

void connectWifi() {
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void setup()
{
  Serial.begin(9600);
  connectWifi();
  influxdb.opendb(DATABASE, DB_USER, DB_PASSWORD);
  Serial.println("Database connected");
  myservo.attach(2);  // attaches the servo on GPIO2 to the servo object
  Serial.println("Servo connected");

}

void loop()
{
  Serial.println("#############");
  Serial.println("Touch to start");
  Serial.println("#############");
  while(!targetIsTouched()){
    delay(10);
  }
  DB_RESPONSE dbresponse;
  int errors = 0;
  int touched = 0;
  int touchedWithNoErrors = 0;
  makeServoDown(myservo);
  delay(100);
  while (errors < 3) {
    makeServoUp(myservo);
    bool win = testLigth3sec(errors+touched);
    if (win) {
      Serial.println("Touché !");
      touched++;
      if (errors == 0) {
        touchedWithNoErrors++;
      }
    } else {
      errors++;
      Serial.println("Raté !");
    }
    makeServoDown(myservo);
    Serial.print("Cibles touchées : ");
    Serial.println(touched);
    Serial.print("Cibles ratées : ");
    Serial.println(errors);
    delay(1000);
  }
  Serial.print("Tu as perdu, tu as touché ");
  Serial.print(touched);
  Serial.println(" cibles avant de perdre");

  Serial.println("Writing data to host " + String(INFLUXDB_HOST) + ":" + INFLUXDB_PORT + "'s database=" + DATABASE);

  // Writing data using FIELD object
  FIELD dataObj("game"); // Create field object with measurment name=analog_read
  dataObj.addTag("valueType", "total"); // Add method tag
  dataObj.addField("value", touched); // Add value field
 
  Serial.println(influxdb.write(dataObj) == DB_SUCCESS ? "Writing sucess" : "Writing failed");

  FIELD dataObj2("game"); // Create field object with measurment name=analog_read
  dataObj2.addTag("valueType", "noErrors"); // Add method tag
  dataObj2.addField("value", touchedWithNoErrors); // Add value field
 
  Serial.println(influxdb.write(dataObj2) == DB_SUCCESS ? "Writing sucess" : "Writing failed");
}

void makeServoUp(Servo myservo) {
  int pos;
  for (pos = 0; pos <= 75; pos += 1) // goes from 0 degrees to 180 degrees
  { // in steps of 1 degree
    myservo.write(pos);               // tell servo to go to position in variable 'pos'
    delay(15);                        // waits 15ms for the servo to reach the position
  }
}

void makeServoDown(Servo myservo) {
  int pos;
  for (pos = myservo.read(); pos >= 0; pos -= 1) // goes from 180 degrees to 0 degrees
  {
    myservo.write(pos);               // tell servo to go to position in variable 'pos'
    delay(15);                        // waits 15ms for the servo to reach the position
  }
}

bool testLigth3sec(int nbTour) {
  float i;
  for (i = 0; i <= 1000.0 + (2000.0/nbTour); i++) {
    if (targetIsTouched()) {
      return true;
    }
    delay(1);
  }
  return false;
}

bool targetIsTouched() {
  int sensorValue = analogRead(A0);
  if(sensorValue > 300){
    return true;
  }
  return false;
}
