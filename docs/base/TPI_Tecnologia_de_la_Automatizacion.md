<!-- VERSION BASE (original del grupo). La version ampliada esta en docs/modificado/. -->
<!-- El logo original estaba embebido en base64; se omitio en esta copia. -->

Cátedras:
Tecnología de la Automatización

Trabajo práctico integrador

Profesores:
Elvio Nicolas Oliva Saccenti
José Luis Catalano

Grupo X
Integrantes:

| Estudiantes | Nro. Legajo | Correo Electrónico |
| :---- | :---- | :---- |
| Doglio, Ramiro | 15071 | matiasdoglio@gmail.com |
| Falchi Massietti, Alexander Julián | 14949 | alexandermassietti@gmail.com |
| Fermani, Julián | 15172 | julianfermani45@gmail.com |
| Francioni, Ulises | 15887 | ulisesfrancioni@gmail.com |
| Fumero, Ignacio | 16083 | fumero.ignacio@gmail.com |
| Medina, Lisandro | 15669 | lichimedina05@gmail.com |
| Pineda, Martina | 16280 | martii.pineda@gmail.com |
| Pineda, Alvaro | 14303 | alvaropineda1017@gmail.com |

**Fecha de Entrega**: 20/11/2026

# Índice

**1. Resumen**
**2. Introducción**
**3. Objetivo**
5.1 Objetivo general
5.2 Objetivos específicos
**4. Marco Teórico**
**5. Diseño y Metodología**
**6. Esquema de Conexionado**
**7. Desarrollo del Código**
**8. Resultados**
**9. Conclusiones**

# 1. Resumen

El presente trabajo describe el desarrollo de un dispositivo de soporte motorizado para teléfonos celulares, cuya finalidad es orientar de forma automática la cámara del teléfono hacia la dirección de la que proviene una voz humana. El sistema está pensado principalmente para su uso en canales de streaming y programas en formato podcast, en los que resulta necesario que la cámara acompañe a un presentador que se desplaza dentro del espacio de grabación, sin requerir la intervención de un operador de cámara.

El proyecto tiene como objetivos principales construir un prototipo funcional basado en la plataforma Arduino, capaz de detectar la procedencia de la voz y reorientar el soporte en tiempo real, manteniendo una imagen estable y utilizando exclusivamente los componentes provistos por la cátedra.

# 2. Introducción

En los últimos años, los canales de streaming y los programas en formato podcast han experimentado un crecimiento sostenido de popularidad a nivel mundial. Este fenómeno ha impulsado la aparición de numerosos programas emergentes que, a diferencia de las grandes producciones, suelen contar con presupuestos acotados, equipos de trabajo reducidos y equipamiento técnico limitado.

En este contexto, se detectó que una de las necesidades más frecuentes de estos programas es disponer de un control adecuado sobre el movimiento de la cámara, capaz de reaccionar momento a momento a lo que ocurre en escena. En una producción profesional, esta tarea es realizada por operadores de cámara o mediante sistemas de seguimiento automatizados de alto costo; sin embargo, para un programa en sus inicios, estas alternativas resultan generalmente inaccesibles. Como consecuencia, es habitual que la cámara permanezca fija, de modo que el presentador queda fuera de cuadro al desplazarse, o que se vea obligado a restringir sus movimientos para permanecer dentro del encuadre.

La problemática a resolver consiste, por lo tanto, en lograr que la cámara, la de un teléfono celular, dispositivo ampliamente disponible y utilizado en producciones de bajo presupuesto, se oriente de manera automática hacia la persona que está hablando, siguiendo sus desplazamientos sin intervención humana. El proyecto propone abordar esta necesidad mediante el desarrollo de un soporte motorizado que identifica la dirección de procedencia de la voz y ajusta la orientación del teléfono en consecuencia, ofreciendo una solución de bajo costo y accesible para este tipo de producciones.

# 3. Objetivo

## 3.1 Objetivo general

Desarrollar un prototipo funcional, utilizando la plataforma Arduino, de un soporte para teléfono celular capaz de orientarse automáticamente hacia la fuente de una voz humana, con el fin de comprender el funcionamiento de las tecnologías involucradas en un sistema de control y su aplicación práctica a partir de los recursos limitados provistos por la cátedra.

## 3.2 Objetivos específicos

* Lograr que el soporte detecte la dirección de procedencia de una voz humana y siga su desplazamiento en tiempo real.
* Garantizar que la imagen captada por el teléfono se mantenga estable durante los movimientos del soporte, evitando sacudidas o cambios bruscos de orientación.
* Implementar el sistema completo utilizando únicamente los componentes electrónicos provistos por la cátedra.

# 4. Marco Teórico

# 5. Diseño y Metodología

## 5.1 Descripción general del sistema

El sistema se organiza en tres etapas funcionales: detección, procesamiento y actuación. La etapa de detección está formada por dos módulos de micrófono con salida digital, ubicados a ambos lados del soporte y separados entre sí una distancia conocida. La etapa de procesamiento está a cargo de una placa NodeMCU basada en el microcontrolador ESP8266, programada mediante el entorno de desarrollo de Arduino, que mide la diferencia de tiempo con la que el sonido llega a cada micrófono y, a partir de ella, calcula la dirección de la fuente. Finalmente, la etapa de actuación está compuesta por un motor paso a paso, comandado a través de su módulo controlador (driver), que hace girar el soporte sobre el que va montado el teléfono.

Como interfaz con el usuario, el sistema incorpora un pulsador que permite activar o desactivar el modo de escucha y un LED que indica cuándo dicho modo se encuentra activo.

| Componente | Función en el sistema |
| :---- | :---- |
| Placa NodeMCU ESP8266 | Unidad de control: registra los tiempos de llegada del sonido, calcula el ángulo de la fuente y comanda el motor. |
| Módulos de micrófono (×2) | Detectan el sonido y entregan un pulso digital cuando éste supera el umbral de disparo del módulo. |
| Motor paso a paso | Hace girar el soporte del teléfono. Resolución de 2048 pasos por vuelta (≈ 0,18° por paso). |
| Driver del motor | Amplifica las señales de control de la placa para energizar las bobinas del motor. |
| Pulsador | Activa o desactiva el modo de escucha. |
| LED indicador | Señala visualmente que el sistema se encuentra escuchando. |

## 5.2 Principio de localización de la fuente sonora

Para determinar la dirección del presentador se emplea el método de diferencia de tiempo de llegada (TDOA, por sus siglas en inglés, Time Difference of Arrival). Cuando una persona habla desde un costado del soporte, la onda sonora recorre una distancia algo menor hasta el micrófono más cercano, por lo que lo alcanza unos microsegundos antes que al otro. Midiendo esa diferencia de tiempo (Δt) y conociendo la separación entre micrófonos (d) y la velocidad del sonido en el aire (c), el ángulo θ de la fuente respecto del frente del soporte se obtiene como:

**θ = arcsen( c · Δt / d )**

Con una separación de d = 0,20 m y c = 343 m/s, la máxima diferencia de tiempo posible —correspondiente a un sonido que llega exactamente desde un lateral— es de aproximadamente 583 µs. Por convención, en el diseño un ángulo positivo indica que la fuente se encuentra a la izquierda del soporte y uno negativo, a la derecha.

## 5.3 Lógica de funcionamiento

El programa de control ejecuta de forma continua el siguiente ciclo:

1. **Activación.** El sistema permanece en reposo hasta que el usuario presiona el pulsador. La lectura del pulsador incluye un filtro antirrebote de 40 ms, y cada pulsación alterna entre los estados de escucha activa e inactiva, lo que se refleja en el LED.
2. **Captura de los tiempos de llegada.** Cada micrófono está asociado a una interrupción por flanco ascendente. Cuando un micrófono se dispara, el microcontrolador registra de inmediato el instante en microsegundos; sólo se conserva el primer disparo de cada micrófono, que corresponde al frente de onda directo.
3. **Ventana de espera.** Una vez detectado el primer micrófono, el sistema espera hasta 2 ms a que reaccione el segundo, tiempo holgadamente superior a la diferencia máxima esperable de 583 µs.
4. **Cálculo del ángulo.** Si ambos micrófonos reaccionaron, se calcula Δt y, con él, el ángulo θ mediante la expresión anterior. Si la diferencia es menor a 60 µs (equivalente a unos ±6°), se considera que la fuente ya está al frente y el soporte no se mueve. Si sólo reaccionó uno de los micrófonos, se asume que la fuente está claramente hacia ese lado y se adopta un giro fijo de 45°.
5. **Conversión a pasos.** El ángulo se convierte en la cantidad de pasos del motor según pasos = θ / 360° × 2048. Un parámetro de sentido permite invertir el giro si el montaje físico así lo requiere.
6. **Actuación.** El motor gira la cantidad de pasos calculada a una velocidad de 10 rpm y, al finalizar, se desenergizan sus bobinas.
7. **Pausa y reinicio.** El sistema espera 800 ms antes de volver a escuchar y descarta los tiempos registrados durante ese lapso, de modo que el propio ruido del motor no sea interpretado como una nueva fuente sonora.

## 5.4 Criterios de diseño

Las decisiones de diseño adoptadas responden directamente a los objetivos específicos planteados:

* **Uso de interrupciones.** Dado que las diferencias de tiempo a medir son del orden de cientos de microsegundos, una lectura periódica de los pines (polling) no ofrecería la precisión necesaria. Las interrupciones permiten registrar el instante de llegada en el momento exacto en que ocurre, lo que hace posible el seguimiento en tiempo real.
* **Zona muerta.** El umbral de 60 µs evita que el soporte realice pequeñas correcciones continuas ante variaciones mínimas de la posición del presentador, lo que contribuye directamente a la estabilidad de la imagen.
* **Velocidad de giro reducida.** Operar el motor a 10 rpm produce movimientos suaves y sin sacudidas; un giro de 45° demora aproximadamente 0,75 s. Se prioriza así la estabilidad de la imagen por sobre la rapidez de respuesta.
* **Desenergizado de bobinas.** Apagar las bobinas una vez completado el giro reduce el consumo y el calentamiento del motor; la caja reductora del propio motor mantiene la posición del soporte.
* **Pausa posterior al movimiento.** El intervalo de 800 ms evita que el ruido mecánico del motor vuelva a disparar los micrófonos y genere movimientos espurios.
* **Realimentación a través de los sensores.** Al estar los micrófonos montados sobre la parte giratoria del soporte, cada movimiento reorienta también a los sensores. De este modo, cada nueva medición refleja el error de orientación remanente, y el sistema se estabiliza cuando la fuente queda dentro de la zona muerta frontal.
* **Componentes provistos.** El diseño se resolvió íntegramente con los módulos disponibles en el kit de la cátedra, sin incorporar hardware adicional.

Entre las limitaciones propias del diseño se identifican dos. En primer lugar, los módulos de micrófono se disparan por nivel sonoro, por lo que el sistema reacciona ante cualquier sonido que supere el umbral y no exclusivamente ante la voz humana. En segundo lugar, un arreglo de sólo dos micrófonos no permite distinguir si la fuente se encuentra delante o detrás del soporte, por lo que el rango de detección útil es el semiplano frontal (de −90° a +90°).

*Tabla 2. Parámetros de configuración del sistema.*

| Parámetro | Valor | Función |
| :---- | :---- | :---- |
| Separación entre micrófonos | 0,20 m | Base de medición para el cálculo del ángulo. |
| Velocidad del sonido | 343 m/s | Constante física utilizada en el cálculo. |
| Zona muerta | 60 µs (≈ ±6°) | Diferencia por debajo de la cual se considera que la fuente está al frente. |
| Ventana de espera | 2000 µs | Tiempo máximo de espera al segundo micrófono. |
| Ángulo con un solo micrófono | 45° | Giro aplicado cuando sólo reacciona un micrófono. |
| Resolución del motor | 2048 pasos/vuelta | Precisión angular de ≈ 0,18° por paso. |
| Velocidad del motor | 10 rpm | Movimiento suave para preservar la estabilidad de la imagen. |
| Pausa posterior al giro | 800 ms | Evita detectar el ruido del propio motor. |
| Antirrebote del pulsador | 40 ms | Filtra los rebotes mecánicos del pulsador. |

## 5.5 Metodología de desarrollo

El desarrollo se llevó a cabo mediante un enfoque de prototipado incremental, avanzando en etapas sucesivas y validando cada una antes de pasar a la siguiente, en línea con los lineamientos de la cátedra:

1. **Definición de la idea y selección de componentes.** A partir de la problemática planteada se definió el principio de funcionamiento y se seleccionaron, entre los componentes provistos, los necesarios para las etapas de detección, procesamiento y actuación.
2. **Pruebas parciales por módulo.** Antes del montaje final se verificó cada módulo por separado: el disparo y la sensibilidad de los micrófonos, el giro y la cantidad de pasos por vuelta del motor, y la respuesta del pulsador y del LED.
3. **Integración y codificación.** Una vez validados los módulos se integraron en un único programa de control. Durante esta etapa se utilizó el monitor serie para visualizar en tiempo real las diferencias de tiempo medidas y los ángulos calculados.
4. **Calibración.** Con el prototipo armado se ajustaron experimentalmente los parámetros de la Tabla 2, como el sentido de giro, la zona muerta, la ventana de espera y la pausa posterior al movimiento.
5. **Ensayo del prototipo completo.** Finalmente se evaluó el comportamiento del sistema con una persona hablando desde distintas posiciones alrededor del soporte.

# 6. Esquema de Conexionado

# 7. Desarrollo del Código

En esta sección se presentan los fragmentos más relevantes del programa de control y se explica su funcionamiento. El programa fue desarrollado en el lenguaje del entorno Arduino (basado en C++) para la placa NodeMCU ESP8266.

## 7.1 Estructura general del programa

El código se organiza en bloques bien diferenciados, cada uno identificado con un comentario de encabezado:

* **Pines:** asignación de los pines de la placa a cada componente.
* **Ajustes de geometría:** constantes físicas y parámetros de calibración del sistema.
* **Motor:** configuración del motor paso a paso y de los tiempos de espera.
* **Estado:** variables que almacenan el estado del sistema durante la ejecución.
* **Funciones auxiliares:** rutinas de interrupción, control del LED, apagado de bobinas, reinicio de tiempos y lectura del pulsador.
* **setup() y loop():** inicialización del hardware y ciclo principal de control.

Todos los parámetros ajustables se declaran como constantes al comienzo del programa, con nombres descriptivos y comentarios que indican su unidad y propósito. De este modo, la calibración del sistema puede realizarse modificando un único lugar del código, sin intervenir en la lógica de control.

## 7.2 Definición de pines y parámetros

```cpp
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
```

Se incluye la biblioteca Stepper, que provee las funciones para controlar el motor paso a paso. Los tiempos se expresan en microsegundos, que es la unidad en que trabaja la medición de diferencias de llegada. El LED utilizado es el integrado en la placa, conectado al pin GPIO2, que se enciende con nivel bajo (LOW).

## 7.3 Configuración y control del motor

```cpp
const int PASOS_POR_VUELTA = 2048;
Stepper motor(PASOS_POR_VUELTA, D0, D7, D6, D8);   // orden IN1, IN3, IN2, IN4
void apagarBobinas() {
  digitalWrite(D0, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
}
```

Se crea el objeto motor indicando la cantidad de pasos por vuelta y los cuatro pines de control. Los pines se pasan en el orden IN1, IN3, IN2, IN4 porque la secuencia de energizado que genera la biblioteca Stepper no coincide con el orden físico de las bobinas del motor; con este reordenamiento el motor gira correctamente en ambos sentidos. La función apagarBobinas() pone en nivel bajo las cuatro salidas una vez terminado el movimiento, lo que evita el consumo de corriente y el calentamiento del motor mientras está detenido.

## 7.4 Captura de los tiempos de llegada mediante interrupciones

```cpp
volatile uint32_t tIzq = 0, tDer = 0;         // instante (us) en que reaccionó cada micrófono
void IRAM_ATTR isrIzq() { if (!tIzq) tIzq = micros(); }
void IRAM_ATTR isrDer() { if (!tDer) tDer = micros(); }
void limpiarTiempos() {
  noInterrupts();
  tIzq = 0;
  tDer = 0;
  interrupts();
}
```

Este es el núcleo de la detección. Cada micrófono tiene asociada una rutina de servicio de interrupción (ISR) que guarda el instante, en microsegundos, en que se produjo el disparo. La condición if (!tIzq) hace que sólo se registre el primer disparo de cada micrófono, ignorando los siguientes pulsos que genera la misma palabra o sus ecos.

* **volatile:** indica al compilador que estas variables pueden cambiar en cualquier momento desde una interrupción, por lo que siempre deben leerse desde memoria.
* **IRAM_ATTR:** en el ESP8266, las rutinas de interrupción deben ubicarse en la memoria RAM interna para poder ejecutarse en todo momento; de lo contrario, la placa puede reiniciarse.
* **noInterrupts() / interrupts():** deshabilitan momentáneamente las interrupciones para que el reinicio de ambas variables se realice como una operación indivisible, sin que un micrófono pueda dispararse a mitad del proceso.

## 7.5 Lectura del pulsador con antirrebote

```cpp
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
```

Al presionar un pulsador mecánico, sus contactos rebotan y generan varias transiciones en pocos milisegundos. La función registra el momento de cada cambio y sólo acepta un nuevo estado cuando éste se mantiene estable durante más de DEBOUNCE_MS (40 ms). Como el pulsador utiliza la resistencia de pull-up interna, su estado presionado corresponde a nivel bajo. Cada pulsación válida alterna la variable escuchando, actualiza el LED, descarta los tiempos que pudieran haberse registrado y envía un mensaje por el monitor serie. La función no utiliza demoras bloqueantes, por lo que no detiene la ejecución del resto del programa.

## 7.6 Inicialización

```cpp
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
```

La función setup() se ejecuta una única vez al encender la placa. Inicia la comunicación serie a 115200 baudios, configura el pulsador con pull-up interno, el LED como salida (inicialmente apagado) y los micrófonos como entradas. Luego asocia cada micrófono a su rutina de interrupción, que se activará en el flanco ascendente (RISING) de la señal, es decir, en el instante en que el sonido supera el umbral del módulo. Por último, fija la velocidad del motor en 10 rpm.

## 7.7 Ciclo principal de control

La función loop() se ejecuta de forma repetida e implementa la lógica. Se analiza a continuación en tres partes.

**Lectura del estado y ventana de espera**

```cpp
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
```

En cada iteración se revisa primero el pulsador. Si la escucha está desactivada, se descartan los tiempos registrados y se sale de la función. En caso contrario, se copian los tiempos de ambos micrófonos a variables locales (a y b) con las interrupciones deshabilitadas, para trabajar con valores coherentes entre sí. Si ningún micrófono se disparó, no hay nada que procesar. Si sólo se disparó uno, el programa sigue esperando al otro mientras no se haya cumplido la ventana de VENTANA_US (2 ms) desde el primer disparo.

**Cálculo del ángulo**

```cpp
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
```

Cuando ambos micrófonos reaccionaron, se calcula la diferencia de tiempo y se aplica la expresión θ = arcsen(c · Δt / d).

* **Cálculo de la diferencia con signo:** la resta se realiza entre valores sin signo y luego se convierte a un entero con signo. Así se obtiene un valor positivo cuando el sonido llegó primero al micrófono izquierdo y negativo en caso contrario. Además, el cálculo sigue siendo correcto aun cuando el contador de microsegundos se desborda y reinicia, lo que ocurre aproximadamente cada 71 minutos.
* **Zona muerta:** si la diferencia es menor a 60 µs, se informa que el sonido está al frente y no se mueve el motor.
* **Conversión de unidades:** el factor 1e-6 convierte microsegundos a segundos, para que el producto con la velocidad del sonido resulte en metros.
* **Acotamiento del argumento:** la función constrain() limita el valor a [−1, 1], ya que el ruido de medición podría producir una diferencia mayor a la físicamente posible y el arcoseno no está definido fuera de ese intervalo.
* **Conversión a grados:** asin() devuelve el ángulo en radianes, que se convierte a grados multiplicando por 180/π.

Si sólo reaccionó uno de los micrófonos, se asigna directamente un ángulo de ±45° hacia el lado correspondiente. En todos los casos se envía por el monitor serie el resultado obtenido, lo que resultó fundamental durante las pruebas y la calibración.

**Movimiento del motor**

```cpp
  int pasos = (int)(angulo / 360.0f * PASOS_POR_VUELTA) * SENTIDO;
  if (pasos != 0) {
    motor.step(pasos);
    apagarBobinas();
  }
  delay(ESPERA_POST_MS);                       // el motor tambien hace ruido
  limpiarTiempos();
}
```

El ángulo se convierte en una cantidad entera de pasos, proporcional a la fracción de vuelta que representa, y se multiplica por SENTIDO para adaptar el giro al montaje físico. La función motor.step() mueve el motor la cantidad de pasos indicada; el signo determina el sentido de giro. Esta llamada es bloqueante, es decir, el programa no continúa hasta que finaliza el movimiento. Luego se apagan las bobinas, se espera ESPERA_POST_MS (800 ms) para que el ruido del motor no sea detectado como una nueva fuente, y se reinician los tiempos para comenzar un nuevo ciclo de medición.

## 7.8 Mensajes del monitor serie

El programa informa por el puerto serie cada evento relevante, lo que permite seguir su funcionamiento en tiempo real desde la computadora. La Tabla 3 resume los mensajes posibles.

*Tabla 3. Mensajes emitidos por el monitor serie.*

| Mensaje | Significado |
| :---- | :---- |
| Listo. Presiona el boton para activar la escucha. | La placa terminó de inicializarse. |
| Escucha ACTIVADA / DESACTIVADA | Se presionó el pulsador y cambió el modo de escucha. |
| dt = … us -> angulo = … grados | Ambos micrófonos reaccionaron; se informan la diferencia de tiempo medida y el ángulo calculado. |
| Solo reacciono el micrófono … -> … grados | Sólo uno de los micrófonos reaccionó; se aplica el giro fijo de 45°. |
| Sonido al frente, no giro | La diferencia de tiempo quedó dentro de la zona muerta. |

# 8. Resultados

# 9. Conclusiones
