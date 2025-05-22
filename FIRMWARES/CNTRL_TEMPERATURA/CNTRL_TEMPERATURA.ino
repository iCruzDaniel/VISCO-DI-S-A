// ====== COM CONFIG ======

// --- Variables del dispositivo ----
char ID = 'T';  // Identificador del dispositivo (CNTRL DE TEMPERATURA)
char mensaje = 'N';
int valor = 0;
String tramaRecibida;

// ===== Librería de Lógica Difusa =====
#include <Fuzzy.h>

Fuzzy *fuzzy = new Fuzzy();

// ===== Configuración de Fuzzy =====
void configurarFuzzy() {
  // Entrada: Error [-35, 35]
  FuzzyInput *error = new FuzzyInput(1);
  FuzzySet *NG = new FuzzySet(-35, -35, -25, -15);
  FuzzySet *NP = new FuzzySet(-25, -15, -10, 0);
  FuzzySet *C = new FuzzySet(-5, 0, 0, 5);
  FuzzySet *PP = new FuzzySet(0, 10, 15, 25);
  FuzzySet *PG = new FuzzySet(15, 25, 35, 35);
  error->addFuzzySet(NG);
  error->addFuzzySet(NP);
  error->addFuzzySet(C);
  error->addFuzzySet(PP);
  error->addFuzzySet(PG);
  fuzzy->addFuzzyInput(error);

  // Entrada: Derivada Error [-1, 1]
  FuzzyInput *derivada = new FuzzyInput(2);
  FuzzySet *N = new FuzzySet(-1, -1, -0.5, 0);
  FuzzySet *CE = new FuzzySet(-0.25, 0, 0, 0.25); // C ya usado
  FuzzySet *P = new FuzzySet(0, 0.5, 1, 1);
  derivada->addFuzzySet(N);
  derivada->addFuzzySet(CE);
  derivada->addFuzzySet(P);
  fuzzy->addFuzzyInput(derivada);

  // Salida: Potencia [0, 100]
  FuzzyOutput *potencia = new FuzzyOutput(1);
  FuzzySet *M = new FuzzySet(80, 100, 100, 100);
  FuzzySet *A = new FuzzySet(60, 75, 85, 100);
  FuzzySet *MD = new FuzzySet(40, 50, 60, 70);
  FuzzySet *B = new FuzzySet(20, 30, 40, 50);
  FuzzySet *AP = new FuzzySet(0, 0, 10, 20);
  potencia->addFuzzySet(M);
  potencia->addFuzzySet(A);
  potencia->addFuzzySet(MD);
  potencia->addFuzzySet(B);
  potencia->addFuzzySet(AP);
  fuzzy->addFuzzyOutput(potencia);

  // Reglas difusas (1-15)
  fuzzy->addFuzzyRule(new FuzzyRule(1, new FuzzyRuleAntecedent(NG, N), new FuzzyRuleConsequent(M)));
  fuzzy->addFuzzyRule(new FuzzyRule(2, new FuzzyRuleAntecedent(NP, N), new FuzzyRuleConsequent(A)));
  fuzzy->addFuzzyRule(new FuzzyRule(3, new FuzzyRuleAntecedent(C, N), new FuzzyRuleConsequent(MD)));
  fuzzy->addFuzzyRule(new FuzzyRule(4, new FuzzyRuleAntecedent(PP, N), new FuzzyRuleConsequent(B)));
  fuzzy->addFuzzyRule(new FuzzyRule(5, new FuzzyRuleAntecedent(PG, N), new FuzzyRuleConsequent(AP)));

  fuzzy->addFuzzyRule(new FuzzyRule(6, new FuzzyRuleAntecedent(NG, CE), new FuzzyRuleConsequent(M)));
  fuzzy->addFuzzyRule(new FuzzyRule(7, new FuzzyRuleAntecedent(NP, CE), new FuzzyRuleConsequent(A)));
  fuzzy->addFuzzyRule(new FuzzyRule(8, new FuzzyRuleAntecedent(C, CE), new FuzzyRuleConsequent(MD)));
  fuzzy->addFuzzyRule(new FuzzyRule(9, new FuzzyRuleAntecedent(PP, CE), new FuzzyRuleConsequent(B)));
  fuzzy->addFuzzyRule(new FuzzyRule(10, new FuzzyRuleAntecedent(PG, CE), new FuzzyRuleConsequent(AP)));

  fuzzy->addFuzzyRule(new FuzzyRule(11, new FuzzyRuleAntecedent(NG, P), new FuzzyRuleConsequent(M)));
  fuzzy->addFuzzyRule(new FuzzyRule(12, new FuzzyRuleAntecedent(NP, P), new FuzzyRuleConsequent(A)));
  fuzzy->addFuzzyRule(new FuzzyRule(13, new FuzzyRuleAntecedent(C, P), new FuzzyRuleConsequent(MD)));
  fuzzy->addFuzzyRule(new FuzzyRule(14, new FuzzyRuleAntecedent(PP, P), new FuzzyRuleConsequent(B)));
  fuzzy->addFuzzyRule(new FuzzyRule(15, new FuzzyRuleAntecedent(PG, P), new FuzzyRuleConsequent(AP)));
}
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
  configurarFuzzy();
}

void loop() {
  if (Serial.available() > 0) {
    delay(10);
    tramaRecibida = Serial.readStringUntil('|');

    if (tramaRecibida[0] == ID) {
      mensaje = tramaRecibida[1];
      valor = 100 * (tramaRecibida[2] - '0') + 10 * (tramaRecibida[3] - '0') + (tramaRecibida[4] - '0');
      Serial.flush();
    }

    switch (mensaje) {
      case 'R':
        Serial.println("mensaje 1");
        break;

      case 'I':
        Serial.println("mensaje 2");
        break;

      case 'S':
        Serial.println("mensaje 3");
        break;

      case 'F':
        Serial.println("mensaje 4");
        break;

      case 'A':
        Serial.println("mensaje 5");
        break;
    }
    mensaje = ' ';
  }
}