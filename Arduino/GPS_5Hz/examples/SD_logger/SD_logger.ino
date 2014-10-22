#include <GPS_5Hz.h>
#include <SoftwareSerial.h>
#include <SD.h>
/*
Ejemplo de logger de Ladyada modificado por Rafael Almaguer para funcionar
con la libreria GPS_5Hz
Este codigo te muestra como escuchar al módulo GPS y guardar la información
leída en una memoria microSD para poder acceder a los datos cuando los necesites.

Probado y funciona genial con el GPS Logger Shield de 5Hz usando el chip MTK33x9
-------> http://5hertz.com/index.php?main_page=product_info&products_id=898
Compra uno hoy en nuestra tienda de electrónica y ayuda a mantener
el Software y Hardware open source :)
*/

SoftwareSerial mySerial(2, 3); //Constructor de puerto Serial por software en los pines indicados
GPS_5Hz GPS(&mySerial); //Constructor de la clase GPS (comunicandose por el puerto previamente creado)

// Pon GPSECHO a 'false' para apagar la generación de Eco de los datos del GPS a la consola Serial
// Ponlo a 'true' si quieres debuggear y escuchar las sentencias del GPS en la consola
#define GPSECHO  true
/* Poner a 'true' para solamente loggear datos a la SD cuando el GPS tiene un "fix" (Encontró su ubicación)
para debuggear, mantenlo como 'false'*/
#define LOG_FIXONLY false  

// Pines utilizados
#define chipSelect 10
#define ledPin 13

File logfile;

// Parpadea un mensaje de error
void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}

void setup() {
  // Conecta a 115200 para poder leer el GPS lo suficientemente rápido y
  // hacer eco sin perder caracteres
  Serial.begin(115200);
  Serial.println("\r\nUltimate GPSlogger Shield");
  
  pinMode(ledPin, OUTPUT);
  
  // Asegurate de que el pin de selección por default esta habilitado como salida, 
  // aunque no lo uses:
  pinMode(10, OUTPUT);
  
  // Mira si hay alguna tarjeta SD conectada y que deba ser inicializada:d:
  if (!SD.begin(chipSelect)) {      // if you're using an UNO, you can use this line instead
    Serial.println("Card init. failed!");
    error(2);
  }
  char filename[15];
  strcpy(filename, "GPSLOG00.TXT"); //Nombre base para los archivos generados
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;
    //Crea el archivo si no existe, no abre uno existente
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  //Verifica que pueda abrir el archivo en modo escritura
  logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); Serial.println(filename);
    error(3);
  }
  Serial.print("Writing to "); Serial.println(filename);
  
  // conecta con el GPS a la velocidad deseada
  GPS.begin(9600);

  // Descomenta esta linea para activar el RMC (mínimo recomendado) y GGA (información de fix) incluyendo altitud
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Descomenta esta linea para activar solamente la información mínima recomendada
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // Para hacer data logging, no es recomendable usar nada más allá de solo RMC o RMC+GGA
  // para mantener los archivos de log de un tamaño razonable
  // Indica la velocidad de actualización
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // frecuencia de actualización de 1 o 5Hz

  // Si el firmware lo permite, desactiva la información sobre el estado de la antena para ahorrar espacio:
  GPS.sendCommand(PGCMD_NOANTENNA);
  
  Serial.println("Ready!");
}

void loop() {
  char c = GPS.read(); //Se debe leer lo más pronto posible para evitar perder información
  if (GPSECHO) //Imprime dato si la constante GPSECHO es true
     if (c)   Serial.print(c);

  // Si se recibió una sentencia, podemos hacer el checksum y obtener sus datos...
  if (GPS.newNMEAreceived()) {
        
    if (!GPS.parse(GPS.lastNMEA()))   // Verifica que la información sea correcta, esto pone la bandera newNMEAreceived() en false
      return;  // Podemos fallar al leer una oración, en cuyo caso solo esperamos a la siguiente
    
    // Sentencia leida correctamtente! 
    Serial.println("OK");
    if (LOG_FIXONLY && !GPS.fix) {
        Serial.print("No Fix");
        return;
    }
    // Hora de loggear la información!:
    Serial.println("Log");
    char *stringptr = GPS.lastNMEA();
    uint8_t stringsize = strlen(stringptr);
    if (stringsize != logfile.write((uint8_t *)stringptr, stringsize))    //Escribe el string en la memoria SD
      error(4);
    if (strstr(stringptr, "RMC"))   logfile.flush(); //Comprueba que se haya guardado la información correctamente
    Serial.println();
  }
}
/* fin del código */
