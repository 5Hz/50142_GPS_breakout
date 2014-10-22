// Código de prueba para el shield GPS / GPS logger de 5Hz 
// usando el driver MTK33x9
//
// Éste código escucha al módulo GPS en una interrupción, 
// lo cual dota al programa de más libertad, es solo cuestión de
// analizar la información cuando una sentencia completa NMEA
// esté disponible y acceder a esta información cuando la requieras.
//
// Código probado y funcionando con el GPS Shield de 5Hz
// usando el chip MTK33x9
//    ------> http://5hertz.com/index.php?main_page=product_info&cPath=26_31&products_id=899
// Compra uno hoy en nuestra tienda de electrónica y 
// ayuda a mabtener el Hardware y Software open source :)

#include <GPS_5Hz.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); //Constructor de puerto Serial por software, en los pines 2, 3

GPS_5Hz GPS(&mySerial); //Constructor de la clase GPS, comunicandose por el puerto creado previamente

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
  Serial.println("5Hz GPS library basic test!");

  // 9600 bauds es el default para comunicarse con nuestro módulo GPS
  GPS.begin(9600);
  
  // Descomenta esta linea para activar el RMC (mínimo recomendado) y GGA (información de fix) incluyendo altitud
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Descomenta esta linea para activar solamente la información mínima recomendada
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Para analizar información, no sugerimos usar nada mas que simple RMC o RMC+GGA
  // ya que más sentencias pueden requerir mucho tiempo para analizar y nuestro 
  // analizador de cualquier manera no toma en cuenta otras sentencias por el momento.
  
  // Indica la velocidad de actualización
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // frecuencia de actualización de 1 o 5Hz
  // Para analizar la información, esta tasa de refresco funciona bien y deja un margen de tiempo
  // para analizar toda la información e imprimirla, no sugerimos una velocidad mayor a 1Hz utilizando Arduino

  // Solicita actualizaciones sobre el estado de la antena, comentar para ahorrar tiempo
  GPS.sendCommand(PGCMD_ANTENNA);

  // Lo bonito de este código es que puedes tener una interrupción del timer0 cada 1ms, 
  // y que lea la info del GPS por ti. Eso hace el código del Loop mucho mas sencillo!
  useInterrupt(true);

  delay(1000);
  // Pregunta por la version del firmware
  mySerial.println(PMTK_Q_RELEASE);
}

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

uint32_t timer = millis();
void loop()                     // run over and over again
{
  // En caso de no estar usando la interrupción anterior, 
  // deberás leer la información "a mano" del GPS, no recomendable (Podrías perder información)
  if (! usingInterrupt) {
    // Lee la información del GPS en el Main loop
    char c = GPS.read();
    // Si quieres debuggear, es el momento de hacerlo!
    if (GPSECHO)
      if (c) Serial.print(c);
  }
  
  // Si una sentencia es recibida, podemos hacer checksum y analizarla...
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // Verifica que la información sea correcta, esto pone la bandera newNMEAreceived() en false
      return;  // Podemos fallar al leer una oración, en cuyo caso solo esperamos a la siguiente
  }

  // si la variable timer supera el valor de millis, solo lo reseteamos
  if (timer > millis())  timer = millis();

  // aproximadamente cada 2 ms, imprimimos el estado actual
  if (millis() - timer > 2000) { 
    timer = millis(); // resetea el timer
    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }
}
