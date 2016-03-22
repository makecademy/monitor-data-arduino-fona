// Based on the FONA example code by Adafruit

// Libraries
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>
#include "DHT.h"

// DHT sensor
#define DHTPIN 7 
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Pins
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// Buffer
char replybuffer[255];

// Instances
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Fona instance
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
uint8_t type;

// Thing name
String yourThing = "8g62og";

void setup() {

  // Initi serial
  while (!Serial);
  Serial.begin(115200);
  Serial.println(F("FONA reading SMS"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default: 
      Serial.println(F("???")); break;
  }
  
  // Print module IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  // Setup GPRS settings
  fona.setGPRSNetworkSettings(F("internet"));
  //fona.setGPRSNetworkSettings(F("your_APN"), F("your_username"), F("your_password"));

  // Init DHT
  dht.begin();

  // Wait
  delay(1000);

  // Turn GPRS off & on again
  if (!fona.enableGPRS(false))
    Serial.println(F("Failed to turn off"));

  delay(1000);
 
  if (!fona.enableGPRS(true))
    Serial.println(F("Failed to turn on"));

  delay(1000);

}
void loop() {

  // Measure data
  float temperature = dht.readTemperature(); 
  float humidity = dht.readHumidity();

  // Log
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  // Prepare request
  uint16_t statuscode;
  int16_t length;
  String url = "http://dweet.io/dweet/for/";
  url += yourThing;
  url += "?temperature=";
  url += String(temperature);
  url += "&humidity=";
  url += String(humidity);
  char buf[80];
  url.toCharArray(buf, url.length());

  Serial.print("Request: ");
  Serial.println(buf);
  
  // Send location to Dweet.io
  if (!fona.HTTP_GET_start(buf, &statuscode, (uint16_t *)&length)) {
    Serial.println("Failed!");
  }
  while (length > 0) {
    while (fona.available()) {
      char c = fona.read();
           
      // Serial.write is too slow, we'll write directly to Serial register!
      #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
        loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
        UDR0 = c;
      #else
        Serial.write(c);
      #endif
      length--;
    }
  }
  fona.HTTP_GET_end();

  // Wait
  delay(60 * 1000);
  
}

