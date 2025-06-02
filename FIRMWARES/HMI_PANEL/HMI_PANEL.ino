// ====== LIB CONFIG ======
// --- Includes ----
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <SD.h>
// --- Object Init ----
LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD 20x4

File file;  //Archivo de para guardar info

// --- Configuración del teclado ---
const byte ROWS = 5;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { 'A', 'B', '#', '*' },
  { '1', '2', '3', 'U' },
  { '4', '5', '6', 'D' },
  { '7', '8', '9', 'E' },
  { 'L', '0', 'R', 'N' }
};
// byte rowPins[ROWS] = { 9, 8, 7, 6, 5 };
byte rowPins[ROWS] = { 6, 5, 4, 3, 2 };
// byte colPins[COLS] = { 4, 3, 2, A0 };
byte colPins[COLS] = { A0, A1, A2, A3 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Estados del sistema ---
enum AppState {
  START_SCREEN,
  MAIN_MENU,
  NEW_TEST_MENU,
  CONFIG_VARS_MENU,
  EDIT_TEMP,
  EDIT_ITER,
  SPHERE_POSITION_QUESTION,
  SPHERE_RELEASED,
  SAVE_DATA_QUESTION
};

AppState currentState = START_SCREEN;
// ==============================

// ====== COM CONFIG ======
// --- Varibles del dispositivo ----
char ID = 'M';  //Identificador del dispositivo (HMI Panel master)
char respuesta = 'N';
int valor = 0;

bool sol_init_temp = 0; //variables para establecer comunicacion entre micros (0=N0 1=Ok)
bool sol_init_c = 0;    //variables para establecer comunicacion entre micros (0=N0 1=Ok)
bool inicializacion_aux = false; //saber si se ejecutó la funcion en el loop
int iniciar_temp = 0; // variable auxiliar para establecer el inicio de la temperatura.
int iniciar_caida = 0; // variable auxiliar para establecer el inicio de la temperatura.

bool init_caida_com = true;

//--------------  VARIABLES CONSULTA TEMPERATURA ACTUAL -------------------
unsigned long lastRequestTime_temp = 0;
const unsigned long intervalo_temp_act = 2000; // tiempo entre solicitudes en ms
byte temperatura_act_EEPROM = -1; // para comparar cambios antes de escribir
//-----------------------
// --- Cadena para recibir trama ----
String tramaRecibida;
// ==============================


// ========= PIN CONFIG =============
const int chipSelect = 10;
// ---------------------------------
// ==============================


// ===== Contantes =====
const int TEMP_ADDR = 0;
const int TEMP_ACT_ADDR = 2;
const int ITER_ADDR = sizeof(byte);  // sizeof(int) a sizeof(byte)
// ---------------------------------
// ==============================


// ===== Variables Globales =====
byte temperatura = 20;
byte iteraciones = 1;
byte currentIteration = 1;
byte selectedOption = 0;
// ---------------------------------
#define MAX_INPUT_BUFFER_LEN 4  // Max 3 dígitos para temp/iter + null terminator
char inputBuffer[MAX_INPUT_BUFFER_LEN];
byte inputBufferLen = 0;  // Para llevar la cuenta de los caracteres actuales
bool sphereInPosition = false;
// ==============================


// ===== Funciones Principales ----=====
// ---------------------------------
// ==============================


// ===== Funciones Complementarias =====

// Función para verificar RAM libre (mantenerla para depuración)
int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
// ---------------------------------


// ==============================



// ===== MAIN =====
void setup() {
  // //=================
  // pinMode(7, OUTPUT);
  // digitalWrite(7, 1);
  // delay(4000);
  // digitalWrite(7, 0);
  // //=================


  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
 
  // Inicializar inputBuffer
  inputBuffer[0] = '\0';  // Asegurar que esté vacío al inicio
  inputBufferLen = 0;

  EEPROM.get(TEMP_ADDR, temperatura);
  EEPROM.get(ITER_ADDR, iteraciones);

  if (temperatura < 0 || temperatura > 100) temperatura = 20;
  if (iteraciones < 1) iteraciones = 1;

  //  Serial.println(F("Inicializando tarjeta SD..."));
  //  if (!SD.begin(chipSelect)) {
  //    Serial.println(F("Error: Fallo al inicializar la SD."));
  //    lcd.clear();
  //    lcd.setCursor(0, 0);
  //    lcd.print(F("Fallo SD!"));
  //    while (true)
  //      ;  // Detiene el programa aquí para que veas el error
  //  }
   Serial.println(F("SD inicializada correctamente."));
   Serial.print(F("RAM libre al inicio: "));
   Serial.println(freeRam());
  solicitud_init();
//   if (init_temp == 1 && init_c == 1) {
//    showStartScreen();
//  }
//bucle para enviar comandos hasta que los esclavos respondan
    while (!(sol_init_temp && sol_init_c)) {
  solicitud_init();
  unsigned long t0 = millis();
  while (millis() - t0 < 1000) {
    procesarTramaSerial();
  }
}
  
  // if (temperatura == temperatura_act_EEPROM){
  //   iniciar_caida = 0;
  //   while (iniciar_caida != 1) {
  //       procesarTramaSerial();
  //       Serial.print(F("CI|"));
  //       delay(1000);
  //   }
  // }


}


void loop() {
  char key = keypad.getKey();
  if (key) handleKey(key);
  procesarTramaSerial();
  
  // Leer la trama completa en el Arduino
  
    if (sol_init_temp && sol_init_c && !inicializacion_aux) {
     showStartScreen();
    inicializacion_aux = true;
  }
  consultarTemperaturaCadaIntervalo();

  if (sol_init_temp && sol_init_c && !inicializacion_aux) {
     showStartScreen();
    inicializacion_aux = true;
  }

    if (temperatura == temperatura_act_EEPROM){
      if (init_caida_com){
        iniciar_caida = 0;
        while (iniciar_caida != 1) {
            procesarTramaSerial();
            Serial.print(F("CI|"));
            delay(1000);
            init_caida_com = false;
            
    }
    showTestRunningScreen();
      }
      
  }
}

void solicitud_init(){ //envio para inicializar temp y caida
   Serial.print(F("TR|"));
   delay(500);
   Serial.print(F("CR|"));
   delay(500);
}
// void sistem_init(){
//  Serial.print(F("TI|"));
//   delay(500);
//   Serial.print(F("CI|"));
//   delay(500);
// }
void procesarTramaSerial() {
  if (Serial.available() > 0) {
    delay(10);  // Espera breve para asegurar recepción completa
    String tramaRecibida = Serial.readStringUntil('|');  // Leer hasta delimitador
    char esclavoID = tramaRecibida[0];
    char respuesta = tramaRecibida[1];
    int valor = 0;

    if (tramaRecibida.length() >= 5) {
      valor = 100 * (tramaRecibida[2] - '0') + 10 * (tramaRecibida[3] - '0') + (tramaRecibida[4] - '0');
    }

    Serial.flush();  // Limpiar buffer de entrada

    if (esclavoID == 'T') {
      switch (respuesta) {
        case 'R':
          sol_init_temp = true;
          break;
        case 'C':
          
          iniciar_temp = valor;
          break;
        // case 'V':
        //   Serial.println("respuesta 3");
        //   break;
        case 'S':
          Serial.println("respuesta 4");
          break;
        case 'A':
          temperatura_act_EEPROM = valor;
          EEPROM.put(TEMP_ACT_ADDR, temperatura_act_EEPROM);
          break;
      }
    }

    if (esclavoID == 'C') {
      switch (respuesta) {
        case 'R':
          sol_init_c = true;
          break;
        case 'C':
          iniciar_caida = valor;
          //Serial.println("respuesta 2");
          break;
        case 'V':
          Serial.println("respuesta 3");
          break;
        case 'S':
        Serial.println("respuesta 4");
        break;
      }
    }
  }
}
//----------Funcion consulta temperatura actual
void consultarTemperaturaCadaIntervalo() {
  unsigned long time_temp_act = millis();

  if (time_temp_act - lastRequestTime_temp >= intervalo_temp_act) {
    lastRequestTime_temp = time_temp_act;
    procesarTramaSerial();
    // Enviar solicitud al esclavo (por ejemplo: "T?")
    Serial.print(F("TA|"));
  }
  

}
// -----------

void showStartScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F(">> Inicializando <<"));
  lcd.setCursor(3, 1);
  lcd.print(F("--Ensayo de"));
  lcd.setCursor(3, 2);
  lcd.print(F("Viscosidad --"));
  lcd.setCursor(5, 3);
  lcd.print(F("Espere..."));
  delay(2500);
  currentState = MAIN_MENU;
  showMainMenu();
}

void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("--Menu Principal--"));

  lcd.setCursor(1, 2);
  lcd.print(F("Nuevo ensayo"));
  if (selectedOption == 0) lcd.setCursor(18, 2), lcd.print(F("<"));

  lcd.setCursor(1, 3);
  lcd.print(F("Config. de HMI"));
  if (selectedOption == 1) lcd.setCursor(18, 3), lcd.print(F("<"));
}

void showNewTestMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("--Inicio y Config--"));

  lcd.setCursor(1, 2);
  lcd.print(F("Iniciar ensayo"));
  if (selectedOption == 0) lcd.setCursor(18, 2), lcd.print(F("<"));

  lcd.setCursor(1, 3);
  lcd.print(F("Config. Variables"));
  if (selectedOption == 1) lcd.setCursor(18, 3), lcd.print(F("<"));
}

void showConfigVarsMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("-Config. Variables-"));

  lcd.setCursor(1, 1);
  lcd.print(F("Temperatura = "));
  lcd.print(temperatura);
  if (selectedOption == 0) lcd.setCursor(19, 1), lcd.print(F("<"));

  lcd.setCursor(1, 2);
  lcd.print(F("Iteraciones = "));
  lcd.print(iteraciones);
  if (selectedOption == 1) lcd.setCursor(19, 2), lcd.print(F("<"));

  lcd.setCursor(1, 3);
  lcd.print(F("Ent (Modificar)"));
}

void showEditTemp() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(F("Nueva temperatura:"));
  lcd.setCursor(1, 2);
  lcd.print(F("> "));
  lcd.print(inputBuffer);  // Imprimir el char array
  lcd.setCursor(1, 3);
  lcd.print(F("Rango: 0-100"));
}

void showEditIter() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(F("Nuevas iteraciones:"));
  lcd.setCursor(1, 2);
  lcd.print(F("> "));
  lcd.print(inputBuffer);  // Imprimir el char array
  lcd.setCursor(1, 3);
  lcd.print(F("Minimo: 1"));
}

void showTestRunningScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("-Ejecutando Ensayo-"));

  lcd.setCursor(1, 1);
  lcd.print(F("Iter. = "));
  lcd.print(currentIteration);

  lcd.setCursor(1, 2);
  lcd.print(F("Esfera en posicion?"));

  lcd.setCursor(1, 3);
  lcd.print(F("Si"));
  if (selectedOption == 0) lcd.setCursor(18, 3), lcd.print(F("<"));
}

void showSphereReleased() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("-Ensayo en Progreso-"));
  lcd.setCursor(2, 2);
  lcd.print(F("Esfera Liberada!"));
  delay(2000);

  if (currentIteration < iteraciones) {
    currentIteration++;
    currentState = SPHERE_POSITION_QUESTION;
    showTestRunningScreen();
  } else {
    currentState = SAVE_DATA_QUESTION;
    selectedOption = 0;
    showSaveDataQuestion();
  }
}

// NUEVO: Menú para guardar datos
void showSaveDataQuestion() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("-Guardar resultados?-"));

  lcd.setCursor(1, 2);
  lcd.print(F("Si"));
  if (selectedOption == 0) lcd.setCursor(18, 2), lcd.print(F("<"));

  lcd.setCursor(1, 3);
  lcd.print(F("No"));
  if (selectedOption == 1) lcd.setCursor(18, 3), lcd.print(F("<"));
}

void saveDataToSD() {
  Serial.print(F("RAM libre antes de abrir archivo SD: "));
  Serial.println(freeRam());

  file = SD.open("data.txt", FILE_WRITE);

  if (file) {
    Serial.print(F("RAM libre despues de abrir y antes de escribir SD: "));
    Serial.println(freeRam());

    file.print(F("Temperatura: "));
    file.println(temperatura);
    file.print(F("Iteraciones: "));
    file.println(iteraciones);
    file.println(F("------"));
    file.close();

    Serial.print(F("RAM libre despues de cerrar archivo SD: "));
    Serial.println(freeRam());

    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print(F("Datos guardados"));
    delay(2000);
  } else {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print(F("Error al guardar"));
    delay(2000);
    Serial.print(F("RAM libre despues de error de guardar SD: "));
    Serial.println(freeRam());
  }

  currentState = MAIN_MENU;
  showMainMenu();
}

void handleKey(char key) {

  switch (currentState) {
    case MAIN_MENU:
      if (key == 'U' || key == 'D') selectedOption = (selectedOption + 1) % 2, showMainMenu();
      else if (key == 'N') {
        if (selectedOption == 0) currentState = NEW_TEST_MENU, selectedOption = 0, showNewTestMenu();
      }
      break;

    case NEW_TEST_MENU:
      if (key == 'U' || key == 'D') selectedOption = (selectedOption + 1) % 2, showNewTestMenu();
      else if (key == 'N') {
        
      if (selectedOption == 0) {
        currentIteration = 1;
        currentState = SPHERE_POSITION_QUESTION;
        if (temperatura == temperatura_act_EEPROM){
          showTestRunningScreen();
        }
        
        iniciar_temp = 0;
        while (iniciar_temp != 1) {
          procesarTramaSerial();
          Serial.print(F("TI|"));
          delay(1000);
        }
        iniciar_temp = 0;
        while (iniciar_temp != 1) {
          procesarTramaSerial();
          Serial.print(F("TS0"));
          Serial.print(temperatura);
          Serial.print(F("|"));
          delay(1000);
        }
        
        
      }
        else currentState = CONFIG_VARS_MENU, selectedOption = 0, showConfigVarsMenu();

      } else if (key == 'E') currentState = MAIN_MENU, showMainMenu();
      break;

    case CONFIG_VARS_MENU:
      if (key == 'U' || key == 'D') selectedOption = (selectedOption + 1) % 2, showConfigVarsMenu();
      else if (key == 'N') {
        if (selectedOption == 0) {
          currentState = EDIT_TEMP;
          inputBufferLen = 0;
          inputBuffer[0] = '\0';  // Limpiar buffer
          showEditTemp();
        } else if (selectedOption == 1) {
          currentState = EDIT_ITER;
          inputBufferLen = 0;
          inputBuffer[0] = '\0';  // Limpiar buffer
          showEditIter();
        }
      } else if (key == 'E') currentState = NEW_TEST_MENU, showNewTestMenu();
      break;

    case EDIT_TEMP:
      if ((key >= '0' && key <= '9') && inputBufferLen < (MAX_INPUT_BUFFER_LEN - 1)) {
        inputBuffer[inputBufferLen++] = key;
        inputBuffer[inputBufferLen] = '\0';  // Asegurar el terminador nulo
        showEditTemp();
      } else if (key == 'L' && inputBufferLen > 0) {
        inputBuffer[--inputBufferLen] = '\0';  // Retroceder y poner terminador nulo
        showEditTemp();
      } else if (key == 'E') currentState = CONFIG_VARS_MENU, showConfigVarsMenu();
      else if (key == 'N' && inputBufferLen > 0) {
        int newTemp = atoi(inputBuffer);  // Usar atoi() para convertir char[] a int
        if (newTemp >= 0 && newTemp <= 100) {
          temperatura = (byte)newTemp;  // Cast a byte
          EEPROM.put(TEMP_ADDR, temperatura);
          currentState = CONFIG_VARS_MENU;
          showConfigVarsMenu();
          
        } else {
          lcd.setCursor(1, 3), lcd.print(F("Error: Fuera rango!"));
          delay(1500);
          inputBufferLen = 0;
          inputBuffer[0] = '\0';  // Resetear buffer
          showEditTemp();
        }
      }
      break;

    case EDIT_ITER:
      // Para iteraciones, MAX_INPUT_BUFFER_LEN asume 3 dígitos, si solo esperas 2, ajusta
      // o usa MAX_INPUT_BUFFER_LEN directamente si es suficiente (ej. 99 iteraciones)
      if ((key >= '0' && key <= '9') && inputBufferLen < (MAX_INPUT_BUFFER_LEN - 1)) {
        inputBuffer[inputBufferLen++] = key;
        inputBuffer[inputBufferLen] = '\0';
        showEditIter();
      } else if (key == 'L' && inputBufferLen > 0) {
        inputBuffer[--inputBufferLen] = '\0';
        showEditIter();
      } else if (key == 'E') currentState = CONFIG_VARS_MENU, showConfigVarsMenu();
      else if (key == 'N' && inputBufferLen > 0) {
        int newIter = atoi(inputBuffer);  // Usar atoi()
        if (newIter >= 1) {
          iteraciones = (byte)newIter;  // Cast a byte
          EEPROM.put(ITER_ADDR, iteraciones);
          currentState = CONFIG_VARS_MENU;
          showConfigVarsMenu();
        } else {
          lcd.setCursor(1, 3), lcd.print(F("Error: Minimo 1!"));
          delay(1500);
          inputBufferLen = 0;
          inputBuffer[0] = '\0';  // Resetear buffer
          showEditIter();
        }
      }
      break;

    case SPHERE_POSITION_QUESTION:
      if (key == 'N' && selectedOption == 0) {
        Serial.print(F("CS001|"));
        sphereInPosition = true;
        // Serial.print(F("SphereInPosition: "));
        // Serial.println(sphereInPosition);
        currentState = SPHERE_RELEASED;
        showSphereReleased();
      } else if (key == 'E') currentState = NEW_TEST_MENU, showNewTestMenu();
      break;

    case SAVE_DATA_QUESTION:
      if (key == 'U' || key == 'D') selectedOption = (selectedOption + 1) % 2, showSaveDataQuestion();
      else if (key == 'N') {
        if (selectedOption == 0) saveDataToSD();
        else {
          currentState = MAIN_MENU;
          showMainMenu();
        }
      }
      break;
  }
}