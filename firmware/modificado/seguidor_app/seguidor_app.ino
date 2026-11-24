// =====================================================================
//  VERSION MODIFICADA - Seguidor de sonido + App (Web Serial)
//  NodeMCU ESP8266 + 2x KY-037 + 28BYJ-48/ULN2003 + pulsador KY-004
//
//  Cambios respecto de la version base (firmware/base):
//   1. Envia lineas "@..." en formato JSON para que la app las grafique.
//      (Los mensajes de texto originales se mantienen para el monitor serie.)
//   2. Recibe comandos por USB: modo manual, ir a un angulo, girar, parar...
//   3. Lleva la cuenta de pasos -> sabe hacia donde apunta (0 = centro).
//   4. Limite de giro por software (por defecto +-120 grados) para no
//      enrollar los cables.
//   5. Funciona con los microfonos GIRANDO con el soporte (lazo cerrado)
//      o FIJOS (lazo abierto). Se elige desde la app o con comando.
//   6. El motor se mueve paso a paso y entre pasos se revisa si llego la
//      orden de PARAR (desde la app o desde el control IR opcional).
//   7. La pausa post-movimiento ya no bloquea: la app sigue respondiendo.
//   8. Control infrarrojo OPCIONAL (USAR_IR). Ver docs/modificado/OPCIONES_IR.md
//   9. Movimiento suavizado: ganancia < 1 y giro maximo por sonido, para que
//      un aplauso aislado no haga girar todo de una.
//  10. MODO VOZ (E1): en vez de medir tiempos compara la ACTIVIDAD (pulsos)
//      de cada micro, corregida por un factor de balance calibrado desde el
//      centro (comando I). Necesario porque con modulos de salida digital el
//      ataque lento de la voz hace inutil la diferencia de tiempos.
//      Ver docs/modificado/RESULTADOS_MICROFONOS.md
//
//  COMANDOS (terminar cada uno con Enter / salto de linea):
//   L1 / L0 / L  escucha ON / OFF / alternar
//   A            modo AUTOMATICO (sigue el sonido)
//   M            modo MANUAL (solo se mueve con ordenes; el sonido se informa)
//   G<grados>    ir a un angulo absoluto. Ej: G-30   (+ = izquierda)
//   J<grados>    girar relativo.          Ej: J5
//   C            volver al centro (G0)
//   X            PARAR el motor ya
//   Z            tomar la posicion actual como centro (0)
//   R            microfonos GIRAN con el soporte (lazo cerrado)
//   F            microfonos FIJOS (lazo abierto)
//   T<grados>    limite de giro. Ej: T120
//   K<valor>     ganancia (fraccion del error corregida por sonido). Ej: K0.5
//   P<grados>    giro maximo por sonido (aislado). Ej: P15
//   V<rpm>       velocidad del motor, 5 a 15. Ej: V12
//   W<ms>        pausa tras cada giro (ruido del motor). Ej: W500
//   Q1 / Q0      modo calibracion on/off (informa actividad de cada mic, no mueve)
//   O<us>        offset de dt (diferencia de sensibilidad). Ej: O35
//   D<metros>    separacion efectiva entre mics. Ej: D0.205
//   Y<us>        zona muerta. Ej: Y45
//   U1 / U0      usar / ignorar sonidos que solo oye UN micro (defecto U0 = ignorar)
//   E1 / E0      modo VOZ por actividad on / off (requiere mics que giran)
//   I            calibrar balance: hablar ~3 s desde el centro (luego S para guardar)
//   N<grados>    modo voz: giro maximo por decision (2 a 30). Ej: N10
//   H<factor>    balance manual = pulsos DER / pulsos IZQ al centro. Ej: H6
//   S            guardar calibracion en la memoria de la placa
//   B            volver a la calibracion de fabrica (luego S para guardar)
//   ?            pedir estado
// =====================================================================
#include <Stepper.h>
#include <EEPROM.h>

// ---------- Control IR opcional ----------
// 0 = sin IR (no hace falta instalar nada).
// 1 = con receptor IR KY-022 en D3. Requiere la libreria "IRremoteESP8266"
//     (Arduino IDE -> Herramientas -> Administrar bibliotecas).
#define USAR_IR 0

// ---------- Pines ----------
const uint8_t PIN_MIC_IZQ = D1;
const uint8_t PIN_MIC_DER = D2;
const uint8_t PIN_BOTON   = D5;
const uint8_t PIN_LED     = D4;               // GPIO2, activo en LOW

// ---------- Ajustes de geometria ----------
const float SEPARACION_M     = 0.137;         // distancia entre micrófonos (metros) ~ alto de un Galaxy J2
const float VEL_SONIDO       = 343.0;         // m/s
const long  ZONA_MUERTA_US   = 60;            // diferencia menor a esto = sonido al frente
const unsigned long VENTANA_US = 2000;        // tiempo maximo de espera al 2do micrófono
const float ANGULO_UNICO     = 45.0;          // grados si solo reacciona un micrófono
const int   SENTIDO          = 1;             // cambiar a -1 si gira al lado contrario
const float ZONA_MUERTA_DEG  = 6.0;           // correccion menor a esto = no mover
// Suavizado: cada sonido corrige solo una parte del error y nunca mas de MAX_GIRO.
// Un aplauso aislado mueve "un poco"; una voz sostenida (muchos disparos seguidos)
// va acercando el soporte de a poco hasta dejarlo apuntando.
float ganancia   = 0.5;      // fraccion del error que se corrige por sonido (0.1 a 1)
float maxGiroDeg = 15.0;     // giro maximo por sonido (grados)
// Racha: si llegan varios sonidos seguidos del MISMO lado (voz sostenida), cada
// uno puede corregir mas: tope = maxGiro * racha, ganancia += 0.25 por sonido.
// Un aplauso aislado (racha = 1) sigue moviendo poco.
const unsigned long VENTANA_RACHA_MS = 2000;  // max. tiempo entre sonidos de una racha
int   racha = 0;
int   ladoRacha = 0;
unsigned long ultimoSonidoMs = 0;

// ---------- Calibracion (se ajusta desde la app y se guarda en EEPROM) ----------
long  offsetUs     = 0;              // corrimiento de dt por diferencia de sensibilidad
float sepEfectiva  = SEPARACION_M;   // separacion "efectiva" medida (m)
long  zonaMuertaUs = ZONA_MUERTA_US; // zona muerta (us)
bool  modoCal      = false;          // en calibracion: informa pero NO mueve el motor
volatile uint32_t cntIzq = 0, cntDer = 0;   // flancos por micro (medidor de actividad)
unsigned long ultimoCnt = 0;
struct Calib { uint32_t magic; long offset; float sep; long zona; float balance; };
const uint32_t CAL_MAGIC_V1 = 0xCA1B0001;   // version sin balance
const uint32_t CAL_MAGIC    = 0xCA1B0002;

// ---------- Modo VOZ (por actividad) ----------
bool  modoVoz      = false;
float balanceDer   = 1.0;            // pulsos DER / pulsos IZQ con la fuente al centro
volatile uint32_t vozIzq = 0, vozDer = 0;
unsigned long ultimaVoz = 0;
const unsigned long VOZ_VENTANA_MS = 200;   // cada cuanto se compara
const uint32_t VOZ_MIN_PULSOS = 20;         // menos que esto (sumando) = silencio
const float    VOZ_ZONA       = 0.30;       // |desbalance| menor = al frente
float          vozPasoDeg     = 15.0;       // giro maximo por decision (comando N)
// Suavizado: se decide con la SUMA de las ultimas VOZ_N ventanas (media movil),
// asi un pico suelto de 200 ms no hace temblar el soporte.
const int      VOZ_N          = 3;
uint32_t histI[VOZ_N], histD[VOZ_N];
int histPos = 0, histLlenas = 0;
void vaciarHistVoz() { histPos = 0; histLlenas = 0; }
unsigned long calBalHasta = 0;              // > 0: calibrando balance
uint32_t calBalI = 0, calBalD = 0;

// ---------- Motor ----------
const int PASOS_POR_VUELTA = 2048;
Stepper motor(PASOS_POR_VUELTA, D0, D7, D6, D8);   // orden IN1, IN3, IN2, IN4

unsigned long esperaPostMs = 500;   // pausa tras girar (ruido del motor). Base: 800
int velRpm = 12;                    // velocidad del motor (5 a 15 rpm). Base: 10
const unsigned long DEBOUNCE_MS    = 40;
const unsigned long ESTADO_CADA_MS = 1000;

// ---------- Estado ----------
volatile uint32_t tIzq = 0, tDer = 0;         // instante (us) en que reaccionó cada micrófono
bool escuchando = false;
bool lastReading = HIGH, stableState = HIGH;
unsigned long lastChange = 0;

long  posPasos   = 0;        // posicion actual en pasos (+ = izquierda, 0 = centro)
bool  modoAuto   = true;     // true = sigue el sonido; false = manual
bool  micsGiran  = true;     // true = mics sobre la parte giratoria
bool  usarUnico  = false;    // false = descartar sonidos que oyo un solo micro
float limiteDeg  = 120.0;    // limite de giro a cada lado
bool  pedirStop  = false;
bool  moviendo   = false;
unsigned long ignorarHasta = 0;   // pausa post-movimiento (no bloqueante)
unsigned long ultimoEstado = 0;

char  bufSerie[32];
uint8_t lenSerie = 0;

#if USAR_IR
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
const uint16_t PIN_IR = D3;    // GPIO0: el receptor queda en HIGH en reposo, no traba el arranque
IRrecv irrecv(PIN_IR);
decode_results irRes;
// Completar con los codigos de SU control: con USAR_IR 1, apretar cada
// boton y copiar el numero que aparece en el monitor serie ("IR codigo = 0x...").
const uint64_t IR_STOP    = 0x0;
const uint64_t IR_IZQ     = 0x0;   // gira 10 grados a la izquierda
const uint64_t IR_DER     = 0x0;   // gira 10 grados a la derecha
const uint64_t IR_CENTRO  = 0x0;
const uint64_t IR_ESCUCHA = 0x0;   // escucha ON/OFF
const uint64_t IR_MODO    = 0x0;   // auto / manual
#endif

// =====================================================================
// Deteccion de ATAQUE: solo vale el primer pulso despues de un silencio de ese micro.
// Con una voz sostenida el comparador dispara en cada ciclo de la onda; sin esto
// cada micro "engancha" un ciclo distinto y el dt sale al azar (+-1000 us).
const uint32_t SILENCIO_US = 15000;
volatile uint32_t ultIzq = 0, ultDer = 0;     // ultimo pulso de cada micro
void IRAM_ATTR isrIzq() {
  uint32_t t = micros();
  if (!tIzq && t - ultIzq > SILENCIO_US) tIzq = t;
  ultIzq = t; cntIzq++; vozIzq++;
}
void IRAM_ATTR isrDer() {
  uint32_t t = micros();
  if (!tDer && t - ultDer > SILENCIO_US) tDer = t;
  ultDer = t; cntDer++; vozDer++;
}

void cargarCalib() {
  Calib c;
  EEPROM.get(0, c);
  if ((c.magic == CAL_MAGIC || c.magic == CAL_MAGIC_V1) && c.sep > 0.05f && c.sep < 1.0f) {
    offsetUs = c.offset; sepEfectiva = c.sep; zonaMuertaUs = c.zona;
    if (c.magic == CAL_MAGIC && c.balance > 0.05f && c.balance < 50.0f) balanceDer = c.balance;
    Serial.println("Calibracion cargada de la memoria");
  }
}

void guardarCalib() {
  Calib c = { CAL_MAGIC, offsetUs, sepEfectiva, zonaMuertaUs, balanceDer };
  EEPROM.put(0, c);
  EEPROM.commit();
  Serial.println("Calibracion GUARDADA");
  Serial.println("@SAVED {}");
}

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

float pasosAGrados(long p) { return p * 360.0f / PASOS_POR_VUELTA; }
long  gradosAPasos(float g) { return lroundf(g / 360.0f * PASOS_POR_VUELTA); }

void enviarEstado() {
  Serial.printf("@STATE {\"esc\":%d,\"auto\":%d,\"giran\":%d,\"pos\":%ld,"
                "\"deg\":%.1f,\"lim\":%.0f,\"sep\":%.2f,\"mov\":%d,\"ir\":%d,"
                "\"k\":%.2f,\"pmax\":%.0f,\"vel\":%d,\"pausa\":%lu,"
                "\"cal\":%d,\"off\":%ld,\"zona\":%ld,\"unico\":%d,\"voz\":%d,\"bal\":%.2f,\"vpaso\":%.0f}\n",
                escuchando, modoAuto, micsGiran, posPasos,
                pasosAGrados(posPasos), limiteDeg, sepEfectiva, moviendo, USAR_IR,
                ganancia, maxGiroDeg, velRpm, esperaPostMs,
                modoCal, offsetUs, zonaMuertaUs, usarUnico, modoVoz, balanceDer, vozPasoDeg);
  ultimoEstado = millis();
}

void setEscucha(bool on) {
  escuchando = on;
  setLed(escuchando);
  limpiarTiempos();
  Serial.println(escuchando ? "Escucha ACTIVADA" : "Escucha DESACTIVADA");
  enviarEstado();
}

// ---------- Entradas (se revisan tambien DURANTE el movimiento) ----------
void ejecutarComando(char *cmd);

void revisarSerie() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (lenSerie) {
        bufSerie[lenSerie] = 0;
        ejecutarComando(bufSerie);
        lenSerie = 0;
      }
    } else if (lenSerie < sizeof(bufSerie) - 1) {
      bufSerie[lenSerie++] = c;
    }
  }
}

void revisarBoton() {
  bool reading = digitalRead(PIN_BOTON);
  if (reading != lastReading) {
    lastChange = millis();
    lastReading = reading;
  }
  if (millis() - lastChange > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) setEscucha(!escuchando);
  }
}

void revisarIR() {
#if USAR_IR
  if (!irrecv.decode(&irRes)) return;
  uint64_t v = irRes.value;
  irrecv.resume();
  if (irRes.repeat) return;                   // ignorar "boton mantenido"
  Serial.printf("IR codigo = 0x%s\n", uint64ToString(v, 16).c_str());
  Serial.printf("@IR {\"code\":\"0x%s\"}\n", uint64ToString(v, 16).c_str());
  if      (v == IR_STOP    && v) ejecutarComando((char*)"X");
  else if (v == IR_IZQ     && v) ejecutarComando((char*)"J10");
  else if (v == IR_DER     && v) ejecutarComando((char*)"J-10");
  else if (v == IR_CENTRO  && v) ejecutarComando((char*)"C");
  else if (v == IR_ESCUCHA && v) ejecutarComando((char*)"L");
  else if (v == IR_MODO    && v) ejecutarComando((char*)(modoAuto ? "M" : "A"));
#endif
}

// ---------- Movimiento paso a paso con PARADA y LIMITE ----------
void moverPasos(long n) {
  long lim = gradosAPasos(limiteDeg);
  long destino = constrain(posPasos + n, -lim, lim);
  if (destino != posPasos + n) {
    Serial.println("Limite de giro alcanzado, se recorta el movimiento");
    Serial.printf("@LIMIT {\"lim\":%.0f}\n", limiteDeg);
  }
  n = destino - posPasos;
  if (n == 0) return;

  int dir = n > 0 ? 1 : -1;
  pedirStop = false;
  moviendo = true;
  Serial.printf("@MOVE {\"from\":%.1f,\"to\":%.1f}\n",
                pasosAGrados(posPasos), pasosAGrados(destino));

  for (long i = 0; i < labs(n); i++) {
    motor.step(dir * SENTIDO);                // un paso por vez
    posPasos += dir;
    if ((i & 15) == 0) {                      // cada 16 pasos (~45 ms)
      revisarSerie();
      revisarIR();
      if (pedirStop) {
        Serial.println("Movimiento DETENIDO");
        break;
      }
    }
    if ((i & 63) == 0) Serial.printf("@POS {\"pos\":%ld,\"deg\":%.1f}\n",
                                     posPasos, pasosAGrados(posPasos));
    yield();                                  // evita el reinicio por watchdog
  }
  apagarBobinas();
  moviendo = false;
  pedirStop = false;
  ignorarHasta = millis() + esperaPostMs;   // el motor tambien hace ruido
  enviarEstado();
}

void irAGrados(float g) { moverPasos(gradosAPasos(g) - posPasos); }

// ---------- Comandos ----------
void ejecutarComando(char *cmd) {
  char c = toupper(cmd[0]);
  float v = atof(cmd + 1);

  if (c == 'X') { pedirStop = true; Serial.println("Orden de PARAR"); return; }
  if (c == '?') { enviarEstado(); return; }
  if (moviendo) { Serial.println("@ERR {\"msg\":\"ocupado moviendo\"}"); return; }

  switch (c) {
    case 'L':
      if (cmd[1] == '1') setEscucha(true);
      else if (cmd[1] == '0') setEscucha(false);
      else setEscucha(!escuchando);
      break;
    case 'A': modoAuto = true;  Serial.println("Modo AUTOMATICO"); enviarEstado(); break;
    case 'M': modoAuto = false; Serial.println("Modo MANUAL");     enviarEstado(); break;
    case 'G': irAGrados(v); break;
    case 'J': moverPasos(gradosAPasos(v)); break;
    case 'C': irAGrados(0); break;
    case 'Z': posPasos = 0; Serial.println("Centro redefinido"); enviarEstado(); break;
    case 'R': micsGiran = true;  Serial.println("Microfonos GIRAN (lazo cerrado)"); enviarEstado(); break;
    case 'F': micsGiran = false; Serial.println("Microfonos FIJOS (lazo abierto)"); enviarEstado(); break;
    case 'T': limiteDeg = constrain(v, 10.0f, 180.0f); enviarEstado(); break;
    case 'K': ganancia = constrain(v, 0.1f, 1.0f); enviarEstado(); break;
    case 'P': maxGiroDeg = constrain(v, 2.0f, 90.0f); enviarEstado(); break;
    case 'V': velRpm = constrain((int)v, 5, 15); motor.setSpeed(velRpm); enviarEstado(); break;
    case 'W': esperaPostMs = constrain((long)v, 100L, 2000L); enviarEstado(); break;
    case 'Q':                                   // Q1 / Q0: modo calibracion
      modoCal = cmd[1] == '1';
      noInterrupts(); cntIzq = 0; cntDer = 0; interrupts();
      if (modoCal && !escuchando) setEscucha(true);
      Serial.println(modoCal ? "Modo CALIBRACION (el motor no se mueve)" : "Fin de calibracion");
      enviarEstado(); break;
    case 'O': offsetUs = constrain((long)v, -400L, 400L); enviarEstado(); break;
    case 'D': sepEfectiva = constrain(v, 0.05f, 1.0f); enviarEstado(); break;
    case 'Y': zonaMuertaUs = constrain((long)v, 10L, 300L); enviarEstado(); break;
    case 'U': usarUnico = (cmd[1] == '1');
              Serial.println(usarUnico ? "Se usan sonidos de un solo micro" : "Se ignoran sonidos de un solo micro");
              enviarEstado(); break;
    case 'E': modoVoz = (cmd[1] == '1');
              if (modoVoz && !micsGiran) Serial.println("AVISO: el modo voz necesita mics que giran (R)");
              Serial.println(modoVoz ? "Modo VOZ (por actividad)" : "Modo voz apagado");
              enviarEstado(); break;
    case 'N': vozPasoDeg = constrain(v, 2.0f, 30.0f); enviarEstado(); break;
    case 'H': if (v > 0.05f && v < 50.0f) balanceDer = v; enviarEstado(); break;
    case 'I': calBalI = 0; calBalD = 0; calBalHasta = millis() + 3000;
              if (!escuchando) setEscucha(true);
              Serial.println("Calibrando balance: habla desde el CENTRO durante 3 s..."); break;
    case 'S': guardarCalib(); break;
    case 'B': offsetUs = 0; sepEfectiva = SEPARACION_M; zonaMuertaUs = ZONA_MUERTA_US; balanceDer = 1.0;
              Serial.println("Calibracion restablecida (falta S para guardar)"); enviarEstado(); break;
    default:  Serial.printf("@ERR {\"msg\":\"comando desconocido: %s\"}\n", cmd);
  }
}

// ---------- Procesamiento del sonido ----------
void procesarSonido() {
  noInterrupts();
  uint32_t a = tIzq, b = tDer;
  interrupts();

  if (!a && !b) return;                        // todavia no hay sonido

  bool ambos = a && b;
  uint32_t primero = ambos ? min(a, b) : (a ? a : b);
  if (!ambos && (micros() - primero) < VENTANA_US) return;   // espero al otro micrófono

  long  dt = 0, dtCrudo = 0;
  float angRel;                                // angulo respecto del frente de los mics (+ = izq)
  const char *fuente;
  if (ambos) {
    dtCrudo = (long)(int32_t)(b - a);          // >0 : llegó primero al izquierdo
    dt = dtCrudo - offsetUs;                   // corregido por calibracion
    // El sonido no puede tardar mas que sep/c entre mics (~400 us con 13.7 cm).
    // Un dt mayor = un micro se disparo antes por ruido (sensibilidad muy alta)
    // o muy tarde (sensibilidad baja). Se descarta en vez de saturar a +-90.
    long dtMax = lroundf(sepEfectiva / VEL_SONIDO * 1e6f * 1.25f);
    if (labs(dt) > dtMax) {
      Serial.printf("dt = %ld us imposible (max %ld) -> descartado\n", dt, dtMax);
      ignorarHasta = millis() + 150;
      limpiarTiempos();
      return;
    }
    if (labs(dt) < zonaMuertaUs) {
      angRel = 0;
      Serial.println("Sonido al frente");
    } else {
      float s = (VEL_SONIDO * dt * 1e-6f) / sepEfectiva;
      s = constrain(s, -1.0f, 1.0f);
      angRel = asin(s) * 180.0f / PI;
      Serial.printf("dt = %ld us -> angulo = %.1f grados\n", dt, angRel);
    }
    fuente = "ambos";
  } else {
    if (!usarUnico && !modoCal) {              // tipico de sensibilidades desparejas
      Serial.printf("Solo reacciono el micro %s -> ignorado\n", a ? "izquierdo" : "derecho");
      ignorarHasta = millis() + 150;
      limpiarTiempos();
      return;
    }
    angRel = a ? ANGULO_UNICO : -ANGULO_UNICO;
    fuente = a ? "izq" : "der";
    Serial.printf("Solo reacciono el micrófono %s -> %.1f grados\n",
                  a ? "izquierdo" : "derecho", angRel);
  }

  // Direccion absoluta del sonido y correccion necesaria
  float posDeg = pasosAGrados(posPasos);
  float angAbs = micsGiran ? posDeg + angRel : angRel;
  float correccion = angAbs - posDeg;

  // Racha: sonidos seguidos del mismo lado = voz sostenida -> se permite corregir mas
  int lado = fabs(correccion) < ZONA_MUERTA_DEG ? 0 : (correccion > 0 ? 1 : -1);
  if (lado != 0 && lado == ladoRacha && millis() - ultimoSonidoMs < VENTANA_RACHA_MS)
    racha = min(racha + 1, 4);
  else
    racha = lado != 0 ? 1 : 0;
  ladoRacha = lado;

  Serial.printf("@SND {\"dt\":%ld,\"raw\":%ld,\"rel\":%.1f,\"abs\":%.1f,\"src\":\"%s\",\"pos\":%.1f,\"auto\":%d,\"racha\":%d}\n",
                dt, dtCrudo, angRel, angAbs, fuente, posDeg, modoAuto, racha);

  if (modoCal) {                               // calibrando: nunca mover
    ignorarHasta = millis() + 300;             // evita contar ecos del mismo aplauso
  } else if (modoAuto && lado != 0) {
    float giro;
    if (micsGiran) {
      // Lazo cerrado: el proximo sonido vuelve a medir el error, se corrige de a poco.
      float k    = min(1.0f, ganancia + 0.25f * (racha - 1));
      float tope = maxGiroDeg * racha;
      giro = constrain(correccion * k, -tope, tope);
    } else {
      // Lazo abierto: la medicion es absoluta y no habra otra que corrija el resto,
      // asi que se va directo al angulo medido (solo lo recorta el limite de giro).
      giro = correccion;
    }
    moverPasos(gradosAPasos(giro));
  } else {
    if (modoAuto) Serial.println("Sonido al frente, no giro");
    ignorarHasta = millis() + esperaPostMs;
  }
  ultimoSonidoMs = millis();                   // se mide desde que termino el giro
  limpiarTiempos();
}

// ---------- Modo VOZ: comparar actividad de cada micro ----------
void procesarVoz() {
  if (millis() - ultimaVoz < VOZ_VENTANA_MS) return;
  ultimaVoz = millis();
  noInterrupts();
  uint32_t ci = vozIzq, cd = vozDer;
  vozIzq = 0; vozDer = 0;
  interrupts();

  if (calBalHasta) {                           // calibrando balance desde el centro
    calBalI += ci; calBalD += cd;
    if ((long)(millis() - calBalHasta) >= 0) {
      calBalHasta = 0;
      if (calBalI < 30 || calBalD < 30) {
        Serial.printf("Balance: muy poca senal (IZQ %lu, DER %lu). Acercate o subi sensibilidad.%s",
                      (unsigned long)calBalI, (unsigned long)calBalD, "\n");
      } else {
        balanceDer = constrain((float)calBalD / calBalI, 0.05f, 50.0f);
        Serial.printf("Balance calibrado: DER/IZQ = %.2f (IZQ %lu, DER %lu). S para guardar.%s",
                      balanceDer, (unsigned long)calBalI, (unsigned long)calBalD, "\n");
      }
      enviarEstado();
    }
    return;
  }

  // En silencio no se suma: la historia se reinicia para no mezclar frases.
  if (ci + cd / balanceDer < VOZ_MIN_PULSOS / 2) { vaciarHistVoz(); return; }
  histI[histPos] = ci; histD[histPos] = cd;
  histPos = (histPos + 1) % VOZ_N;
  if (histLlenas < VOZ_N) histLlenas++;
  if (histLlenas < 2) return;                  // decidir ya con 2 ventanas (0,4 s), luego con VOZ_N
  ci = 0; cd = 0;
  for (int k = 0; k < histLlenas; k++) { ci += histI[k]; cd += histD[k]; }

  float dn = cd / balanceDer;                  // derecho normalizado
  float tot = ci + dn;
  if (tot < VOZ_MIN_PULSOS * histLlenas) return;    // muy poca senal
  float des = (ci - dn) / tot;                 // +1 = solo IZQ, -1 = solo DER
  float giro = 0;
  if (fabs(des) >= VOZ_ZONA)
    giro = (des > 0 ? 1 : -1) * vozPasoDeg * min(1.0f, (fabs(des) - VOZ_ZONA) / (1 - VOZ_ZONA) * 2);
  Serial.printf("@VOZ {\"i\":%lu,\"d\":%lu,\"des\":%.2f,\"giro\":%.1f}%s",
                (unsigned long)ci, (unsigned long)cd, des, giro, "\n");
  if (!modoCal && modoAuto && micsGiran && fabs(giro) >= 1.0f)
  {
    moverPasos(gradosAPasos(giro));
    ignorarHasta = millis() + 150;             // en voz basta una pausa corta: los contadores se limpian
  }
}

// =====================================================================
void setup() {
  Serial.begin(115200);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  setLed(false);
  pinMode(PIN_MIC_IZQ, INPUT);
  pinMode(PIN_MIC_DER, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_MIC_IZQ), isrIzq, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_MIC_DER), isrDer, RISING);
  motor.setSpeed(velRpm);
  apagarBobinas();
  EEPROM.begin(64);
  cargarCalib();
#if USAR_IR
  irrecv.enableIRIn();
#endif
  Serial.println("\nListo. Presiona el boton para activar la escucha.");
  enviarEstado();
}

void loop() {
  revisarSerie();
  revisarBoton();
  revisarIR();

  if (millis() - ultimoEstado > ESTADO_CADA_MS) enviarEstado();

  if (modoCal && millis() - ultimoCnt >= 250) {  // medidor de actividad de cada mic
    noInterrupts();
    uint32_t ci = cntIzq, cd = cntDer;
    cntIzq = 0; cntDer = 0;
    interrupts();
    Serial.printf("@CNT {\"i\":%lu,\"d\":%lu}\n", (unsigned long)ci, (unsigned long)cd);
    ultimoCnt = millis();
  }

  if (!escuchando || (long)(millis() - ignorarHasta) < 0) {   // resta: segura ante desborde
    limpiarTiempos();
    noInterrupts(); vozIzq = 0; vozDer = 0; interrupts();   // no contar ruido del motor
    vaciarHistVoz();                           // tras girar, medir de nuevo desde cero
    ultimaVoz = millis();
    return;
  }
  if (modoVoz || calBalHasta) {
    procesarVoz();
    limpiarTiempos();                          // en modo voz no se usan los tiempos
    return;
  }
  procesarSonido();
}
