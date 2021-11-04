  

/* Este software fue creado con base en los ejemplos incluidos en las bibliotecas TinyGPS y NXTIoT_dev:
  https://github.com/mikalhart/TinyGPS
  https://github.com/NXTIoT/Libreria_Devkit_NXTIoT */
// Incluir bibliotecas para el módulo de posicionamiento GPS y la tarjeta Devkit
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#define SigfoxDeb
// Definir entrada del módulo de posicionamiento GPS en los pines 4 y 3 (TX y RX, respectivamente)
TinyGPS gps;
SoftwareSerial myGPS(4, 3);
String bufer; // Variable para almacenar la carga útil del mensaje a enviar
const int boton = 6; // Definir el pin del botón de acción (azul)
const int modem = 7; // Definir el pin del módem Sigfox
unsigned long limitMs=900000;
unsigned long startTime=0;
void setup() {
  Serial.begin(9600); // Baudios del monitor serie de la pc
  myGPS.begin(9600); // Baudios del módulo de posicionamiento GPS (sensor)
  myGPS.println("\n\rInicio"); // Imprimir mensaje de Inicio del módulo
  pinMode(boton, INPUT); // Definir el botón de acción como entrada
  pinMode(modem, OUTPUT); // Definir el módem como salida
}
void loop() {
  bool newData = false; // Bandera de recepción de información del módulo
  // Variables auxiliares para indicar errores, en caso de haber
  unsigned long chars;
  unsigned short sentences, failed;
  // Lectura de los valores de módulo cada segundo
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (myGPS.available()) // Revisar disponibilidad del sensor
    {
      char c = myGPS.read(); // Leer información del sensor40.
    
      if (gps.encode(c)) // Si entra un mensaje del sensorc ` n*\][=-
        newData = true; // Se cambia la bandera de recepción de la información
    }
  }
  if (newData) // Se ejecuta cuando sí hay información proveniente del sensor
  {
    float flat, flon; // Definir variables de latitud y longitud
    unsigned long age; // Definir variable auxiliar de año (no utilizada)
    // * Obtener valores de latitud y longitud, e imprimirlos en el puerto serie *
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    if (digitalRead(boton) == LOW) { // Se ejecuta si el usuario presiona el botón de acción
      // * Enviar los valores de latitud y longitud a la carga útil del mensaje *
      leer_info(flat, flon);
      delay(1000); // Esperar un segundo al terminar la rutina
    }
    if(startTime+limitMs < millis()){
      Serial.println("Ha pasado 15 min!"); 
      startTime+=limitMs;
      leer_info(flat, flon);
    }
  }
  // Rutina para informar error de recepción de la información en el monitor en serie
  gps.stats(&chars, &sentences, &failed);
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  if (chars == 0)
    Serial.println("** No se han recibido caracteres del GPS: compruebe el cableado **");
}
// Función para agregar información a la carga útil del mensaje para enviar
void leer_info(float flat, float flon) {
  /* La variable "bufer" almacena la carga útil del mensaje. Se inicializa como "AT$SF="
    porque es la instrucción para el envío del mensaje */
  bufer = "AT$SF=";
  add_float(flat); // Añadir el valor de latitud a la carga útil
  Serial.println();
  //send_message(bufer); // * Enviar la carga útil (latitud) en un mensaje *
  delay(1000);
  //bufer = "AT$SF="; // Se define una nueva carga útil para enviar el valor de longitud
  add_float(flon); // Añadir el valor de Longitud a la carga útil
  Serial.println();
  Serial.println(bufer);
  send_message(bufer); // * Enviar la carga útil (longitud) en un mensaje *
}
/* Función para añadir los valores de latitud y longitud en formato hexadecimal
  a la carga útil que se enviará en mensaje por la red Sigfox */
void add_float(float var1)
{
  // Definir variable auxiliar de tipo byte, respecto a la variable de entrada
  byte* a1 = (byte*) &var1;
  String str1; // Definir variable auxiliar para añadir la info. a la carga útil
  // Convertir y añadir la información a la carga útil
  for (int i = 0; i < 4; i++) // Nota: el dato de formato de coma flotante requiere 4 bytes
  {
    str1 = String(a1[i], HEX); // Conversión de byte a valor hexadecimal
    if (str1.length() < 2)
    {
      bufer += 0 + str1; // Añadir ceros si es necesario completar el mensaje
    }
    else
    {
      bufer += str1;
    }
  }
}
// Función para enviar la carga útil por mensaje en la red Sigfox
void send_message(String payload)
{
  bufer += "\n"; //Agregar un salto de línea en la carga útil
  digitalWrite(modem, HIGH); // Encender el módem de la red Sigfox
  delay(1000);
  Serial.print("AT$RC\n"); // Reiniciar el canal para asegurar sintonizar la frecuencia correcta
  Serial.print(bufer); // * Enviar la carga útil *
  delay(2000);
  digitalWrite(modem, LOW); // Apagar el módem de la red Sigfox
  delay(500);
}
