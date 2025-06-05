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

float ERROR_RANGE[] = { -20, 20 };
float DERIVADA_RANGE[] = { -5, 5 };
float POTENCIA_RANGE[] = { 0, 100 };

// ===Conjuntos difusos para Error=== ///
// Error Negativo Grande (calor extremo - necesitamos enfriar)
float E_NG[] = { -20, -20, -15, -10, 'R' };  
// Error Negativo Pequeño (calor moderado - enfriar suave)
float E_NP[] = { -12, -8, -4, 'T' };         
// Error Cero (en setpoint)
float E_C[] = { -2, 0, 2, 'T' };             
// Error Positivo Pequeño (frío moderado - calentar suave)
float E_PP[] = { 4, 8, 12, 'T' };            
// Error Positivo Grande (frío extremo - calentar fuerte)
float E_PG[] = { 10, 15, 20, 20, 'R' };   

// ===Conjuntos para Derivada=== ///
// Derivada Negativa (enfriándose rápido)
float D_N[] = { -5, -5, -2, -0.5, 'R' };     
// Derivada Cero (estable)
float D_C[] = { -1, 0, 1, 'T' };             
// Derivada Positiva (calentándose rápido)
float D_P[] = { 0.5, 2, 5, 5, 'R' };  

// ===Conjuntos para Potencia=== ///
// Potencia Máxima (100% PWM)
float P_M[] = { 80, 90, 100, 100, 'R' };     
// Potencia Alta (70-90%)
float P_A[] = { 60, 75, 85, 'T' };           
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

// ========= PIN CONFIG =============
// Pines para sensor y actuador
const int pinPWM = 9;     // Pin PWM para potencia
const int pinDIR = 8;      // Pin de dirección del puente H
Adafruit_MAX31865 max = Adafruit_MAX31865(10, 11, 12, 13);  // sensor temperatura
pt100rtd PT100 = pt100rtd();

// ===== MAIN =====
void setup() {
  Serial.begin(9600);
  pinMode(pinPWM, OUTPUT);
  pinMode(pinDIR, OUTPUT);
  max.begin(MAX31865_3WIRE);
  
  // Inicialmente apagado
  digitalWrite(pinDIR, LOW);
  analogWrite(pinPWM, 0);
}

void loop() {
  // Recepción de tramas
  if (Serial.available() > 0) {
    delay(10);
    tramaRecibida = Serial.readStringUntil('|');

    if (tramaRecibida[0] == ID) {
      mensaje = tramaRecibida[1];
      valor = 100 * (tramaRecibida[2] - '0') + 10 * (tramaRecibida[3] - '0') + (tramaRecibida[4] - '0');

      Serial.flush();  //Limpiar buffer de entrada
    }

    switch (mensaje) {
      case 'R':  // Inicializar
        leerSensor();
        Serial.println("Control listo");
        break;

      case 'I':  // Iniciar control
        if (!setPointInicialLeido) {
          Serial.println(F("No se ha definido el SetPoint. Usa el comando 'S' primero."));
        } else {
          Serial.println(F("Control iniciado con lógica difusa."));
          controlActivo = true;
        }
        break;

      case 'S':  // Cambiar SetPoint
        setPoint = valor;
        setPointInicialLeido = true;
        Serial.print(F("Setpoint actual: "));
        Serial.println(setPoint);
        break;

      case 'F':  // Finalizar control
        controlActivo = false;
        aplicarPotencia(0);  // Apagar actuador
        Serial.println("Control finalizado y potencia desactivada");
        break;

      case 'A':  // Solicitar valor actual
        Serial.print("Potencia actual: ");
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
    derivada = (error - errorAnterior) / dt; // Derivada normalizada
    errorAnterior = error;

    potencia = calcularPotencia(error, derivada);
    aplicarPotencia(potencia);  // Control de actuador real

    Serial.print("Temp: "); Serial.print(tempTruncada, 2);
    Serial.print(" | Error: "); Serial.print(error, 2);
    Serial.print(" | Derivada: "); Serial.print(derivada, 2);
    Serial.print(" | Potencia: "); Serial.print(potencia, 2);
    Serial.print("% | Modo: "); Serial.print(error > 0 ? "CALENTAR" : "ENFRIAR");
    Serial.print(" | PWM: "); Serial.println(map(abs(potencia), 0, 100, 0, 255));    

    delay(200); // Pequeño delay para estabilidad
  }
}

// Función de control difuso
float calcularPotencia(float error, float derivada) {
  float B[tam];
  controlfuzzy.inicio(B, tam);

  // Reglas para CALENTAR (error positivo)
  controlfuzzy.regla_simple(E_PG, ERROR_RANGE, error, P_M, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_PP, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_A, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_PP, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_MD, POTENCIA_RANGE, B, tam);

  // Reglas para ENFRIAR (error negativo)
  controlfuzzy.regla_simple(E_NG, ERROR_RANGE, error, P_M, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_NP, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_A, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_NP, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_MD, POTENCIA_RANGE, B, tam);

  // Reglas para error CERO (estabilidad)
  controlfuzzy.regla_compuesta2(E_C, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);

  return controlfuzzy.defusi(B, POTENCIA_RANGE, tam);
}

// Lectura real del sensor
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

//fdfd