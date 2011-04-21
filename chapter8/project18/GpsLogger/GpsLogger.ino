/*
 GPS Datalogger
 Language: Arduino
 */

#include <SD.h>
#include <SoftwareSerial.h>

// set up a software serial port for the GPS receiver
// using pins 2 and 3 (2 is the Arduino's RX, 3 is TX)
SoftwareSerial gpsPort(2, 3);

// On the Arduino Ethernet Shield, CS for the SD card is pin 4. 
// Note that even if it's not  used as the CS pin, the SPI hardware 
// CS pin (10 on most Arduino boards, 53 on the Mega) must be 
// left as an output or the SD library functions will not work.
const int chipSelect = 4;

const int sdErrorLed = 5;     // lights when there's an error with the SD card
const int sdWriteLed = 6;     // lights when writing to the SD card
boolean cardPresent = false;  // whether or not there is a card present

File dataFile;                // the file to write to on the SD card
String gpsData = "";          // String to gather data for writing to SD card
int lastReadLength = 0;       // length of the last batch of data
int reserveLength = 1023;     // memory reserved for the String holding the data

long lastSentenceTime = 0;    // last time a GPS senstence arrived
long delayTime = 250;         // how long to wait before starting an SD write

void setup() {
  Serial.begin(9600);          // hardware serial is only being used for debugging   
  gpsPort.begin(9600);         // the GPS receiver is attached to the soft serial

  Serial.print("Initializing SD card...");
  // make sure that the default SPI chip select pin 
  // is set to output:
  pinMode(10, OUTPUT);

  // initialize the SD card indicator LEDs:
  pinMode(sdErrorLed, OUTPUT);
  pinMode(sdWriteLed, OUTPUT);

  // reserve space for the incoming data string:
  gpsData.reserve(reserveLength);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    // if the card's not working, let the user know:
    digitalWrite(sdErrorLed, HIGH);
    Serial.println("Card failed, or not present");
    cardPresent = false;
  } 
  else {
    // if the card's working, initialize the data file:
    cardPresent = true;
    Serial.println("card initialized.");
    // if the file already exists, delete the previous version:
    if  (SD.exists("datalog.txt")) {
      // light the Write LED to let the user know 
      // that the card's being accessed:
      digitalWrite(sdWriteLed, HIGH);
      SD.remove("datalog.txt"); 
      digitalWrite(sdWriteLed, LOW);
    }
    // open the file for writing:
    dataFile = SD.open("datalog.txt", FILE_WRITE);
  }
}

void loop() {
  // while there's incoming GPS data, save it to a String: 
  while (gpsPort.available() ) {
    char inChar = gpsPort.read();
    gpsData = gpsData + inChar;
    lastReadLength++;
    if (inChar == '\n') {
      lastSentenceTime = millis();
    }
  }
  if (millis() - lastSentenceTime > delayTime) {
    if (gpsData.length() + lastReadLength > reserveLength) {
      if (cardPresent && dataFile) {
        // indicate that the card is being accessed:
        digitalWrite(sdWriteLed, HIGH);
        // write the current data to the card:
        dataFile.print(gpsData);
        // make sure the card saves the data:
        dataFile.flush();
        // print the data you wrote and clear the string:
        Serial.println(gpsData);
        gpsData = "";
        digitalWrite(sdWriteLed, LOW);
      }    // if the file isn't open, let the user know:
      else {
        digitalWrite(sdErrorLed, HIGH);
        Serial.println("error opening datalog.txt");
      } 
    }
  }

}

























