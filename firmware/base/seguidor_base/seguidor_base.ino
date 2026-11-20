// =====================================================================
//  VERSION BASE (original del grupo) - NO MODIFICAR
//  Seguidor de sonido con NodeMCU ESP8266 + motor 28BYJ-48 (ULN2003)
//  La version con app, modo manual y limite de giro esta en
//  firmware/modificado/seguidor_app/seguidor_app.ino
// =====================================================================
#include <Stepper.h>

// ---------- Pines ----------
const uint8_t PIN_MIC_IZQ = D1;
const uint8_t PIN_MIC_DER = D2;
const uint8_t PIN_BOTON   = D5;
const uint8_t PIN_LED     = D4;               // GPIO2, activo en LOW

// ---------- Ajustes de geometria ----------
const float SEPARACION_M     = 0.20;          // distancia entre micrófonos (metros)
const float VEL_SONIDO       = 343.0;         // m/s
const long  ZONA_MUERTA_US   = 60;            // diferencia menor a esto = sonido al frente
const unsigned long VENTANA_US = 2000;        // tiempo maximo de espera al 2do micrófono
const float ANGULO_UNICO     = 45.0;          // grados si solo reacciona un micrófono
const int   SENTIDO          = 1;             // cambiar a -1 si gira al lado contrario

// ---------- Motor ----------
const int PASOS_POR_VUELTA = 2048;
Stepper motor(PASOS_POR_VUELTA, D0, D7, D6, D8);   // orden IN1, IN3, IN2, IN4

const unsigned long ESPERA_POST_MS = 800;
const unsigned long DEBOUNCE_MS    = 40;

// ---------- Estado ----------
volatile uint32_t tIzq = 0, tDer = 0;         // instante (us) en que reaccionó cada micrófono
bool escuchando = false;
bool lastReading = HIGH, stableState = HIGH;
unsigned long lastChange = 0;

void IRAM_ATTR isrIzq() { if (!tIzq) tIzq = micros(); }
void IRAM_ATTR isrDer() { if (!tDer) tDer = micros(); }

void setLed(bool on) { digitalWrite(PIN_LED, on ? LOW : HIGH); }

void apagarBobinas() {
  digitalWrite(D0, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
}

void limpiarTiempos() {
  noInterrupts();
  tIzq = 0;
  tDer = 0;
  interrupts();
}

void revisarBoton() {
  bool reading = digitalRead(PIN_BOTON);
  if (reading != lastReading) {
    lastChange = millis();
    lastReading = reading;
  }
  if (millis() - lastChange > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) {
      escuchando = !escuchando;
      setLed(escuchando);
      limpiarTiempos();
      Serial.println(escuchando ? "Escucha ACTIVADA" : "Escucha DESACTIVADA");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  setLed(false);
  pinMode(PIN_MIC_IZQ, INPUT);
  pinMode(PIN_MIC_DER, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_MIC_IZQ), isrIzq, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_MIC_DER), isrDer, RISING);
  motor.setSpeed(10);
  Serial.println("\nListo. Presiona el boton para activar la escucha.");
}

void loop() {
  revisarBoton();

  if (!escuchando) {
    limpiarTiempos();
    return;
  }

  noInterrupts();
  uint32_t a = tIzq, b = tDer;
  interrupts();

  if (!a && !b) return;                        // todavia no hay sonido

  bool ambos = a && b;
  uint32_t primero = ambos ? min(a, b) : (a ? a : b);
  if (!ambos && (micros() - primero) < VENTANA_US) return;   // espero al otro micrófono

  float angulo;                                // positivo = izquierda
  if (ambos) {
    long dt = (long)(int32_t)(b - a);          // >0 : llegó primero al izquierdo
    if (abs(dt) < ZONA_MUERTA_US) {
      Serial.println("Sonido al frente, no giro");
      delay(ESPERA_POST_MS);
      limpiarTiempos();
      return;
    }
    float s = (VEL_SONIDO * dt * 1e-6f) / SEPARACION_M;
    s = constrain(s, -1.0f, 1.0f);
    angulo = asin(s) * 180.0f / PI;
    Serial.printf("dt = %ld us -> angulo = %.1f grados\n", dt, angulo);
  } else {
    angulo = a ? ANGULO_UNICO : -ANGULO_UNICO;
    Serial.printf("Solo reacciono el micrófono %s -> %.1f grados\n",
                  a ? "izquierdo" : "derecho", angulo);
  }

  int pasos = (int)(angulo / 360.0f * PASOS_POR_VUELTA) * SENTIDO;
  if (pasos != 0) {
    motor.step(pasos);
    apagarBobinas();
  }

  delay(ESPERA_POST_MS);                       // el motor tambien hace ruido
  limpiarTiempos();
}
