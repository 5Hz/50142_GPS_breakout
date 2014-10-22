#include <GPS_5Hz.h>
#include <SoftwareSerial.h>

// Código de prueba para los módulos GPS de 5Hz usando el driver MTK3329/MTK3339
//
// Éste código enciende el datalogger integrado LOCUS. 
// El datalogger se apaga al desconectar la energía, asi que debes tenerlo
// siempre encendido si quieres usarlo!
//
// Probado y funciona genial con el GPS Logger Shield de 5Hz usando el chip MTK33x9
// -------> http://5hertz.com/index.php?main_page=product_info&products_id=898
// Compra uno hoy en nuestra tienda de electrónica y ayuda a mantener
// el Software y Hardware open source :)

SoftwareSerial mySerial(2, 3); //Constructor de puerto Serial por software en los pines indicados
GPS_5Hz GPS(&mySerial); //Constructor de la clase GPS (comunicandose por el puerto previamente creado)

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  false

// Esto mantiene un control sobre si usas o no interrupciones
boolean usingInterrupt = false;
void useInterrupt(boolean); // Prototipo de funcion, necesario para Arduino 0023

void setup()  
{
  // Conecta a 115200 para poder leer el GPS lo suficientemente rápido y
  // hacer eco sin perder caracteres
  Serial.begin(115200);
  Serial.println("5Hz GPS library logging start test!");

  // 9600 bauds es el default para comunicarse con nuestro módulo GPS
  GPS.begin(9600);
  
  // Puedes ajustar que sentencias va a emitir el módulo aqui:
  // Default es RMC + GGA
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Default es 1 Hz tasa de actualización
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  // Lo bonito de este código es que puedes tener una interrupción del timer0 cada 1ms, 
  // y que lea la info del GPS por ti. Eso hace el código del Loop mucho mas sencillo!
  useInterrupt(true);
  delay(500);
  Serial.print("\nSTARTING LOGGING....");
  if (GPS.LOCUS_StartLogger())
    Serial.println(" STARTED!");
  else
    Serial.println(" no response :(");
  delay(1000);
}

void loop() //Ciclo principal
{ 
   // No hace nada! Todas las lecturas e impresiones se hacen en la interrupción
}

/******************************************************************/

// La interrupción es llamada cada 1ms, busca alguna información nueva del GPS, y la almacena
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // Si quieres debuggear, es el momento de hacerlo!
  #ifdef UDR0
    if (GPSECHO)
    // Escribir directo al registro UDR0 es mucho más rapido que un Serial.print, 
    //pero solo un caracter puede ser escrito a la vez. 
      if (c) UDR0 = c; 
  #endif 
}

void useInterrupt(boolean v) {
  if (v) {
    // El Timer0 ya está siendo usado por la función millis() - Interrumpiremos en algún punto intermedio 
    // y llamaremos la función de arriba
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // No llames a la función de COMPA más
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}


