// ====== COM CONFIG ======

// --- Varibles del dispositivo ----
char ID = 'C';  //Identificador del dispositivo (CNTRL de caida)
char mensaje = 'N';
int valor = 0;

// --- Cadena para recibir trama ----
String tramaRecibida;
// ==============================


// ========= PIN CONFIG =============
const byte PIN_PULSE = A0;
const byte PIN_CAP = A1;
const byte PIN_LED = 2;
// const byte PIN_BUTTON = 2;
const byte IMAN = 3;
// ---------------------------------
// ==============================


// ===== Contantes =====
// ----------- Parametros de calibracion sensor de metales -------------
const byte NPULSE = 8;
const int NMEAS = 30;
const byte CHARGE_DELAY_US = 2;
const byte DISCHARGE_DELAY_US = 10;
const byte EMA_ALPHA_SHIFT = 6;
const int THRESHOLD_MARGIN = 55;
// ---------------------------------
// ==============================


// ===== Variables Globales =====
// ------------ Sensor de Metales --------------
bool systenInit = false;
long avgFiltered = 0;
long threshold = 0;
volatile bool recalibrate = false;
byte pasoEstado = 0;                        // 0 = esperando bobina 1, 1 = contando hasta bobina 2
const unsigned long TIEMPO_LIMITE = 10000;  // Máximo 10 segundos entre pasos
// ---------------------------------
unsigned long T0 = 0;
unsigned long T1;
unsigned long T2;
// ==============================


// ===== Funciones Principales ----=====
// ----------- Inicializar elementos del sistema --------------
void InitSystem() {
  CalibrateMetalSensor();
  Serial.println("CR|");
}
// ----------- Iniciar Caida --------------
void InitFall() {
  Serial.println(F("Iniciando caída..."));

  // Suelta la esfera
  digitalWrite(IMAN, LOW);  // Desactiva electroimán para liberar esfera
  unsigned long T0 = millis();
  pasoEstado = 0;
  avgFiltered = 0;

  while (true) {
    unsigned long ahora = millis();  // Actualizado en cada ciclo

    long sum = 0;
    int minVal = 1023, maxVal = 0;

    for (int i = 0; i < NMEAS + 2; i++) {
      dischargeCapacitor();

      for (int j = 0; j < NPULSE; j++) {
        digitalWrite(PIN_PULSE, HIGH);
        delayMicroseconds(CHARGE_DELAY_US);
        digitalWrite(PIN_PULSE, LOW);
        delayMicroseconds(CHARGE_DELAY_US);
      }

      int val = analogRead(PIN_CAP);
      sum += val;
      minVal = min(minVal, val);
      maxVal = max(maxVal, val);
    }

    sum -= minVal;
    sum -= maxVal;

    if (avgFiltered == 0) avgFiltered = sum;
    avgFiltered = ((avgFiltered << EMA_ALPHA_SHIFT) - avgFiltered + sum) >> EMA_ALPHA_SHIFT;

    long diff = sum - avgFiltered;

    if (diff < -threshold) {
      digitalWrite(PIN_LED, HIGH);

      if (pasoEstado == 0) {
        T1 = ahora - T0;
        Serial.println(F("Paso en Bobina 1 - T1 registrado"));
        pasoEstado = 1;
      } else if (pasoEstado == 1) {
        T2 = ahora - T0;
        Serial.print(F("aso en Bobina 2 - T2 registrado: "));
        Serial.print(T2 / 1000.0);
        Serial.println(F(" s"));
        pasoEstado = 0;
        break;  // Sale del bucle, ya se tiene T2
      }

      delay(500);  // Antirrebote
    } else {
      digitalWrite(PIN_LED, LOW);
    }

    if (pasoEstado == 1 && ahora - T0 > TIEMPO_LIMITE) {
      Serial.println(F("Tiempo máximo superado, reiniciando medición."));
      pasoEstado = 0;
      break;  // Sal de loop si pasa demasiado tiempo
    }

    delay(1);
  }

  // Opcional: vuelve a activar el electroimán si se repite el experimento
  digitalWrite(IMAN, HIGH);
}

// ==============================


// ===== Funciones Complementarias =====
// --------- Calibración de sensor detector de metales ----------
void CalibrateMetalSensor() {
  Serial.println(F("Iniciando calibración de sensor de metal..."));

  long sum = 0;
  int minVal = 1023, maxVal = 0;

  for (int i = 0; i < NMEAS + 2; i++) {
    dischargeCapacitor();

    for (int j = 0; j < NPULSE; j++) {
      digitalWrite(PIN_PULSE, HIGH);
      delayMicroseconds(CHARGE_DELAY_US);
      digitalWrite(PIN_PULSE, LOW);
      delayMicroseconds(CHARGE_DELAY_US);
    }

    int val = analogRead(PIN_CAP);
    sum += val;
    minVal = min(minVal, val);
    maxVal = max(maxVal, val);
  }

  sum -= minVal;
  sum -= maxVal;

  avgFiltered = sum;
  threshold = THRESHOLD_MARGIN;  // Solo se usa el margen

  Serial.print(F("✅ Umbral calibrado: "));
  Serial.println(threshold);
}

// ---------------------------------
// ==============================



// ===== MAIN =====
void setup() {
  Serial.begin(9600);

  //Pulsos de carga
  pinMode(PIN_PULSE, OUTPUT);
  digitalWrite(PIN_PULSE, LOW);
  // Pin de lectura al capacitor
  pinMode(PIN_CAP, INPUT);
  // Led indicador
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  // IMAN
  pinMode(IMAN, OUTPUT);
  digitalWrite(IMAN, HIGH);
  //Calibración Inicial
  CalibrateMetalSensor();
}


void loop() {

  // Leer la trama completa en el Arduino
  if (Serial.available() > 0) {
    delay(10);
    tramaRecibida = Serial.readStringUntil('|');  // Leer hasta un salto de línea

    if (tramaRecibida[0] == ID) {

      mensaje = tramaRecibida[1];
      valor = tramaRecibida.substring(2).toInt();


      Serial.flush();  //Limpiar buffer de entrada
    }

    // Ejecutar funcion correspondiente al mensaje
    switch (mensaje) {
      case 'R':
        // Respuesta de inicialización
        InitSystem();
        break;
      case 'I':
        Serial.println("CC|");
        InitFall();
        break;
      case 'S':
        // funcion
        Serial.println("mensaje 3");
        break;
    }
    mensaje = ' ';
    valor = 0;
  }
}

// === Descarga del Condensador ===
void dischargeCapacitor() {
  pinMode(PIN_CAP, OUTPUT);
  digitalWrite(PIN_CAP, LOW);
  delayMicroseconds(DISCHARGE_DELAY_US);
  pinMode(PIN_CAP, INPUT);
}