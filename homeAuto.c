#define BLYNK_TEMPLATE_ID "TMPLvgxtaLm3"
#define BLYNK_DEVICE_NAME "LightDetection"
#define BLYNK_AUTH_TOKEN "VzByhPwh8TRXXs-GUp68FW5TP-XH4ZTw"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include "heltec.h"
#include <HTTPClient.h>
#include <stdio.h>
#include <stdlib.h>

#define DHT_SENSOR_PIN  27
#define DHT_SENSOR_TYPE DHT22 
WidgetLED led(V5);
 
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE); 
int sound_sensor = 26;
int light_sensor = 37;

char auth[] = BLYNK_AUTH_TOKEN;

const char * ssid = "Mandu_Thagudam#87";

const char * pass = "Village87";

#define BLYNK_GREEN     "#23C48E"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
int mode=0;
float currentTemperature=0;
float currentHumidity=0;
int currentLight = 0;
int currentSound = 0;
int size=0;

BlynkTimer timer;

struct node {
    float temperature,humidity;
    int light,sound;
    struct node * next;
};

struct node * front = NULL;
struct node * rear = NULL;

// Enqueue() operation on a queue
void enqueue(float temperature, float humidity, int light, int sound) {
    if(size>9)
        {
            dequeue();
            size--;
            
        }
        
    struct node * ptr;
    ptr = (struct node * ) malloc(sizeof(struct node));
    ptr-> temperature = temperature;
    ptr-> humidity = humidity;
    ptr-> light = light;
    ptr-> sound = sound;
    ptr-> next = NULL;
    if ((front == NULL) && (rear == NULL)) {
        front = rear = ptr;
    } else {
        rear-> next = ptr;
        rear = ptr;
    }
    size++;
    
}

// Dequeue() operation on a queue
void dequeue() {
    if (front == NULL) {
        Serial.println("Underflow");
        return;
    } else {
        front = front-> next;
    }
}

// Display all elements of the queue
void displayData() {
    struct node * temp;
    if ((front == NULL) && (rear == NULL)) {
        Serial.println("Queue is Empty");
    } else {
        Serial.println("The queue is");
        temp = front;
        while (temp) {
            float temperature =temp-> temperature;
            float humidity =temp-> humidity;
            int light =temp-> light;
            int sound =temp-> sound;
            Serial.print(temperature);
            Serial.print(" , ");
            Serial.print(humidity);
            Serial.print(" , ");
            Serial.print(light);
            Serial.print(" , ");
            Serial.print(sound);
            Serial.print(" , ");
            Serial.print(mode);
            Serial.print("-->");
            temp = temp-> next;
        }
        Serial.println("NULL");
    }
}

bool check()
{

  struct node * old = front;
  struct node * latest = rear;

  float t1=old-> temperature;
  float t2=latest-> temperature;
  float tempDifferenec=t2-t1;

  t1=old-> humidity;
  t2=latest-> humidity;
  float humiDifference=t1-t2;

  int t3=old-> light;
  int t4=latest-> light;
  int lightDifference=t4-t3;

  t3=old-> sound;
  t4=latest-> sound;
  int soundDifferenec=t4-t3;

  if(tempDifferenec > 4 || humiDifference >2.5 || lightDifference > 400 || soundDifferenec > 300)
    return false;

  return true;

}


BLYNK_WRITE(V4)
{
  mode=param.asInt();
  if(mode==0)
  {
    led.setColor(BLYNK_GREEN);
  }
  else
  {
   
    if(check())
      led.setColor(BLYNK_YELLOW);
    else
      {
        led.setColor(BLYNK_RED);
        Blynk.logEvent("warningowner");
      }
      
  }
}

void displayReadingsOnOled() {
 
 String temperatureDisplay = "Temperature: " + (String)currentTemperature;
 String humidityDisplay = "Humidity: " + (String)currentHumidity;
 String lightDisplay = "Light: " + (String)currentLight;
 String soundDisplay = "Sound: " + (String)currentSound;
 // Clear the OLED screen
 Heltec.display->clear();
 // Prepare to display temperature
 Heltec.display->drawString(0, 0, temperatureDisplay);
 // Prepare to display humidity
 Heltec.display->drawString(0, 12, humidityDisplay);
 // Prepare to display Light
 Heltec.display->drawString(0, 24, lightDisplay);
 // Prepare to display Sound
 Heltec.display->drawString(0, 36, soundDisplay);

 // Display the readings
 Heltec.display->display();
}

void sendSensor()
{
  // Request temperature to all devices on the data line
  currentTemperature = dht_sensor.readTemperature(true); 
  currentHumidity = dht_sensor.readHumidity(); 
  currentLight = analogRead(light_sensor);
  currentSound = analogRead(sound_sensor);

  while( isnan(currentTemperature) || isnan(currentHumidity) || isnan(currentLight) || isnan(currentSound))
    continue;

  Serial.print("Temperature : ");
  Serial.print(currentTemperature);

  Serial.print(", Humidity : ");
  Serial.print(currentHumidity);

  Serial.print(", Light : ");
  Serial.print(currentLight);

  Serial.print(", Sound : ");
  Serial.print(currentSound);

  Serial.print(", Switch : ");
  Serial.println(mode);

  enqueue(currentTemperature,currentHumidity,currentLight,currentSound);
  displayData();
 

  delay(1000);
  displayReadingsOnOled();

  Blynk.virtualWrite(V1, currentTemperature);
  Blynk.virtualWrite(V2, currentHumidity);
  Blynk.virtualWrite(V0, currentLight);
  Blynk.virtualWrite(V3, currentSound);
  
  delay(2000);

}
void setup()
{   
  
  Serial.begin(9600);
  dht_sensor.begin();  
  
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  Heltec.begin(true /DisplayEnable Enable/, false /LoRa Enable/, false /Serial Enable/);

  Blynk.begin(auth, ssid, pass);
  Blynk.virtualWrite(V4, mode);
  led.setColor(BLYNK_GREEN);
  led.on();
  timer.setInterval(100L, sendSensor);
 
}

void loop()
{
  Blynk.run();
  timer.run();
}