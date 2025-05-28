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

float ERROR_RANGE[] = { -35, 35 };
float DERIVADA_RANGE[] = { -5, 5 };
float POTENCIA_RANGE[] = { 0, 100 };

// Conjuntos difusos para Error
float E_NG[] = { -35, -35, -25, -10, 'R' };
float E_NP[] = { -15, -9, -3, 'T' };
float E_C[] = { -5, 0, 5, 'T' };
float E_PP[] = { 3, 9, 15, 'T' };
float E_PG[] = { 10, 25, 35, 35, 'R' };

// Conjuntos para Derivada
float D_N[] = { -1, -1, -0.5, -0.25, 'R' };
float D_C[] = { -0.5, 0, 0.5, 'T' };
float D_P[] = { 0.25, 0.5, 1, 1, 'R' };

// Conjuntos para Potencia
float P_M[] = { 77, 85, 100, 100, 'R' };
float P_A[] = { 57, 68, 80, 'T' };
float P_MD[] = { 25, 42, 60, 'T' };
float P_B[] = { 7, 18, 30, 'T' };
float P_AP[] = { 0, 0, 10, 'T' };

float paso = 1.0;
const int tam = controlfuzzy.calc_size(POTENCIA_RANGE, paso);


// ===== Funciones Principales ----=====
float error, derivada, potencia, temperaturass;
int tempTruncada = (int)temperaturass;
int SetPoint;
bool controlActivo = false;
float Tlut;

int setPoint = 0;  // SetPoint original

bool setPointInicialLeido = false;
bool nuevoSetPointLeido = false;
// ========= PIN CONFIG =============
// Pines para sensor y actuador
const int pinActuador = 9;                                  // Actuador PWM
Adafruit_MAX31865 max = Adafruit_MAX31865(10, 11, 12, 13);  // sensor temperatura
pt100rtd PT100 = pt100rtd();

// ==============================

// ===== MAIN =====
void setup() {
  Serial.begin(9600);
  pinMode(pinActuador, OUTPUT);
  max.begin(MAX31865_3WIRE);
}

void loop() {
  // Recepción de tramas
  if (Serial.available() > 0) {
    delay(10);
    tramaRecibida = Serial.readStringUntil('|');

    if (tramaRecibida[0] == ID) {
      mensaje = tramaRecibida[1];


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
    int temperaturaEntera = (int)leerSensor();
    tempTruncada = leerSensor();  // Lectura real del sensor

    error = (float)setPoint - tempTruncada;


    derivada = calcularDerivada(error);
    potencia = calcularPotencia(error, derivada);
    aplicarPotencia(potencia);  // Control de actuador real
    calcularDerivada(error);
    Serial.print("Error: "); Serial.println(error);
    Serial.print("Derivada: "); Serial.println(derivada);

    
  delay(500);
  }

}


// Función de control difuso
float calcularPotencia(float error, float derivada) {
  float B[tam];
  controlfuzzy.inicio(B, tam);

  // Aplicación de 15 reglas
  controlfuzzy.regla_compuesta2(E_NG, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_M, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_NP, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_A, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_C, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_MD, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_PP, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_PG, D_N, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_AP, POTENCIA_RANGE, B, tam);

  controlfuzzy.regla_compuesta2(E_NG, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_M, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_NP, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_A, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_C, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_MD, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_PP, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_PG, D_C, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_AP, POTENCIA_RANGE, B, tam);

  controlfuzzy.regla_compuesta2(E_NG, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_M, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_NP, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_A, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_C, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_MD, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_PP, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_B, POTENCIA_RANGE, B, tam);
  controlfuzzy.regla_compuesta2(E_PG, D_P, ERROR_RANGE, DERIVADA_RANGE, error, derivada, P_AP, POTENCIA_RANGE, B, tam);

  return controlfuzzy.defusi(B, POTENCIA_RANGE, tam);
}

// Lectura real del sensor (A0)
float leerSensor() {
  uint16_t rtd, ohmsx100;
  uint32_t dummy;
  float Tlut;
  rtd = max.readRTD();
  // Use uint16_t (ohms * 100) since it matches data type in lookup table.
  dummy = ((uint32_t)(rtd << 1)) * 100 * ((uint32_t)floor(RREF));
  dummy >>= 16;
  ohmsx100 = (uint16_t)(dummy & 0xFFFF);
  Tlut = PT100.celsius(ohmsx100);  // NoobNote: LUT== LookUp Table
  Serial.print(F("Temperatura   = "));
  Serial.print(Tlut, 3);
  Serial.println(F(" °C"));
  return Tlut;
}

// Derivada simple del error
float calcularDerivada(float nuevoError) {
  static float errorAnterior = 0;
  float derivada = nuevoError - errorAnterior;
  errorAnterior = nuevoError;
  delay(200);
  return derivada;
} 

// Aplicar potencia con PWM al actuador
void aplicarPotencia(float p) {
  int salidaPWM = map(p, 0, 100, 0, 255);
  //analogWrite(pinActuador, salidaPWM);

  Serial.print("Aplicando potencia PWM: ");
  Serial.println(salidaPWM);
}
