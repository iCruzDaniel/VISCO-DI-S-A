// ====== COM CONFIG ======
// --- Varibles del dispositivo ----
char ID = 'M';  //Identificador del dispositivo (HMI Panel master)
char respuesta = 'N';
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
  Serial.begin(9600);  // descomentar si es arduino UNO - comentar si es ATTiny
}


void loop() {

  // Leer la trama completa en el Arduino
  if (Serial.available() > 0) {
    delay(10);
    tramaRecibida = Serial.readStringUntil('|');  // Leer hasta un salto de línea
    char esclavoID = tramaRecibida[0];
    respuesta = tramaRecibida[1];
    valor = 100 * (tramaRecibida[2] - '0') + 10 * (tramaRecibida[3] - '0') + (tramaRecibida[4] - '0');

    if (esclavoID == 'T') {
      Serial.flush();  //Limpiar buffer de entrada
      // Ejecutar funcion correspondiente al respuesta
      switch (respuesta) {
        case 'R':
          // funcion
          Serial.println("respuesta 1");
          break;
        case 'I':
          // funcion
          Serial.println("respuesta 2");
          break;
        case 'S':
          // funcion
          Serial.println("respuesta 3");
          break;
        case 'F':
          // funcion
          Serial.println("respuesta 4");
          break;
        case 'A':
          // funcion
          Serial.println("respuesta 4");
          break;
      }
    }
    if (esclavoID == 'C') {
      Serial.flush();  //Limpiar buffer de entrada
      // Ejecutar funcion correspondiente al respuesta
      switch (respuesta) {
        case 'R':
          // funcion
          Serial.println("respuesta 1");
          break;
        case 'I':
          // funcion
          Serial.println("respuesta 2");
          break;
        case 'S':
          // funcion
          Serial.println("respuesta 3");
          break;
      }
    }

    respuesta = ' ';
  }
}
