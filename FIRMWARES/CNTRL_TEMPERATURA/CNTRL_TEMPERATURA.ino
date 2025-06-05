#include <fuzzy.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_MAX31865.h>
#include <pt100rtd.h>
#define RREF 430.0

// ====== COM CONFIG ======
char ID = 'T';  // Identificador del dispositivo
char mensaje = 'N';
int valor = 0;

// --- Cadena para recibir trama ----
String tramaRecibida;

// ===== CONTROL DIFUSO ======
fuzzy controlfuzzy;

float ERROR_RANGE[] = { -15, 15 };  //rango de error
float DERIVADA_RANGE[] = { -5, 5 };
float POTENCIA_RANGE[] = { 0, 100 };

// ===Conjuntos difusos para Error=== ///
// Error Negativo Grande (calor extremo - necesitamos enfriar)
float E_NG[] = { -15, -15, -10, -5, 'R' }; 
// Error Negativo Pequeño (calor moderado - enfriar suave)
float E_NP[] = { -8, -5, -2, 'T' };
// Error Cero (en setpoint)
float E_C[] = { -2, 0, 2, 'T' };           
// Error Positivo Pequeño (frío moderado - calentar suave)
float E_PP[] = { 2, 5, 8, 'T' };       
// Error Positivo Grande (frío extremo - calentar fuerte)
float E_PG[] = { 5, 10, 15, 15, 'R' };  

// ===Conjuntos para Derivada=== ///
// Derivada Negativa (enfriándose rápido)
float D_N[] = { -5, -5, -2, -0.5, 'R' };     
// Derivada Cero (estable)
float D_C[] = { -1, 0, 1, 'T' };             
// Derivada Positiva (calentándose rápido)
float D_P[] = { 0.5, 2, 5, 5, 'R' };  

// ===Conjuntos para Potencia=== ///
// Potencia Máxima (100% PWM)
// Potencia Alta (70-100%)
float P_A[] = { 60, 85, 100, 'T' };           
// Potencia Media (40-70%)
float P_MD[] = { 30, 50, 70, 'T' };          
// Potencia Baja (10-40%)
float P_B[] = { 5, 20, 35, 'T' };            
// Potencia Mínima (0-10%)
float P_AP[] = { 0, 0, 10, 'T' };   

float paso = 1.0;
const int tam = controlfuzzy.calc_size(POTENCIA_RANGE, paso);

// ===== Funciones Principales ----=====
float error, derivada, potencia, temperaturass;
float tempTruncada;
int SetPoint;
bool controlActivo = false;
float Tlut;

int setPoint = 0;  // SetPoint original
bool setPointInicialLeido = false;
bool nuevoSetPointLeido = false;

//=== Verificacion del SetPoint === //
const int MIN_SETPOINT = 25;
const int MAX_SETPOINT = 40;

enum EstadosSistema { INICIO, LISTO, FALLO, CONTROL_ACTIVO };
EstadosSistema estadoSistema = INICIO;

// ========= PIN CONFIG =============
// Pines para sensor y actuador
const int pinPWM = 9;     // Pin PWM para potencia
const int pinDIR = 8;      // Pin de dirección del puente H
const int RELE_MOTOBOMBA = 4;    // Pin para el relé de la motobomba
const int RELE_VENTILADORES = 5;  // Pin para el relé de los ventiladores
Adafruit_MAX31865 max = Adafruit_MAX31865(10, 11, 12, 13);  // sensor temperatura
pt100rtd PT100 = pt100rtd();

// ===== MAIN =====
void setup() {
  Serial.begin(9600);
  pinMode(pinPWM, OUTPUT);
  pinMode(pinDIR, OUTPUT);
  pinMode(RELE_MOTOBOMBA, OUTPUT);
  pinMode(RELE_VENTILADORES, OUTPUT);

  max.begin(MAX31865_3WIRE);
 
  // Inicialmente apagado
  digitalWrite(pinDIR, LOW);
  analogWrite(pinPWM, 0);
  apagarDispositivos(); 

}

void loop() {
  // Recepción de tramas
  if (Serial.available() > 0) {
    delay(10);
    tramaRecibida = Serial.readStringUntil('|');

    if (tramaRecibida[0] == ID) {
      mensaje = tramaRecibida[1];
      valor = tramaRecibida.substring(2).toInt();

      // Bandera para indicar que se está procesando un comando
      static bool comandoActivo = false;
    }

    switch (mensaje) {
      case 'R':  // Inicializar
        Serial.println(F("Iniciando verificación de componentes..."));
        
        if (verificarComponentes()) {
          Serial.println(F("OK: Todos los componentes funcionando"));
          Serial.println(F("Sistema listo para iniciar control"));
          estadoSistema = LISTO;
        } else {
          Serial.println(F("ERROR: Corregir fallos antes de continuar"));
          estadoSistema = FALLO;
        }

        Serial.println(F(" TR|"));
       break;

      case 'I':  // Iniciar control
        if (estadoSistema != LISTO) {
          Serial.println(F("ERROR: Ejecutar verificación (comando R) primero"));
        } else if (!setPointInicialLeido) {
          Serial.println(F("ERROR: Definir setpoint primero (comando S)"));
        } else if (!validarSetpoint(setPoint)) {
          Serial.println(F("ERROR: Setpoint fuera de rango"));
        } else {
          controlActivo = true;
          activarDispositivos(); 
          Serial.println(F("Control difuso iniciado correctamente"));
        }
        break;

      case 'S':  // Cambiar SetPoint
        if (validarSetpoint(valor)) {
          setPoint = valor;
          setPointInicialLeido = true;
          Serial.print(F("Setpoint actualizado: "));
          Serial.println(setPoint);
        }
       break;

      case 'F':  // Finalizar control
        controlActivo = false;
        aplicarPotencia(0);  // Apagar actuador
          apagarDispositivos(); 
        Serial.println(F("Control finalizado"));
        break;

      case 'A':  // Solicitar valor actual
        Serial.print(F("Potencia actual: "));
        Serial.print(setPoint);
        Serial.println(potencia);
        break;
    }
    mensaje = ' ';
  }

  // Lógica del control difuso si está activo
  if (controlActivo) {
    static float errorAnterior = 0.0;
    static unsigned long lastTime = 0;
    unsigned long now = millis();
    
    float dt = (now - lastTime) / 1000.0; // Tiempo en segundos
    lastTime = now;

    tempTruncada = leerSensor();  // Lectura real del sensor

    error = (float)setPoint - tempTruncada;

    if (abs(error) > 20) { // Margen mayor que el rango de operación
      Serial.print(F("ERROR CRÍTICO: Diferenica temperatura "));
      Serial.print(abs(error));
      Serial.println(F("°C excede límites del sistema"));
      aplicarPotencia(0);
      controlActivo = false;
      return;
    }
    derivada = (error - errorAnterior) / dt; // Derivada normalizada
    errorAnterior = error;

    potencia = calcularPotencia(error, derivada);
    aplicarPotencia(potencia);  // Control deL actuador 

    Serial.print(F("Temp: ")); Serial.print(tempTruncada, 2);
    Serial.print(F(" | Error: ")); Serial.print(error, 2);
    Serial.print(F(" | Derivada: ")); Serial.print(derivada, 2);
    Serial.print(F(" | Potencia: ")); Serial.print(potencia, 2);
    Serial.print(F("% | Modo: ")); Serial.print(error > 0 ? "CALENTAR" : "ENFRIAR");
    Serial.print(F(" | PWM: ")); Serial.println(map(abs(potencia), 0, 100, 0, 255));    

    delay(200); // delay para estabilidad
  }
}

// Función de control difuso
float calcularPotencia(float error, float derivada) {
  float B[tam];
  controlfuzzy.inicio(B, tam);

  controlfuzzy.regla_compuesta2(E_NG, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_A, POTENCIA_RANGE, B, tam);  // Regla 1
  controlfuzzy.regla_compuesta2(E_NP, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_A, POTENCIA_RANGE, B, tam);  // Regla 2
  controlfuzzy.regla_compuesta2(E_C, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_AP, POTENCIA_RANGE, B, tam);    // Regla 3
  controlfuzzy.regla_compuesta2(E_PP, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);   // Regla 4
  controlfuzzy.regla_compuesta2(E_PG, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_A, POTENCIA_RANGE, B, tam);   // Regla 5
  
  controlfuzzy.regla_compuesta2(E_NG, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);    // Regla 6
  controlfuzzy.regla_compuesta2(E_NP, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);   // Regla 7
  controlfuzzy.regla_compuesta2(E_C, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_AP, POTENCIA_RANGE, B, tam);  // Regla 8
  controlfuzzy.regla_compuesta2(E_PP, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);   // Regla 9
  controlfuzzy.regla_compuesta2(E_PG, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);   // Regla 10
  
  controlfuzzy.regla_compuesta2(E_NG, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);   // Regla 11
  controlfuzzy.regla_compuesta2(E_NP, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);   // Regla 12
  controlfuzzy.regla_compuesta2(E_C, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_AP, POTENCIA_RANGE, B, tam);   // Regla 13
  controlfuzzy.regla_compuesta2(E_PP, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);  // Regla 14
  controlfuzzy.regla_compuesta2(E_PG, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);  // Regla 15

  return controlfuzzy.defusi(B, POTENCIA_RANGE, tam);
}

// Lectura sensor Temp
float leerSensor() {
  uint16_t rtd, ohmsx100;
  uint32_t dummy;
  float Tlut;
  rtd = max.readRTD();
  dummy = ((uint32_t)(rtd << 1)) * 100 * ((uint32_t)floor(RREF));
  dummy >>= 16;
  ohmsx100 = (uint16_t)(dummy & 0xFFFF);
  Tlut = PT100.celsius(ohmsx100);
  return Tlut;
}

// Aplicar potencia con PWM y dirección al puente H
void aplicarPotencia(float potencia) {
  // Determinar dirección (CALENTAR o ENFRIAR)
  if (error > 0.1) { // Margen pequeño para evitar oscilaciones
    digitalWrite(pinDIR, HIGH); // Modo CALENTAR
  } 
  else if (error < -0.1) {
    digitalWrite(pinDIR, LOW); // Modo ENFRIAR
  }
  
  // Protección: no cambiar dirección con potencia aplicada
  static bool lastDir;
  bool currentDir = digitalRead(pinDIR);
  if (currentDir != lastDir && abs(potencia) > 5) {
    analogWrite(pinPWM, 0); // Apagar momentáneamente
    delay(100); // Pequeño retardo para protección
  }
  lastDir = currentDir;
  
  // Aplicar PWM (siempre positivo)
  int pwm = map(abs(potencia), 0, 100, 0, 255);
  analogWrite(pinPWM, pwm);
}

// Función de validación
bool validarSetpoint(int sp) {
  if (sp >= MIN_SETPOINT && sp <= MAX_SETPOINT) {
    return true;
  }
  mostrarAyudaSetpoint();
  return false;
}
void mostrarAyudaSetpoint() {
  Serial.println(F("\n=== AYUDA ==="));
  Serial.println(F("El setpoint debe estar entre 25 y 40°C"));
  Serial.println(F("Ejemplo válido: T S 30| (para 30°C)"));
  Serial.println(F("Formato: T S XX| (XX = valor 2 dígitos)"));
}
bool verificarComponentes() {
  bool todosOK = true;
  
  // 1. Verificar sensor MAX31865
  if (!max.begin(MAX31865_3WIRE)) {
    Serial.println(F("ERROR: Sensor MAX31865 no responde"));
    todosOK = false;
  } else {
    // Lectura de prueba
    uint16_t rtd = max.readRTD();
    if (rtd == 0 || rtd == 0xFFFF) {
      Serial.println(F("ERROR: Lectura inválida del sensor"));
      todosOK = false;
    }
  }
  return todosOK;
}

void activarDispositivos() {
  digitalWrite(RELE_MOTOBOMBA, LOW);     // Activación motobomba (LOW para relés activos por bajo)
  digitalWrite(RELE_VENTILADORES, LOW);  // Activación ventiladores
  Serial.println(F("Dispositivos activados"));
  delay(1500);
}

void apagarDispositivos() {
  digitalWrite(RELE_MOTOBOMBA, HIGH);    // Desactivación motobomba
  digitalWrite(RELE_VENTILADORES, HIGH); // Desactivación ventiladores
  Serial.println(F("Dispositivos apagados"));
}
