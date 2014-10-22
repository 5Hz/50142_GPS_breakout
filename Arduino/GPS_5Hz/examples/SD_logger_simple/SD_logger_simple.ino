#include <SD.h>
#include <GPS_5Hz.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);
GPS_5Hz GPS(&mySerial);

//Archivo donde almacenar datos del GPS
File myFile;

void setup()
{
  Serial.begin(115200);
  Serial.println("GPS 5Hz logging test!");
  GPS.begin(9600);
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Request updates on antenna status, comment out to keep quiet
  //GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  // Ask for firmware version
  //mySerial.println(PMTK_Q_RELEASE);
  
  //Inicializa pin CS para SD
  pinMode(10, OUTPUT);
  
  if (!SD.begin(10)) {
    Serial.println("error de inicializacion!");
    return;
  }
  Serial.println("inicializacion correcta.");
  
  myFile = SD.open("GPS_5Hz.txt", FILE_WRITE);
  if (!myFile) Serial.println("error abriendo GPS_5Hz.txt");
  myFile.close();
}

void loop()
{
  GPS.read();
  if (GPS.newNMEAreceived() == true && strstr(GPS.lastNMEA(), "$GPRMC"))
  {
    myFile = SD.open("GPS_5Hz.txt", FILE_WRITE);
    myFile.println(GPS.lastNMEA());
    myFile.close();
  }
}
