// ====== COM CONFIG ======
// --- Únicamente para ATTiny ----
#include "SoftwareSerial.h"
// SoftwareSerial COM(3, 4);
SoftwareSerial COM(0, 1);

// --- Varibles del dispositivo ----
char ID = 'C';  //Identificares M:(master), C:(slave), T:(slave)
char mensaje = 'N';
int valor = 0;

// --- Cadena para recibir trama ----
String tramaRecibida;
// ==============================


// ========= PIN CONFIG =============
// ---------------------------------
// ==============================


// ===== Contantes =====
// ---------------------------------
// ==============================


// ===== Variables Globales =====
// ---------------------------------
// ==============================


// ===== Funciones Principales ----=====
// ---------------------------------
// ==============================


// ===== Funciones Complementarias =====
// ---------------------------------
// ==============================



// ===== MAIN =====
void setup() {
  COM.begin(9600);  // comentar si es arduino UNO - descomentar si es ATTiny
  // Serial.begin(9600);  // descomentar si es arduino UNO - comentar si es ATTiny
}


void loop() {

  // Leer la trama completa en el Arduino
  if (COM.available() > 0) {
    delay(10);
    tramaRecibida = COM.readStringUntil('|');  // Leer hasta un salto de línea

    if (tramaRecibida[0] == ID) {

      mensaje = tramaRecibida[1];
      valor = 100 * (tramaRecibida[2] - '0') + 10 * (tramaRecibida[3] - '0') + (tramaRecibida[4] - '0');


      COM.flush(); //Limpiar buffer de entrada
    }

    // Ejecutar funcion correspondiente al mensaje
    switch (mensaje) {
      case 'R':
        // funcion
        COM.println("mensaje 1");
        break;
      case 'I':
        // funcion
        COM.println("mensaje 2");
        break;
      case 'S':
        // funcion
        COM.println("mensaje 3");
        break;
      case 'F':
        // funcion
        COM.println("mensaje 4");
        break;
      case 'A':
        // funcion
        COM.println("mensaje 4");
        break;
    }
    mensaje = ' ';
  }

  // if (millis() - lastReceiveTime > timeout) {   // Verificar si ha pasado mucho tiempo sin recibir trama (unicamente el maestro)
  //   // intrucciones a aplicar
  // }
}
