// #include <WiFi.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncElegantOTA.h>

// const char* ssid = "Redmi";
// const char* password = "omaromaromar";

// AsyncWebServer server(80);

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>



BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;





#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include<string.h>
// #define //pm1PIN 13//DSM501A input D6 on ESP8266
#define PM25PIN 12
byte buff[2];
// unsigned long duration//pm1;
unsigned long durationPM25;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
// unsigned long lowpulseoccupancy//pm1 = 0;
unsigned long lowpulseoccupancyPM25 = 0;

 
int i=0;


// If using software SPI (the default case):
#define OLED_CLK   15//27//15  //18   //D0
#define OLED_MOSI  2//5//2   //D1
#define OLED_RESET 4//18 //4   //res
#define OLED_DC     16//19//16
#define OLED_CS    17//21//17
#define BUZZER 14

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup()   {                
//  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
    Serial.begin(115200);
  Serial.println("Starting please wait 30s");
   display.print("this is the app");
  // pinMode(//pm1PIN,INPUT);
  pinMode(PM25PIN,INPUT);
  starttime = millis(); 
  pinMode(BUZZER,OUTPUT);
  // pinMode(iled, OUTPUT);
  // digitalWrite(iled, LOW);                                     //iled default closed
  
  Serial.begin(115200);                                         //send and receive at 115200 baud
  Serial.print("*********************************** WaveShare ***********************************\n");

  // Create the BLE Device
  BLEDevice::init("SSCS Challenge");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  if (deviceConnected) {

    String  str=String(0);

    
    pCharacteristic->setValue((char*)str.c_str());
    pCharacteristic->notify();
  }
  
  

}
float calculateConcentration(long lowpulseInMicroSeconds, long durationinSeconds){
  
  float ratio = (lowpulseInMicroSeconds/1000000.0)/30.0*100.0; //Calculate the ratio
  float concentration = 0.001915 * pow(ratio,2) + 0.09522 * ratio - 0.04884;//Calculate the mg/m3
  return concentration;
}

// float r//pm1= 0;

float rPM25=0;


void loop()
{
  // digitalWrite(BUZZER, HIGH);
  delay(5000);
  // duration//pm1 = pulseIn(//pm1PIN, LOW);
  durationPM25 = pulseIn(PM25PIN, LOW);
  
  // lowpulseoccupancy//pm1 += duration//pm1;
  lowpulseoccupancyPM25 += durationPM25;
  
  endtime = millis();
  if ((endtime-starttime) > sampletime_ms) //Only after 30s has passed we calcualte the ratio
  {
    /*
    ratio1 = (lowpulseoccupancy/1000000.0)/30.0*100.0; //Calculate the ratio
    Serial.print("ratio1: ");
    Serial.println(ratio1);
    
    concentration = 0.001915 * pow(ratio1,2) + 0.09522 * ratio1 - 0.04884;//Calculate the mg/m3
    */
    // float con//pm1 = calculateConcentration(lowpulseoccupancy//pm1,30);
    float conPM25 = calculateConcentration(lowpulseoccupancyPM25,30);
    // Serial.print("//pm1 ");
    // Serial.print(con//pm1);
    Serial.print("  PM25 ");
    Serial.println(conPM25);
    // lowpulseoccupancy//pm1 = 0;
    lowpulseoccupancyPM25 = 0;
    starttime = millis();
    // r//pm1= con//pm1;
    rPM25= conPM25;

    static unsigned long thisMicros = 0;
    static unsigned long lastMicros = 0;
    display.clearDisplay();
    display.setCursor(0,0);
    // display.print("//pm1: ");
    // display.print(r//pm1);
    display.print("ug/m3");
    display.print("\n");
    display.print("  PM25: ");
    display.print("ug/m3");
    display.print(rPM25);
    display.display();
    lastMicros = thisMicros;
    thisMicros = micros();
    
      // notify changed value
    if (deviceConnected) {
    
        String  str=String(rPM25);

        
        pCharacteristic->setValue((char*)str.c_str());
        pCharacteristic->notify();

        }
        
    }
    
     if (rPM25 > 12.0)
    {
    
      display.clearDisplay();
      display.setCursor(0,0);
      display.print(" GOOD ");
      delay(300);
      }


     if (rPM25 > 12.0 && rPM25<35.4)
    {
      for (int i =0;i<5;i++)
      {
      display.clearDisplay();
      display.setCursor(0,0);
      display.print(" Moderate ");
      delay(300);
      }
    }
    
         if (rPM25 >35.4 && rPM25<55.4)
    {
      for (int i =0;i<3;i++)
      {
      digitalWrite(BUZZER, HIGH);
      display.clearDisplay();
      display.setCursor(0,0);
      display.print(" Unhealthy  ");
      display.print(" for sensative group !!");
      delay(300);
      digitalWrite(BUZZER, LOW);

      }
    }
     if (rPM25 >55.4 && rPM25<150.4)
    {
      for (int i =0;i<5;i++)
      {
      digitalWrite(BUZZER, HIGH);
      display.clearDisplay();
      display.setCursor(0,0);
      display.print(" Unhealthy !! ");
      delay(200);
      digitalWrite(BUZZER, LOW);

      }
    }

     if (rPM25 > 150.4 && rPM25<250.4)
    {
      for (int i =0;i<6;i++)
      {
      digitalWrite(BUZZER, HIGH);
      display.clearDisplay();
      display.setCursor(0,0);
      display.print(" Very Unhealthy ");
      delay(200);
      digitalWrite(BUZZER, LOW);

      }
    }
     if (rPM25 > 250.4)
    {
      for (int i =0;i<10;i++)
      {
      digitalWrite(BUZZER, HIGH);
      display.clearDisplay();
      display.setCursor(0,0);
      display.print(" Hazadeous !!! ");
      delay(100);
      digitalWrite(BUZZER, LOW);

      }
  } 

}

  
