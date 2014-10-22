#include <GPS_5Hz.h>
#include <SoftwareSerial.h>

// Código de prueba para los módulos GPS de 5Hz usando el driver MTK3329/MTK3339
//
// Éste código solicita al módulo GPS la información almacenada en su FLASH y la imprime en el monitor serial
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
  Serial.println("GPS 5Hz dump logger data test!");
  
  // 9600 bauds es el default para comunicarse con nuestro módulo GPS
  GPS.begin(9600);
  
  //Desactiva toda transmision de datos para solo leer la información del Log interno del GPS
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
  
  // Limpia el buffer del puerto serial antes de comenzar.
  while (mySerial.available())
     mySerial.read();

  delay(1000);
  GPS.sendCommand(PMTK_LOCUS_DUMPLOG);
  Serial.println("----------------------------------------------------");
}


void loop()
{  
  if (mySerial.available()) {
    char c = mySerial.read();
    if (c) {
#ifdef UDR0
      UDR0 = c;  
#else
      Serial.print(c);
#endif
    }
  }
}

