<!-- VERSION MODIFICADA: completa las secciones 4, 6, 8 y agrega app, modo manual, limite de giro e IR. La original esta en docs/base/. -->

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
3.1 Objetivo general
3.2 Objetivos específicos
**4. Marco Teórico**
4.1 Sistemas de control: lazo abierto y lazo cerrado
4.2 Diagrama de bloques del sistema
4.3 Tipo de controlador
4.4 Localización acústica por TDOA
4.5 Sensor: módulo de micrófono KY-037
4.6 Actuador: motor paso a paso 28BYJ-48 y driver ULN2003
4.7 Microcontrolador ESP8266 e interrupciones
4.8 Comunicación serie y control infrarrojo
**5. Diseño y Metodología**
5.6 Modos de montaje de los micrófonos
5.7 Aplicación de monitoreo y control
5.8 Control manual, límite de giro y control infrarrojo
5.9 Calibración de los micrófonos desde la aplicación
**6. Esquema de Conexionado**
**7. Desarrollo del Código**
7.9 Versión modificada del programa
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

## 4.1 Sistemas de control: lazo abierto y lazo cerrado

Un sistema de control es un conjunto de elementos que actúan sobre un proceso (la *planta*) para que una variable de salida siga un valor deseado, llamado *referencia* o *consigna*. Según exista o no realimentación de la salida, se distinguen dos configuraciones:

* **Lazo abierto.** La acción de control se calcula sin medir el resultado obtenido. Es simple y económico, pero no corrige perturbaciones ni errores del modelo: si el actuador no responde como se esperaba, el controlador no se entera.
* **Lazo cerrado (realimentado).** Un sensor mide la salida y la compara con la referencia. La diferencia, llamada *error*, es la que procesa el controlador. El sistema corrige por sí mismo los desvíos, a costa de mayor complejidad y de la posibilidad de oscilaciones si está mal ajustado.

En el presente proyecto la variable controlada es la **orientación del soporte** respecto de la fuente sonora y la referencia es que la fuente quede **al frente** (θ = 0°). Cuando los micrófonos van montados sobre la parte giratoria, el ángulo que miden es directamente el error de orientación, por lo que el sistema funciona en **lazo cerrado**. Si por razones mecánicas (por ejemplo, los cables) los micrófonos deben quedar fijos, el sistema pasa a funcionar en **lazo abierto** respecto de la orientación: la placa estima la posición del soporte contando los pasos enviados al motor, sin medirla (ver 5.6).

## 4.2 Diagrama de bloques del sistema

```
 Referencia        error         pasos              par
 (0° = al  ──►(Σ)───────►[Controlador]────►[Actuador]──────►[Planta]──────┬──► θ soporte
  frente)      ▲ −        ESP8266          ULN2003 +        soporte +      │
               │                           28BYJ-48         teléfono       │
               │                                                           │
               └──────────────[Sensor: 2× KY-037 + TDOA]◄──────────────────┘
                              (los mics giran con el soporte)

 Perturbaciones: movimiento del presentador, ruido ambiente, ecos, ruido del motor.
```

*Figura 1. Diagrama de bloques en lazo cerrado (micrófonos sobre la parte giratoria).*

| Elemento | Implementación en el proyecto |
| :---- | :---- |
| Referencia | Fuente sonora al frente del soporte (θ = 0°). |
| Sensor / transductor | Dos módulos KY-037 y la medición de Δt por interrupciones. |
| Comparador y controlador | Programa en la NodeMCU ESP8266. |
| Actuador | Driver ULN2003 y motor paso a paso 28BYJ-48. |
| Planta | Soporte giratorio con el teléfono. |
| Variable controlada | Orientación del soporte (θ_soporte). |
| Perturbaciones | Movimiento del presentador, ruido, ecos, ruido mecánico del motor. |

## 4.3 Tipo de controlador

El controlador implementado es **discreto** (actúa sólo cuando ocurre un evento sonoro) y puede describirse como un **controlador proporcional con zona muerta y saturación**:

* **Proporcional:** la cantidad de pasos enviada al motor es proporcional al error medido, pasos = θ/360° × 2048. En la versión base la ganancia es unitaria: el soporte intenta corregir todo el error en un único movimiento. En la versión modificada se adopta una **ganancia K = 0,5**, es decir que cada sonido corrige sólo la mitad del error, y una **saturación por evento** de 15°. Así, un sonido aislado (por ejemplo, un aplauso) produce un movimiento acotado, mientras que una voz sostenida, que genera muchos disparos seguidos, acerca el soporte a la fuente en forma progresiva. Con K = 0,5, el error se reduce a la mitad en cada evento: 40° → 20° → 10° → 5° (dentro de la zona muerta). El sistema converge sin sobrepasar la fuente, lo que favorece la estabilidad de la imagen.
* **Zona muerta:** si |Δt| < 60 µs (≈ ±6°) el controlador no actúa. Esto evita correcciones pequeñas y continuas (oscilaciones o "temblor" de la imagen), a cambio de admitir un error residual de hasta ±6°.
* **Saturación:** el argumento del arcoseno se limita a [−1, 1] y, en la versión modificada, el giro por evento se limita a ±15° y la posición del soporte a ±120° para proteger los cables.

La ley de control de la versión modificada puede escribirse como:

**u[k] = sat_{15°·n}( K_n · e[k] )  si |e[k]| ≥ 6°;   u[k] = 0  en caso contrario**

**K_n = mín( 1 ; 0,5 + 0,25 · (n − 1) )**

donde e[k] es el error medido en el evento sonoro k, u[k] el giro aplicado y n la **racha**: la cantidad de eventos consecutivos detectados del mismo lado con menos de 2 s entre sí (n ≤ 4). Un sonido aislado tiene n = 1 y produce un giro acotado (K = 0,5, como máximo 15°). Una voz sostenida genera eventos sucesivos que aumentan n y, con ello, la ganancia y el tope de giro, de modo que el soporte responde con más decisión cuando hay evidencia repetida de que la fuente está hacia ese lado. Se trata de una forma sencilla de **control con ganancia variable (gain scheduling)** en función de la confianza en la medición. En la simulación, para un error inicial de 60° con voz sostenida, el tiempo medio hasta apuntar a la fuente se redujo de 6,6 s (ganancia fija) a 3,1 s.

Como el motor paso a paso posiciona por cantidad de pasos y su caja reductora mantiene la posición sin alimentación, la planta no presenta inercia apreciable a la velocidad utilizada; la dinámica del lazo queda dominada por el tiempo de giro y la pausa posterior al movimiento. En la versión modificada la velocidad se elevó a 12 rpm (≈ 72°/s) y la pausa se redujo a 500 ms; ambos valores son ajustables (5 a 15 rpm, ya que por encima de ≈ 15 rpm el 28BYJ-48 tiende a perder pasos).

## 4.4 Localización acústica por TDOA

El sonido se propaga en el aire a una velocidad c ≈ 343 m/s (a 20 °C). Si una fuente lejana se encuentra a un ángulo θ del frente de un par de micrófonos separados una distancia d, el frente de onda recorre una distancia adicional d·sen θ hasta el micrófono más alejado. La diferencia de tiempo de llegada (TDOA) es, por lo tanto:

**Δt = d · sen θ / c   ⇒   θ = arcsen( c · Δt / d )**

Esta aproximación de *campo lejano* supone que la fuente está a una distancia mucho mayor que d, de modo que el frente de onda puede considerarse plano. Con d = 0,20 m el Δt máximo es de 583 µs. La resolución angular depende de la precisión con que se mide Δt: un error de 10 µs equivale a ≈ 1° cerca del frente y a varios grados cerca de los laterales, donde la función arcoseno es más empinada.

Un arreglo de dos micrófonos presenta una **ambigüedad frente/atrás**: una fuente a θ y otra a 180° − θ producen el mismo Δt. Por eso el rango útil es el semiplano frontal.

## 4.5 Sensor: módulo de micrófono KY-037

El KY-037 integra un micrófono electret, un amplificador y un **comparador** (LM393) cuyo umbral se ajusta con el potenciómetro azul del módulo. Dispone de dos salidas: AO, analógica, proporcional a la señal del micrófono, y DO, digital, que cambia de estado cuando la señal supera el umbral. En el proyecto se utiliza la salida digital, ya que el ESP8266 posee una única entrada analógica y se necesitan dos canales medidos con precisión de microsegundos.

Al tratarse de un detector por umbral, el módulo responde mejor a **sonidos impulsivos** (aplausos, chasquidos, golpes, palabras pronunciadas con fuerza) que a la voz continua de baja intensidad, cuyo flanco de subida es menos definido. Además, ambos módulos deben calibrarse con umbrales similares; una diferencia de sensibilidad entre ellos se traduce en un error sistemático en Δt.

## 4.6 Actuador: motor paso a paso 28BYJ-48 y driver ULN2003

Un motor paso a paso convierte pulsos eléctricos en desplazamientos angulares discretos. El 28BYJ-48 es un motor unipolar de 5 V con cuatro bobinas y una caja reductora interna de ≈ 1:64, lo que da 2048 pasos por vuelta en modo de paso completo (≈ 0,18° por paso). Su principal ventaja para este proyecto es que permite **posicionar en lazo abierto**: si no pierde pasos, la posición del eje se conoce contando los pasos enviados.

El microcontrolador no puede entregar la corriente que requieren las bobinas (≈ 200 mA), por lo que se utiliza el **ULN2003**, un arreglo de siete transistores Darlington con diodos de protección, que actúa como interfaz de potencia: cada entrada IN1–IN4 controla una bobina.

## 4.7 Microcontrolador ESP8266 e interrupciones

El ESP8266 es un microcontrolador de 32 bits a 80 MHz con WiFi integrado y lógica de 3,3 V. La placa NodeMCU agrega el regulador, el conversor USB-serie (CP2102) y la numeración de pines D0–D8. Una **interrupción** es un mecanismo por el cual un evento de hardware (en este caso, un flanco ascendente en el pin de un micrófono) detiene momentáneamente el programa principal para ejecutar una rutina breve (ISR). Esto permite registrar el instante del evento con una precisión del orden de pocos microsegundos, imposible de lograr leyendo los pines periódicamente.

## 4.8 Comunicación serie y control infrarrojo

La **comunicación serie** (UART) transmite datos bit a bit por un par de líneas. A través del cable USB, la placa y la computadora intercambian texto a 115200 baudios. Los navegadores Chrome y Edge permiten que una página web acceda a ese puerto mediante la interfaz **Web Serial**, lo que hace posible una aplicación de monitoreo sin instalar programas adicionales.

El **control infrarrojo** utiliza luz invisible de ≈ 940 nm modulada a 38 kHz. Un control remoto codifica cada botón como una secuencia de pulsos (por ejemplo, protocolo NEC) y un receptor como el VS1838B (módulo KY-022) demodula la señal y entrega los pulsos en una salida digital. Por otro lado, los sensores IR de obstáculo o de horquilla emiten y detectan su propia luz infrarroja y pueden usarse como finales de carrera (ver 5.8).

# 5. Diseño y Metodología

## 5.1 Descripción general del sistema

El sistema se organiza en tres etapas funcionales: detección, procesamiento y actuación. La etapa de detección está formada por dos módulos de micrófono con salida digital, ubicados a ambos lados del soporte y separados entre sí una distancia conocida. La etapa de procesamiento está a cargo de una placa NodeMCU basada en el microcontrolador ESP8266, programada mediante el entorno de desarrollo de Arduino, que mide la diferencia de tiempo con la que el sonido llega a cada micrófono y, a partir de ella, calcula la dirección de la fuente. Finalmente, la etapa de actuación está compuesta por un motor paso a paso, comandado a través de su módulo controlador (driver), que hace girar el soporte sobre el que va montado el teléfono.

Como interfaz con el usuario, el sistema incorpora un pulsador que permite activar o desactivar el modo de escucha y un LED que indica cuándo dicho modo se encuentra activo. En la versión modificada se suma una aplicación web, conectada por USB, que muestra en tiempo real la orientación del soporte, la posición de los micrófonos y la dirección del último sonido, y que permite comandar el soporte de forma manual (ver 5.7).

*Tabla 1. Componentes del sistema y su función.*

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

Entre las limitaciones propias del diseño se identifican dos. En primer lugar, los módulos de micrófono se disparan por nivel sonoro, por lo que el sistema reacciona ante cualquier sonido que supere el umbral y no exclusivamente ante la voz humana. En segundo lugar, un arreglo de sólo dos micrófonos no permite distinguir si la fuente se encuentra delante o detrás del soporte, por lo que el rango de detección útil es el semiplano frontal (de −90° a +90°). Cabe aclarar, respecto de la primera limitación, que al tratarse de detectores por umbral el sistema responde con mayor confiabilidad a sonidos impulsivos (aplausos, chasquidos, palabras enfáticas) que a la voz hablada a volumen normal; por ello, durante los ensayos se utilizaron ambos tipos de estímulo (ver sección 8).

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

## 5.6 Modos de montaje de los micrófonos

Durante el armado se detectó que, si los micrófonos giran junto con el soporte, sus cables pueden tensarse o enrollarse. Por ese motivo, el programa modificado admite dos modos de funcionamiento, seleccionables desde la aplicación o por comando serie:

| | Micrófonos que giran (modo R) | Micrófonos fijos (modo F) |
| :---- | :---- | :---- |
| Tipo de lazo | Cerrado: el sensor mide el error remanente. | Abierto: el sensor mide la dirección absoluta y la posición del soporte se estima contando pasos. |
| Ángulo medido | Relativo al frente actual del soporte. | Absoluto, respecto del frente fijo de los micrófonos. |
| Corrección aplicada | pasos = θ_medido | pasos = θ_medido − θ_soporte |
| Ventaja | Corrige errores del motor (pasos perdidos). | Los cables de los micrófonos no se mueven. |
| Desventaja | Cables en movimiento. | Si el motor pierde pasos, el error se acumula; requiere una posición inicial conocida ("centro"). |

En ambos modos la placa mantiene un contador de pasos (posPasos), que se toma como 0 al encender o al presionar "Fijar centro aquí" en la aplicación. Ese contador permite mostrar la orientación en el mapa y aplicar el límite de giro.

## 5.7 Aplicación de monitoreo y control

Se desarrolló una aplicación web (archivo app/index.html) que se ejecuta en el navegador Google Chrome o Microsoft Edge y se comunica con la placa por el cable USB mediante la interfaz Web Serial. No requiere instalación ni conexión a Internet. Se descartó el uso del WiFi del ESP8266 porque su funcionamiento genera interrupciones internas que pueden demorar la atención de las interrupciones de los micrófonos y degradar la medición de Δt.

La aplicación ofrece:

* **Mapa en vista superior:** muestra el soporte, los dos micrófonos (izquierdo "I" y derecho "D", a escala de su separación de 0,20 m), la orientación de la cámara, la zona fuera del límite de giro y la dirección de los últimos sonidos detectados, con una estela que se desvanece. El micrófono que se disparó primero se resalta.
* **Datos en vivo:** orientación actual, dirección del último sonido, Δt medido, micrófono que reaccionó primero, estado de la escucha y modo de funcionamiento.
* **Control:** activar o desactivar la escucha; elegir modo automático o manual; girar ±5° o ±15°; ir a un ángulo elegido con un deslizador; volver al centro; y un botón **PARAR** que detiene el motor en medio de un giro.
* **Configuración:** modo de montaje de los micrófonos, límite de giro y redefinición del centro.
* **Modo simulación:** permite probar la aplicación sin la placa. Un clic sobre el mapa simula un aplauso desde ese punto; mantener presionado simula una persona hablando (disparos repetidos, con más ruido de medición y algunos disparos de un solo micrófono), que puede desplazarse arrastrando el cursor.

Para la comunicación se definió un protocolo de texto simple. La placa envía, además de los mensajes originales, líneas que comienzan con "@" seguidas de datos en formato JSON (Tabla 4), y recibe comandos de una letra (Tabla 5).

*Tabla 4. Mensajes de la placa hacia la aplicación.*

| Mensaje | Contenido |
| :---- | :---- |
| @STATE | Estado completo: escucha, modo, montaje, posición (pasos y grados), límite. Se envía cada 1 s y ante cada cambio. |
| @SND | Sonido detectado: Δt, ángulo relativo, ángulo absoluto, micrófono que reaccionó. |
| @MOVE / @POS | Inicio de un giro y posición durante el giro. |
| @LIMIT | El movimiento se recortó por el límite de giro. |
| @ERR | Comando rechazado (por ejemplo, durante un giro). |

*Tabla 5. Comandos de la aplicación hacia la placa.*

| Comando | Acción |
| :---- | :---- |
| L1 / L0 / L | Escucha activada / desactivada / alternar. |
| A / M | Modo automático / manual. |
| `G<grados>` | Ir a un ángulo absoluto (ej.: G-30). |
| `J<grados>` | Girar en forma relativa (ej.: J5). |
| C | Volver al centro. |
| X | Parar el motor inmediatamente. |
| Z | Tomar la posición actual como centro. |
| R / F | Micrófonos que giran / fijos. |
| `T<grados>` | Límite de giro (10° a 180°). |
| `K<valor>` | Ganancia: fracción del error corregida por sonido (0,1 a 1). |
| `P<grados>` | Giro máximo por sonido aislado (2° a 90°). |
| `V<rpm>` | Velocidad del motor (5 a 15 rpm). |
| `W<ms>` | Pausa posterior a cada giro (100 a 2000 ms). |
| Q1 / Q0 | Modo calibración (informa actividad de cada micrófono, no mueve el motor). |
| `O<µs>` / `D<m>` / `Y<µs>` | Offset de Δt / separación efectiva / zona muerta. |
| S / B | Guardar la calibración en EEPROM / restablecer valores de fábrica. |
| ? | Solicitar el estado. |

## 5.8 Control manual, límite de giro y control infrarrojo

El **modo manual** desactiva el seguimiento automático: los sonidos se siguen detectando y se muestran en el mapa, pero el soporte sólo se mueve por orden del usuario. Esto permite, por ejemplo, dejar la cámara fija sobre un invitado.

El **límite de giro por software** impide que el soporte supere ±120° (valor configurable) respecto del centro, recortando cualquier movimiento que lo excedería. Protege los cables sin agregar hardware.

Por sugerencia de la cátedra se evaluó incorporar **control por infrarrojo**. Se analizaron tres alternativas, que se detallan en el documento OPCIONES_IR.md del repositorio:

| Alternativa | Qué agrega | Pin | Observación |
| :---- | :---- | :---- | :---- |
| Receptor KY-022 + control remoto | Ordenes a distancia: parar, girar, centrar, escucha, modo. | D3 | Ya implementada en el programa (se habilita con USAR_IR = 1). |
| Sensor IR de horquilla (KY-010) u obstáculo (KY-032) como fin de carrera | Detención automática en un tope físico y posición de referencia ("home"). | D3 o RX | Requiere una pestaña en la parte giratoria. |
| Sólo límite por software | Protección de cables por conteo de pasos. | — | Ya implementada; no requiere componentes. |

En todos los casos el giro se ejecuta paso a paso y, cada 16 pasos (≈ 45 ms), el programa revisa si llegó una orden de parada, por lo que la detención es prácticamente inmediata.

## 5.9 Calibración de los micrófonos desde la aplicación

La precisión del sistema depende de que ambos micrófonos se disparen en forma pareja y de conocer con exactitud su separación. Para ello se incorporó a la aplicación un asistente de calibración de cuatro pasos. Durante la calibración la placa entra en un modo especial (comando Q1) en el que informa las mediciones pero **no mueve el motor**.

1. **Sensibilidad.** El umbral de cada KY-037 es un potenciómetro físico, por lo que no puede modificarse por software. Lo que sí hace la aplicación es medirlo: las rutinas de interrupción cuentan los flancos de cada micrófono y la placa envía cada 250 ms la actividad de ambos (mensaje @CNT). Con esa información, los integrantes ajustan los potenciómetros hasta que en silencio la actividad sea nula y, ante aplausos, ambos micrófonos reaccionen. La aplicación informa además el porcentaje de eventos detectados por los dos micrófonos (objetivo: ≥ 90 %).
2. **Offset.** Si un micrófono es más sensible, alcanza su umbral antes y todas las mediciones de Δt quedan desplazadas en un valor constante. Con 10 aplausos frente al soporte (Δt teórico = 0), el promedio de las mediciones es ese desplazamiento, que luego se resta: Δt_corregido = Δt_medido − offset.
3. **Escala.** Con 10 aplausos a 90° de cada lado (sen θ = ±1), la separación efectiva es d_ef = c · (Δt_izq − Δt_der) / 2. Al usar la diferencia entre ambos lados, el offset se cancela. Este valor absorbe errores de montaje y la variación de la velocidad del sonido con la temperatura. La aplicación verifica el signo, lo que permite detectar micrófonos cruzados.
4. **Zona muerta.** Se recomienda igual a 3 veces el desvío estándar de las mediciones del paso 2 (mínimo 20 µs), de modo que el ruido de medición no provoque movimientos cuando la fuente está al frente.

Para que la estimación sea robusta frente a ecos y disparos espurios, en cada serie se descartan las muestras que se apartan de la mediana más de 3 desviaciones absolutas medianas (MAD) escaladas, y con las restantes se calculan media y desvío. Los valores obtenidos se envían a la placa (comandos O, D e Y) y se almacenan en su memoria no volátil (EEPROM, comando S), por lo que se conservan al apagarla. En una prueba con el simulador, configurado con un desfase de 45 µs y una separación real de 0,214 m, la calibración estimó 44,4 µs y 0,2145 m, descartando automáticamente un eco.

# 6. Esquema de Conexionado

Todos los módulos de señal (micrófonos, pulsador y receptor IR) se alimentan desde el pin **3V3** de la NodeMCU, para que sus salidas no superen los 3,3 V que admiten las entradas del ESP8266. El driver del motor se alimenta desde el pin **Vin**, que entrega los 5 V del cable USB. Todas las masas (GND) son comunes. Las tablas se leen como *origen → destino*.

```
                         ┌──────────────── NodeMCU ESP8266 ────────────────┐
  KY-037 izq.  DO ──────►│ D1 (GPIO5)                          D0 (GPIO16) │──► IN1 ┐
  KY-037 der.  DO ──────►│ D2 (GPIO4)                          D6 (GPIO12) │──► IN2 │ ULN2003 ══► 28BYJ-48
  KY-004        S ──────►│ D5 (GPIO14)                         D7 (GPIO13) │──► IN3 │ (zócalo blanco)
  [opc] KY-022  S ──────►│ D3 (GPIO0)                          D8 (GPIO15) │──► IN4 ┘
                         │ LED integrado = D4 (GPIO2)                      │
  + de módulos ◄─────────│ 3V3                                  Vin (5 V)  │──► + del ULN2003
  − / G de todos ◄───────│ GND                                  GND        │──► − del ULN2003
                         └───────────────────────── USB ───────────────────┘
                                                     │
                                                computadora (app)
```

*Figura 2. Esquema de conexionado.*

*Tabla 6. Alimentación.*

| Origen | Destino |
| :---- | :---- |
| NodeMCU 3V3 | Riel + de la protoboard |
| NodeMCU GND | Riel − de la protoboard |
| NodeMCU Vin (5 V) | ULN2003 + |
| NodeMCU GND (o riel −) | ULN2003 − |

*Tabla 7. Micrófonos KY-037.*

| Pin del módulo | Micrófono izquierdo | Micrófono derecho |
| :---- | :---- | :---- |
| + | Riel 3V3 | Riel 3V3 |
| G | Riel GND | Riel GND |
| D0 (salida digital) | NodeMCU D1 (GPIO5) | NodeMCU D2 (GPIO4) |
| AO | Sin conectar | Sin conectar |

*Tabla 8. Pulsador KY-004.*

| Pin del módulo | Destino |
| :---- | :---- |
| S | NodeMCU D5 (GPIO14) |
| + (pin del medio) | Riel 3V3 |
| − | Riel GND |

*Tabla 9. Driver ULN2003 y motor.*

| Origen | Destino |
| :---- | :---- |
| NodeMCU D0 (GPIO16) | IN1 |
| NodeMCU D6 (GPIO12) | IN2 |
| NodeMCU D7 (GPIO13) | IN3 |
| NodeMCU D8 (GPIO15) | IN4 |
| Conector del 28BYJ-48 | Zócalo blanco de 5 pines del ULN2003 (entra en una sola posición) |

*Tabla 10. Receptor infrarrojo KY-022 (opcional).*

| Pin del módulo | Destino |
| :---- | :---- |
| S | NodeMCU D3 (GPIO0) |
| + (pin del medio) | Riel 3V3 |
| − | Riel GND |

Consideraciones sobre la elección de pines:

* **D0 (GPIO16) no admite interrupciones**, por eso se lo destina al motor y no a un micrófono.
* **D3 (GPIO0) y D4 (GPIO2)** deben estar en nivel alto al encender la placa, y **D8 (GPIO15)** en nivel bajo; de lo contrario la placa no arranca. El LED integrado (D4) y el receptor IR (D3, cuya salida en reposo es alta) cumplen esa condición, al igual que la entrada IN4 del ULN2003 en D8.
* El LED indicador es el integrado en la placa (D4), que se enciende con nivel bajo.
* Con el pulsador se usa la resistencia de pull-up interna (INPUT_PULLUP); al presionarlo la entrada pasa a nivel bajo.

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

## 7.9 Versión modificada del programa

La versión modificada (firmware/modificado/seguidor_app/seguidor_app.ino) conserva la lógica de detección descripta en 7.2 a 7.7 e incorpora los siguientes cambios:

| Cambio | Motivo |
| :---- | :---- |
| Contador de posición (posPasos) | Conocer hacia dónde apunta el soporte, mostrarlo en la app y aplicar el límite de giro. |
| Movimiento de a un paso (función moverPasos) revisando órdenes cada 16 pasos | Poder **parar** el motor en medio de un giro desde la app o el control IR. Se llama a yield() en cada paso para evitar el reinicio por watchdog del ESP8266. |
| Límite de giro (limiteDeg, ±120° por defecto) | Proteger los cables. |
| Variable micsGiran | Funcionar en lazo cerrado (micrófonos sobre el soporte) o en lazo abierto (micrófonos fijos). |
| Modo manual (modoAuto = false) | Mover el soporte sólo por orden del usuario. |
| Ganancia (ganancia = 0,5), giro máximo por sonido (maxGiroDeg = 15°) y racha | Que un aplauso aislado mueva sólo un poco y una voz sostenida acerque el soporte rápido, aumentando ganancia y tope con cada sonido consecutivo del mismo lado (ver 4.3). Ajustables desde la app. |
| Velocidad 12 rpm y pausa de 500 ms, ajustables (velRpm, esperaPostMs) | Respuesta más rápida que la versión base (10 rpm, 800 ms) sin perder pasos. |
| Pausa post-movimiento no bloqueante (ignorarHasta) | Reemplaza a delay(800): la placa sigue atendiendo la app durante la pausa. |
| Lectura de comandos por el puerto serie (revisarSerie, ejecutarComando) | Recibir las órdenes de la app (Tabla 5). |
| Mensajes "@" en formato JSON | Que la app pueda interpretar los datos (Tabla 4) sin perder los mensajes legibles originales. |
| Control IR opcional (USAR_IR) | Alternativa evaluada en 5.8; desactivada por defecto para no exigir bibliotecas adicionales. |

El fragmento central del cálculo de la corrección en ambos modos de montaje es:

```cpp
float posDeg = pasosAGrados(posPasos);
float angAbs = micsGiran ? posDeg + angRel : angRel;   // dirección del sonido en el "mundo"
float correccion = angAbs - posDeg;                    // cuánto falta girar
// racha: sonidos seguidos del mismo lado (voz sostenida)
int lado = fabs(correccion) < ZONA_MUERTA_DEG ? 0 : (correccion > 0 ? 1 : -1);
if (lado != 0 && lado == ladoRacha && millis() - ultimoSonidoMs < VENTANA_RACHA_MS)
  racha = min(racha + 1, 4);
else
  racha = lado != 0 ? 1 : 0;
ladoRacha = lado;

if (modoAuto && lado != 0) {
  float k    = min(1.0f, ganancia + 0.25f * (racha - 1));
  float tope = maxGiroDeg * racha;
  moverPasos(gradosAPasos(constrain(correccion * k, -tope, tope)));
}
```

Con los micrófonos girando, angAbs − posDeg es igual a angRel; con ganancia 1 y sin límite por evento, el comportamiento sería idéntico al de la versión base. Con los micrófonos fijos, la corrección descuenta la posición actual del soporte.

# 8. Resultados

> **Para completar con los ensayos.** Se propone el siguiente procedimiento y tablas. Borrar este recuadro al terminar.

**Procedimiento de ensayo.** Marcar en el piso, a 1,5 m del soporte, posiciones a −60°, −30°, 0°, +30° y +60° (con un transportador o una hoja impresa). Desde cada posición, producir 5 aplausos y luego decir 5 veces una palabra en voz alta. Registrar para cada estímulo el Δt y el ángulo que muestra la app, si el soporte giró hacia el lado correcto y el ángulo final de la cámara. Repetir con los micrófonos fijos si se usa ese montaje.

*Tabla 11. Precisión de la localización (completar).*

| Posición real | Estímulo | Δt promedio (µs) | Ángulo medido promedio | Error | Aciertos de lado (de 5) |
| :---- | :---- | :---- | :---- | :---- | :---- |
| −60° | Aplauso | | | | |
| −30° | Aplauso | | | | |
| 0° | Aplauso | | | | |
| +30° | Aplauso | | | | |
| +60° | Aplauso | | | | |
| −30° | Voz | | | | |
| +30° | Voz | | | | |

*Tabla 12. Comportamiento del lazo (completar).*

| Ensayo | Resultado |
| :---- | :---- |
| Movimientos hasta quedar apuntando a la fuente (lazo cerrado) | |
| Error final de orientación | |
| Tiempo de respuesta (sonido → fin del giro) | |
| Disparos falsos causados por el ruido del motor | |
| Funcionamiento del botón PARAR | |
| Funcionamiento del límite de giro | |

Se sugiere incluir capturas de la aplicación durante los ensayos y una foto del prototipo armado.

# 9. Conclusiones

> **Borrador para ajustar según los resultados obtenidos.**

Se desarrolló un prototipo de soporte motorizado capaz de orientar un teléfono celular hacia la dirección de procedencia de un sonido, empleando exclusivamente los componentes provistos por la cátedra. El sistema integra las etapas clásicas de un lazo de control: un sensor (dos micrófonos y la medición de la diferencia de tiempo de llegada), un controlador (el ESP8266, que implementa una ley proporcional con zona muerta y saturación), un actuador (driver ULN2003 y motor paso a paso) y una planta (el soporte giratorio).

El montaje de los micrófonos sobre la parte giratoria permitió cerrar el lazo de manera natural, ya que cada medición representa el error de orientación remanente. Se comprobó además que, cuando el montaje obliga a dejar los micrófonos fijos, el sistema puede operar en lazo abierto gracias a la capacidad del motor paso a paso de posicionar contando pasos, con la contrapartida de acumular error si se pierden pasos.

La incorporación de una aplicación de monitoreo resultó de gran utilidad tanto para la calibración como para la demostración del funcionamiento, al hacer visibles en tiempo real las variables internas del sistema. El modo manual, el botón de parada y el límite de giro aportan seguridad y flexibilidad de uso.

Entre las limitaciones observadas se destacan la sensibilidad de los módulos KY-037 a cualquier sonido que supere el umbral, la menor confiabilidad frente a la voz hablada en comparación con sonidos impulsivos, y la ambigüedad frente/atrás propia de un arreglo de dos micrófonos. Como mejoras futuras se proponen: utilizar la salida analógica de los micrófonos con un conversor de mayor velocidad para aplicar técnicas de correlación, sumar un tercer micrófono para resolver la ambigüedad, incorporar un sensor de referencia (fin de carrera) para recuperar la posición absoluta, y un ajuste experimental de la ganancia y del giro máximo por evento según el tipo de producción.
