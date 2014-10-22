#include <GPS_5Hz.h>
#include <SoftwareSerial.h>

// Código de prueba para los módulos GPS de 5Hz usando el driver MTK3329/MTK3339
//
// Éste código lee el estado del datalogger integrado LOCUS. 
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

// Comentado para que no necesariamente active el Logger, solo checa su estado actual
// Si quieres activarlo descomenta las siguientes líneas
//  while (true) {
//    Serial.print("Starting logging....");
//    if (GPS.LOCUS_StartLogger()) {
//      Serial.println(" STARTED!");
//      break;
//    } else {
//      Serial.println(" no response :(");
//    }
//  }
}



void loop()                     // run over and over again
{
  delay(1000);
   
  if (GPS.LOCUS_ReadStatus()) {
     Serial.print("\n\nLog #"); 
     Serial.print(GPS.LOCUS_serial, DEC);
    if (GPS.LOCUS_type == LOCUS_OVERLAP)
      Serial.print(", Overlap, ");
    else if (GPS.LOCUS_type == LOCUS_FULLSTOP)
      Serial.print(", Full Stop, Logging");
   
    if (GPS.LOCUS_mode & 0x1) Serial.print(" AlwaysLocate");
    if (GPS.LOCUS_mode & 0x2) Serial.print(" FixOnly");
    if (GPS.LOCUS_mode & 0x4) Serial.print(" Normal");
    if (GPS.LOCUS_mode & 0x8) Serial.print(" Interval");
    if (GPS.LOCUS_mode & 0x10) Serial.print(" Distance");
    if (GPS.LOCUS_mode & 0x20) Serial.print(" Speed");
    
    Serial.print(", Content "); Serial.print((int)GPS.LOCUS_config);
    Serial.print(", Interval "); Serial.print((int)GPS.LOCUS_interval);
    Serial.print(" sec, Distance "); Serial.print((int)GPS.LOCUS_distance);
    Serial.print(" m, Speed "); Serial.print((int)GPS.LOCUS_speed);
    Serial.print(" m/s, Status "); 
    if (GPS.LOCUS_status) 
      Serial.print("LOGGING, ");
    else 
      Serial.print("OFF, ");
    Serial.print((int)GPS.LOCUS_records); Serial.print(" Records, ");
    Serial.print((int)GPS.LOCUS_percent); Serial.print("% Used "); 

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
