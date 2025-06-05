#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // LCD 20x4

// Configuración del teclado (igual que antes)
const byte ROWS = 5;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'A', 'B', '#', '*'},
  {'1', '2', '3', 'U'},
  {'4', '5', '6', 'D'},  
  {'7', '8', '9', 'E'},  
  {'L', '0', 'R', 'N'},
};
byte rowPins[ROWS] = { 6, 5, 4, 3, 2 };
byte colPins[COLS] = { A0, A1, A2, A3 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Estados del sistema (igual que antes)
enum AppState {
  START_SCREEN,
  MAIN_MENU,
  NEW_TEST_MENU,
  CONFIG_VARS_MENU,
  EDIT_TEMP,
  EDIT_ITER,
  SPHERE_POSITION_QUESTION,
  SPHERE_RELEASED
};
AppState currentState = START_SCREEN;

// Variables globales (igual que antes)
int temperatura = 20;
int iteraciones = 1;
int currentIteration = 1;
byte selectedOption = 0;
String inputBuffer = "";
bool sphereInPosition = false;

// Direcciones EEPROM (igual que antes)
const int TEMP_ADDR = 0;
const int ITER_ADDR = sizeof(int);

void setup() {
    // //=================
  pinMode(7, OUTPUT);
  digitalWrite(7, 1);
  delay(3000);
  digitalWrite(7, 0);
  // //=================
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  
  EEPROM.get(TEMP_ADDR, temperatura);
  EEPROM.get(ITER_ADDR, iteraciones);
  
  if(temperatura < 0 || temperatura > 100) temperatura = 20;
  if(iteraciones < 1) iteraciones = 1;
  
  showStartScreen();
}

void loop() {
  char key = keypad.getKey();
  if(key) handleKey(key);
  procesarTramaSerial();
}

// Pantalla de inicio (con nombre)
void showStartScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">> Inicializando <<");
  lcd.setCursor(3, 1);
  lcd.print("--Ensayo de");
  lcd.setCursor(3, 2);
  lcd.print("Viscosidad --");
  lcd.setCursor(5, 3);
  lcd.print("Espere...");
  delay(2500);
  currentState = MAIN_MENU;
  showMainMenu();
}
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
          Serial.println("respuesta 4");
          break;
        case 'C':
          Serial.println("respuesta 4");
          break;
        case 'V':
          Serial.println("respuesta 3");
          break;
        case 'S':
          Serial.println("respuesta 4");
          break;
        case 'A':
          Serial.println("respuesta 4");
          break;
      }
    }

    if (esclavoID == 'C') {
      switch (respuesta) {
        case 'R':
         Serial.println("respuesta 4");
          break;
        case 'C':
          
          //Serial.println("respuesta 2");
          break;
        case 'V':
          Serial.println("respuesta 4");
          //Serial.println("respuesta 3");
          break;
        case 'S':
        Serial.println("respuesta 4");
        break;
      }
    }
  }
}

// Menú principal (con nombre)
void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("--Menu Principal--");
  
  lcd.setCursor(1, 2);
  lcd.print("Nuevo ensayo");
  if(selectedOption == 0) lcd.setCursor(18, 2), lcd.print("<");
  
  lcd.setCursor(1, 3);
  lcd.print("Config. de HMI");
  if(selectedOption == 1) lcd.setCursor(18, 3), lcd.print("<");
}

// Submenú de nuevo ensayo (con nombre)
void showNewTestMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("--Inicio y Config--");
  
  lcd.setCursor(1, 2);
  lcd.print("Iniciar ensayo");
  if(selectedOption == 0) lcd.setCursor(18, 2), lcd.print("<");
  
  lcd.setCursor(1, 3);
  lcd.print("Config. Variables");

  if(selectedOption == 1) lcd.setCursor(18, 3), lcd.print("<");
}

// Menú de configuración de variables (con nombre)
void showConfigVarsMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("-Config. Variables-");
  
  lcd.setCursor(1, 1);
  lcd.print("Temperatura = ");
  lcd.print(temperatura);
  if(selectedOption == 0) lcd.setCursor(19, 1), lcd.print("<");
  
  lcd.setCursor(1, 2);
  lcd.print("Iteraciones = ");
  lcd.print(iteraciones);
  if(selectedOption == 1) lcd.setCursor(19, 2), lcd.print("<");
  
  lcd.setCursor(1, 3);
  lcd.print("Ent (Modificar)");
}

// Pantallas de edición (sin nombre en primera fila como solicitaste)
void showEditTemp() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Nueva temperatura:");
  lcd.setCursor(1, 2);
  lcd.print("> ");
  lcd.print(inputBuffer);
  lcd.setCursor(1, 3);
  lcd.print("Rango: 0-100");
}

void showEditIter() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Nuevas iteraciones:");
  lcd.setCursor(1, 2);
  lcd.print("> ");
  lcd.print(inputBuffer);
  lcd.setCursor(1, 3);
  lcd.print("Minimo: 1");
}

// Pantalla de ejecución de ensayo (con nombre)
void showTestRunningScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("-Ejecutando Ensayo-");
  
  lcd.setCursor(1, 1);
  lcd.print("Iter. = ");
  lcd.print(currentIteration);
  
  lcd.setCursor(1, 2);
  lcd.print("Esfera en posicion?");
  
  lcd.setCursor(1, 3);
  lcd.print("Si");
  if(selectedOption == 0) lcd.setCursor(18, 3), lcd.print("<");
}

// Pantalla de esfera liberada (con nombre)
void showSphereReleased() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("-Ensayo en Progreso-");
  lcd.setCursor(2, 2);
  lcd.print("Esfera Liberada!");
  delay(2000);
  
  if(currentIteration < iteraciones) {
    currentIteration++;
    currentState = SPHERE_POSITION_QUESTION;
    showTestRunningScreen();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("-Ensayo Finalizado-");
    delay(2000);
    currentState = MAIN_MENU;
    currentIteration = 1;
    showMainMenu();
  }
}

// Manejador de teclas (igual que antes)
void handleKey(char key) {
  switch(currentState) {
    case MAIN_MENU:
      if(key == 'U' || key == 'D') selectedOption = (selectedOption + 1) % 2, showMainMenu();
      else if(key == 'N') {
        if(selectedOption == 0) currentState = NEW_TEST_MENU, selectedOption = 0, showNewTestMenu();
      }
      break;
      
    case NEW_TEST_MENU:
      if(key == 'U' || key == 'D') selectedOption = (selectedOption + 1) % 2, showNewTestMenu();
      else if(key == 'N') {
        if(selectedOption == 0) currentIteration = 1, currentState = SPHERE_POSITION_QUESTION, showTestRunningScreen();
        else currentState = CONFIG_VARS_MENU, selectedOption = 0, showConfigVarsMenu();
      }
      else if(key == 'E') currentState = MAIN_MENU, showMainMenu();
      break;
      
    case CONFIG_VARS_MENU:
      if(key == 'U' || key == 'D') selectedOption = (selectedOption + 1) % 2, showConfigVarsMenu();
      else if(key == 'N') {
        if(selectedOption == 0) currentState = EDIT_TEMP, inputBuffer = "", showEditTemp();
        else if(selectedOption == 1) currentState = EDIT_ITER, inputBuffer = "", showEditIter();
      }
      else if(key == 'E') currentState = NEW_TEST_MENU, showNewTestMenu();
      break;
      
    case EDIT_TEMP:
      if((key >= '0' && key <= '9') && inputBuffer.length() < 3) inputBuffer += key, showEditTemp();
      else if(key == 'L' && inputBuffer.length() > 0) inputBuffer.remove(inputBuffer.length() - 1), showEditTemp();
      else if(key == 'E') currentState = CONFIG_VARS_MENU, showConfigVarsMenu();
      else if(key == 'N' && inputBuffer.length() > 0) {
        int newTemp = inputBuffer.toInt();
        if(newTemp >= 0 && newTemp <= 100) {
          temperatura = newTemp, EEPROM.put(TEMP_ADDR, temperatura);
          currentState = CONFIG_VARS_MENU, showConfigVarsMenu();
        } else {
          lcd.setCursor(1, 3), lcd.print("Error: Fuera rango!");
          delay(1500), inputBuffer = "", showEditTemp();
        }
      }
      break;
      
    case EDIT_ITER:
      if((key >= '0' && key <= '9') && inputBuffer.length() < 2) inputBuffer += key, showEditIter();
      else if(key == 'L' && inputBuffer.length() > 0) inputBuffer.remove(inputBuffer.length() - 1), showEditIter();
      else if(key == 'E') currentState = CONFIG_VARS_MENU, showConfigVarsMenu();
      else if(key == 'N' && inputBuffer.length() > 0) {
        int newIter = inputBuffer.toInt();
        if(newIter >= 1) {
          iteraciones = newIter, EEPROM.put(ITER_ADDR, iteraciones);
          currentState = CONFIG_VARS_MENU, showConfigVarsMenu();
        } else {
          lcd.setCursor(1, 3), lcd.print("Error: Minimo 1!");
          delay(1500), inputBuffer = "", showEditIter();
        }
      }
      break;
      
    case SPHERE_POSITION_QUESTION:
      if(key == 'N' && selectedOption == 0) {
        sphereInPosition = true;
        Serial.print("CI|");
        currentState = SPHERE_RELEASED, showSphereReleased();
      }
      else if(key == 'E') currentState = NEW_TEST_MENU, showNewTestMenu();
      break;
  }
}
