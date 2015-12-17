#include <ArduinoJson.h>
#include <DHT.h>
#include <Bridge.h>
#include <HttpClient.h>
#include <QueueList.h>
#include <FileIO.h>
#include "debug.h"
#include "constants.h"
#include "PubNubMessageChecker.h"

void sendDataToDb(double water = 0);

DHT dht(DHTPIN, DHTTYPE);
PubNubMessageChecker recieveMessage;
String userObjectId = "";
String plantObjectId;
double waterAmount;
String harvest;
int moistureLimit;
String username;

void setup() { 
  PRINT_LN("starting"); 
//  delay(60000);  
  SERIAL_BEGIN();  
  pinMode(PUMP_RELAY_PIN, OUTPUT);  
  Bridge.begin();
  PRINT_LN("bridge started");
  FileSystem.begin();
  PRINT_LN("file system started");  
  SERIAL_FLUSH();
  restoreSession();
}

void loop() {
  PRINT_LN("CHECKING");
  PRINT_LN(username);
  SERIAL_FLUSH();
  checkForMessages();
}

void checkForMessages() {
  String message = recieveMessage(username);
  PRINT_LN("Message: " + message);
  SERIAL_FLUSH();
  if (message.equals(SEND_SENSOR_MESSAGE)) {    
    sendDataToPage();
  } else if (message.equals(SEND_SENSOR_DB)) {
    if (analogRead(MOISTURE_PIN) < moistureLimit) {
      waterPlants();
      sendDataToDb(waterAmount);
    }
    else {
      sendDataToDb();
    }
  }
  else if (message.equals(WATER_MESSAGE)) {
    waterPlants();
    sendDataToDb(waterAmount);
  }
  else if (message.equals(UPDATE_MESSAGE)) {
    restoreSession();
  }
}

void sendDataToPage() {
  HttpClient client;
  String temp, humidity, moisture, light;
  readData(temp, humidity, moisture, light);
  PRINT_LN("temp: " + temp);
  SERIAL_FLUSH();
  String url = "http://pubsub.pubnub.com/publish/" + String(PUBNUB_PUBLISH_KEY) + "/" + String(PUBNUB_SUBSCRIBE_KEY) + "/0/" + username + String(WEBSITE_CHANNEL) + "/0/\"";
  url = String(url + temp + "," + humidity + "," + moisture + "," + light + "\"");
  PRINT_LN("URL:" + url);
  SERIAL_FLUSH();
  client.getAsynchronously(url);
  while (!client.ready()) {}
  char c;
  while (client.available()) {
    c = client.read();
  }
}

void sendDataToDb(double water) {
  String temp, humidity, moisture, light;
  readData(temp, humidity, moisture, light);

  Process dataScript;
  dataScript.begin("python");
  dataScript.addParameter("/mnt/sda1/sendSensorData.py");
  dataScript.addParameter(temp);
  dataScript.addParameter(humidity);
  dataScript.addParameter(moisture);
  dataScript.addParameter(light);
  dataScript.addParameter(userObjectId);
  dataScript.addParameter(plantObjectId);
  dataScript.addParameter(String(water, 4));
  dataScript.addParameter(harvest);
  dataScript.run();

  while (dataScript.available() > 0) {
    char c = dataScript.read();
    //    PRINT(c);
  }
}

void readData(String & temp, String & humidity, String & moisture, String & light) {
  temp = String(dht.readTemperature(), 2);
  humidity = String(dht.readHumidity(), 2);
  moisture = String(analogRead(MOISTURE_PIN));
  light = String(analogRead(LIGHT_PIN));
}

String readFile(const char * fileName){
  PRINT_LN("read File");
  PRINT_LN(fileName);
  File f = FileSystem.open("/mnt/sda1/config.json", FILE_READ);
  String contents;
  while (f.available() > 0) {
    char c = f.read();
    PRINT(c);
    contents += c;    
  }
  PRINT_LN(contents);
  SERIAL_FLUSH();
  return contents;
}

String readSession(){  
  PRINT_LN("readSession");
  SERIAL_FLUSH();
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& session = jsonBuffer.parseObject(readFile("/mnt/sda1/config.json"));
  return session["sessionToken"];
}

void restoreSession() {  
  PRINT_LN("restore Session");
  Process restoreSessionScript;
  restoreSessionScript.begin("python");  
  restoreSessionScript.addParameter("/mnt/sda1/restoreSession.py");
  String sessionToken = readSession();
  restoreSessionScript.addParameter(sessionToken);
  restoreSessionScript.run();
    
  char next_char;
  String json = "";
  while (restoreSessionScript.available()) {
    next_char = restoreSessionScript.read();
    if (String(next_char) == '\0') {
      break;
    } else {
      PRINT(next_char);
      json += next_char;
    }
  }
  PRINT_LN(json);
  StaticJsonBuffer<200> jsonBuffer;
  JsonArray& userInfo = jsonBuffer.parseArray(json);
  userObjectId = userInfo[0].asString();
  plantObjectId = userInfo[1].asString();
  waterAmount = userInfo[2];
  harvest = userInfo[3].asString();
  moistureLimit = userInfo[4];
  username = userInfo[5].asString();
  PRINT_LN(username);
  SERIAL_FLUSH();
}

void waterPlants() {
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  PRINT_LN("water amount: " + String(waterAmount, 4));
  PRINT_LN("water time: " + String((waterAmount / FLOW_RATE), 4));
  delay((waterAmount / FLOW_RATE) * 1000);  //multiply by 1000 to convert to miliseconds
  digitalWrite(PUMP_RELAY_PIN, LOW);
}

