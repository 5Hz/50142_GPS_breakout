This code erases the LOCUS built-in datalogger storage

#include <GPS_5Hz.h>
#include <SoftwareSerial.h>

// Código de prueba para los módulos GPS de 5Hz usando el driver MTK3329/MTK3339
//
// Éste código borra la memoria del datalogger integrado LOCUS. 
// El datalogger se apaga al desconectar la energía, asi que debes tenerlo
// siempre encendido si quieres usarlo!
//
// Probado y funciona genial con el GPS Logger Shield de 5Hz usando el chip MTK33x9
// -------> http://5hertz.com/index.php?main_page=product_info&products_id=898
// Compra uno hoy en nuestra tienda de electrónica y ayuda a mantener
// el Software y Hardware open source :)

SoftwareSerial mySerial(2, 3); //Constructor de puerto Serial por software en los pines indicados
GPS_5Hz GPS(&mySerial); //Constructor de la clase GPS (comunicandose por el puerto previamente creado)

// Pon GPSECHO a 'false' para apagar la generación de Eco de los datos del GPS a la consola Serial
// Ponlo a 'true' si quieres debuggear y escuchar las sentencias del GPS en la consola
#define GPSECHO  false

// Esto mantiene un control sobre si usas o no interrupciones
boolean usingInterrupt = false;
void useInterrupt(boolean); // Prototipo de funcion, necesario para Arduino 0023

void setup()  
{
  // Conecta a 115200 para poder leer el GPS lo suficientemente rápido y
  // hacer eco sin perder caracteres
  Serial.begin(115200);
  Serial.println("5Hz GPS Erase FLASH test!");
  // 9600 bauds es el default para comunicarse con nuestro módulo GPS
  GPS.begin(9600);
  // Lo bonito de este código es que puedes tener una interrupción del timer0 cada 1ms, 
  // y que lea la info del GPS por ti. Eso hace el código del Loop mucho mas sencillo!
  useInterrupt(true);  
  
  //Solicita al GPS dejar de enviar información de cualquier sentencia NMEA
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);

  Serial.println("Este codigo BORRARA la informacion almacenada en la memoria FLASH - permanentemente!");
  Serial.print("Estas seguro de que quieres hacer esto? [Y/N]: ");
  while (Serial.read() != 'Y')   delay(10);
  Serial.println("\nBORRANDO! DESCONECTA TU ARDUINO ANTES DE 5 SEGUNDOS SI LO HICISTE POR ERROR!");
  delay(5000);
  GPS.sendCommand(PMTK_LOCUS_ERASE_FLASH); //Borra info de la flash
  Serial.println("Erased");
}



void loop()
{
  if (mySerial.available()) {
    char c = mySerial.read();
      if (c) UDR0 = c;  
  }
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
