#include <GPS_5Hz.h>
#include <SoftwareSerial.h>

// Código de prueba para los módulos usando el driver MTK3329/MTK3339
//
// Este codigo solo hace eco a lo que sea que llegue de la unidad de GPS
//hacia el monitor serial, util para debuggear
//
// Probado y funciona bien con el Shield GPS de 5Hz usando el chip MTK33x9
//    ------> http://5hertz.com/index.php?main_page=product_info&cPath=26_31&products_id=899
// Compra uno hoy en nuestra tienda de electrónica y
// ayuda a mantener el Software y Hardware open source :)

SoftwareSerial mySerial(2, 3); //Constructor de puerto serial por software, en los pines asociados del shield
GPS_5Hz GPS(&mySerial);

// Pon GPSECHO a 'false' para apagar la generación de Eco de los datos del GPS a la consola Serial
// Ponlo a 'true' si quieres debuggear y escuchar las sentencias del GPS en la consola
#define GPSECHO  true

// Esta bandera lleva registro de si estas usando las interrupciones o no por default
boolean usingInterrupt = false;
void useInterrupt(boolean); // Prototipo de la función, necesario para Arduino 0023

void setup()  
{    
  // Conecta a 115200 para poder leer el GPS lo suficientemente rápido y
  // hacer eco sin perder caracteres
  Serial.begin(115200);
  Serial.println("GPS 5Hz Eco test!");

  // la velocidad de transmisión por default con el módulo es de 9600 bauds
  GPS.begin(9600);
  
  // Puedes ajustar estas líneas para elegir que sentencias emitirá el módulo:  
  // Descomenta esta linea para activar el RMC (mínimo recomendado) y GGA (información de fix) incluyendo altitud
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Descomenta esta linea para activar solamente la información mínima recomendada
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Descomenta esta linea para activar toda la información disponible- a 9600 bauds querrás leer a no más de 1Hz
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
  
  // Indica la velocidad de actualización  
  // Frecuencia de 1 Hz
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  // Frecuencia de 5 Hz - para una velocidad de 9600 bauds tendrás que marcar la salida a solo RMC o RMC+GGA para evitar perder datos
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  // Frecuencia de 10 Hz - para una velocidad de 9600 bauds tendrás que marcar la salida a solo RMC para evitar perder datos
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);

  // Solicita actualizaciones sobre el estado de la antena, comenta la línea para evitar esta información
  GPS.sendCommand(PGCMD_ANTENNA);
  
  // Lo bonito de este código es que puedes tener una interrupción del timer0 cada 1ms, 
  // y que lea la info del GPS por ti. Eso hace el código del Loop mucho mas sencillo!
  useInterrupt(true);
  
  delay(1000);
}

// La interrupción es llamada cada 1ms, busca alguna información nueva del GPS, y la almacena
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // Si quieres debuggear, es el momento de hacerlo!
  if (GPSECHO)
  // Escribir directo al registro UDR0 es mucho más rapido que un Serial.print, 
  //pero solo un caracter puede ser escrito a la vez. 
  if (c) UDR0 = c;  
}

void useInterrupt(boolean v) {
  if (v) {
    // El Timer0 ya está siendo usado por la función millis() - Interrumpiremos en algún punto intermedio y llamaremos la función de arriba
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // No llames a la función de COMPA más
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}


void loop()
{
   // No hace nada! Toda la lectura e impresión es hecha en la interrupción
}
