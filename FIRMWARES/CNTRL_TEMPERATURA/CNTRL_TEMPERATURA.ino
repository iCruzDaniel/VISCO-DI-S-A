// ====== COM CONFIG ======

// --- Varibles del dispositivo ----
char ID = 'T';  //Identificador del dispositivo (CNTRL DE TEMPERATURA)
char mensaje = 'N';
int valor = 0;
// xxxx
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
  Serial.begin(9600); 
}


void loop() {

  // Leer la trama completa en el Arduino
  if (Serial.available() > 0) {
    delay(10);
    tramaRecibida = Serial.readStringUntil('|');  // Leer hasta un salto de línea

    if (tramaRecibida[0] == ID) {

      mensaje = tramaRecibida[1];
      valor = 100 * (tramaRecibida[2] - '0') + 10 * (tramaRecibida[3] - '0') + (tramaRecibida[4] - '0');


      Serial.flush(); //Limpiar buffer de entrada
    }

    // Ejecutar funcion correspondiente al mensaje
    switch (mensaje) {
      case 'R':
        // funcion
        Serial.println("mensaje 1"); //borrar una vez terminada la función de esta seccion...
        break;
      case 'I':
        // funcion
        Serial.println("mensaje 2"); //borrar una vez terminada la función de esta seccion...
        break;
      case 'S':
        // funcion
        Serial.println("mensaje 3"); //borrar una vez terminada la función de esta seccion...
        break;
      case 'F':
        // funcion
        Serial.println("mensaje 4"); //borrar una vez terminada la función de esta seccion...
        break;
      case 'A':
        // funcion
        Serial.println("mensaje 4"); //borrar una vez terminada la función de esta seccion...
        break;
    }
    mensaje = ' ';
  }


}
