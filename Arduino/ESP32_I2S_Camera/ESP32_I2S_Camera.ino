
#include "OV7670.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "BMP.h"
#include <Wire.h>
#include <ESP32Firebase.h>




//camera pins
const int SIOD = 21; // SDA
const int SIOC = 22; // SCL

const int VSYNC = 5;
const int HREF = 19;

const int XCLK = 18;
const int PCLK = 4;

const int D0 = 32;
const int D1 = 14;
const int D2 = 25;
const int D3 = 26;
const int D4 = 27;
const int D5 = 14;
const int D6 = 12;
const int D7 = 13;

//ultrasonic sensor pins

int TRIG_PIN  =16;
int ECHO_PIN = 17;

//LED 

int RED_LIGHT = 23;

#define ssid1       "internet-name"
#define password1    "password"


// IP address or domain of your .NET server
const char* serverUrl = "backend_url"; // server URL
OV7670* camera;
// Firebase konfiguracija
#define FIREBASE_HOST "firebase_host"
#define FIREBASE_AUTH "token"

// FirebaseData firebaseData;

unsigned char bmpHeader[BMP::headerSize];

Firebase firebase(FIREBASE_HOST);

void setup() 
{
  Serial.begin(115200);

  WiFi.begin(ssid1, password1);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  //camera setup
  camera = new OV7670(OV7670::Mode::QQQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);


  //ultrasonic sensor setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  //dioda
  pinMode(RED_LIGHT,OUTPUT);
  digitalWrite(RED_LIGHT,LOW);


}
void loop() {


  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

    // Read the echo pin, and calculate distance
  long duration = pulseIn(ECHO_PIN, HIGH); // Duration in microseconds
  // Calculate the distance in cm
  float distance = (duration * 0.0343) / 2;

  //Print the distance
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  if(distance < 15){
    camera->oneFrame();
    serve();
  }
  
}



void serve()
{
              HTTPClient http;

            http.begin(serverUrl);
            http.addHeader("Content-Type", "image/bmp");

            // Start HTTP POST request
            Serial.println("Sending POST request...");
            size_t frameSize = camera->xres * camera->yres * 2; // Veličina piksel podataka
            size_t totalSize = BMP::headerSize + frameSize;    // Ukupna veličina: header + piksel podaci

            // Kreirajte buffer koji će sadržavati header i piksele
            unsigned char* bmpData = new unsigned char[totalSize];

            // Kopirajte BMP header u buffer
            memcpy(bmpData, bmpHeader, BMP::headerSize);

            // Kopirajte piksel podatke u buffer
            memcpy(bmpData + BMP::headerSize, camera->frame, frameSize);
            digitalWrite(RED_LIGHT,HIGH);
            int httpResponseCode = http.sendRequest("POST", bmpData, totalSize);

              if (httpResponseCode > 0) {
            Serial.printf("Response code: %d\n", httpResponseCode);
            firebase.setInt("/isPhotoTaken", 1);
            String response = http.getString();
            Serial.println("Server response: " + response);
          } else {
            Serial.printf("Error in POST request: %s\n", http.errorToString(httpResponseCode).c_str());
          }
          firebase.setInt("/isPhotoTaken", 0);

          http.end();
         digitalWrite(RED_LIGHT,LOW);



}