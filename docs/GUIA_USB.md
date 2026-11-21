# CW-CONTROL: cómo arrancar, cargar y diagnosticar por USB

Esta guía sirve para **macOS, Linux y Windows**. Para el cableado y la calibración detallada, consultá el [manual de armado e instalación](MANUAL_INSTALACION.md).

## 1. Usar la app si la NodeMCU ya está programada

1. Conectá la NodeMCU con un **cable micro-USB de datos**. Si el hub está dando problemas, probá primero una conexión directa.
2. Cerrá el **Monitor Serie** de Arduino IDE, otras terminales serie y cualquier otra pestaña conectada a la placa.
3. Abrí [`app/index.html`](../app/index.html) en **Chrome o Edge de escritorio**. No requiere instalar dependencias web ni iniciar un servidor. Si la política del navegador bloquea el acceso desde el archivo local, probá servir la carpeta desde `localhost` (por ejemplo, con Live Server de VS Code).
4. Tocá **Conectar placa (USB)** y seleccioná el **CP2102**. El selector de esta app está filtrado para el puente USB que usa nuestra placa (`10c4:ea60`); una placa con otro chip USB puede aparecer en el sistema operativo, pero no en ese selector.
5. Esperá **En vivo · USB**. Sin movimiento, el contador de lecturas debería subir aproximadamente **una vez por segundo**, aunque los valores no cambien. La placa arranca con la escucha apagada.

Sin placa, **Modo simulación** permite usar la app. Los datos se identifican como simulados. En el mapa, un clic simula un aplauso; mantener apretado simula una voz.

## 2. Identificar el puerto por sistema operativo

| Sistema | Dónde mirar | Si no aparece |
| :--- | :--- | :--- |
| **macOS** | En Terminal: `ls /dev/cu.*`. En nuestra Mac apareció `/dev/cu.usbserial-0001`. `ioreg -p IOUSB -w0` permite ver el CP2102 y el hub. | Probá otro cable de datos, otro puerto o sin el hub. Si macOS ve el CP2102 por USB pero no crea un puerto serie, comprobá si tu versión necesita el [driver CP210x de Silicon Labs](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers). En la Mac de prueba funcionó el driver integrado. |
| **Linux** | En terminal: `ls /dev/ttyUSB*` o `ls -l /dev/serial/by-id/`. La placa suele aparecer como `/dev/ttyUSB0`. | Si aparece pero no tenés permiso, mirá el grupo propietario con `ls -l /dev/ttyUSB0`. En Ubuntu/Debian suele ser `dialout`: `sudo usermod -aG dialout "$USER"` y luego cerrá sesión y volvé a entrar. En otras distribuciones el grupo puede ser diferente. No hace falta ejecutar el navegador como root. |
| **Windows** | **Administrador de dispositivos → Puertos (COM y LPT)**. Buscá **Silicon Labs CP210x USB to UART Bridge (COMx)**. | Si aparece como dispositivo desconocido o no hay un COM nuevo, probá otro cable y, si hace falta, instalá el [driver CP210x de Silicon Labs](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers). |

Compará la lista con la placa enchufada y desenchufada si no sabés cuál es. En macOS, no elijas puertos Bluetooth ni de depuración.

## 3. Cargar el firmware correcto

Esto reemplaza el programa que está en la placa y la reinicia. Usá [`firmware/modificado/seguidor_app/seguidor_app.ino`](../firmware/modificado/seguidor_app/seguidor_app.ino), **no** [`seguidor_base.ino`](../firmware/base/seguidor_base/seguidor_base.ino): la versión original no implementa el protocolo de la app. Cerrá Chrome en esa conexión y el Monitor Serie antes de subir.

### Arduino IDE 2 (macOS, Linux y Windows)

1. Instalá [Arduino IDE 2](https://www.arduino.cc/en/software/).
2. En **Preferencias → URLs adicionales del gestor de placas**, agregá `https://arduino.esp8266.com/stable/package_esp8266com_index.json`.
3. En el **Gestor de placas**, instalá **esp8266** de **ESP8266 Community**. En el **Gestor de bibliotecas**, instalá **Stepper**. `EEPROM` viene con el soporte de ESP8266; la biblioteca IR no hace falta con `USAR_IR` desactivado.
4. Elegí **NodeMCU 1.0 (ESP-12E Module)** y el puerto que identificaste. Abrí `seguidor_app.ino` y tocá **Subir**.
5. Si querés verificar la carga en el Monitor Serie, usá **115200 baudios**. Deberías ver líneas `@STATE` aun con la escucha apagada. Cerrá el monitor antes de conectar la app.

### Arduino CLI (alternativa)

Instalá [Arduino CLI](https://arduino.github.io/arduino-cli/latest/installation/) para tu sistema y ejecutá estos comandos desde la raíz del proyecto. Reemplazá **solo el puerto**: el ejemplo usa macOS; para Linux puede ser `/dev/ttyUSB0` y para Windows, `COM4`. Ejecutá cada línea por separado y verificá que termine bien antes de seguir.

```sh
arduino-cli core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266@3.1.2 --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli lib install Stepper@1.1.3
arduino-cli board list
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --build-path .cw-build firmware/modificado/seguidor_app
arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2 --port /dev/cu.usbserial-0001 --input-dir .cw-build firmware/modificado/seguidor_app
```

`.cw-build` contiene solo la salida de compilación; no hace falta versionarla. Si la carga queda en `Connecting...`, verificá que la app esté desconectada e intentá mantener apretado **FLASH** al comenzar la conexión; soltalo cuando empiece la escritura.

## 4. Verificar el equipo sin confundir conexión y telemetría

| Qué ves | Qué significa / qué hacer |
| :--- | :--- |
| **Puerto USB abierto**, pero **Chrome no recibió datos de la placa** | El navegador abrió el puerto, pero no recibió bytes. Con el puerto abierto, pulsá **RST** una vez. Si sigue en cero, probá otro cable o sin el hub. |
| **Puerto USB conectado · solo lectura**, con `IZQ: … DER: …` | La placa manda lecturas de un sketch de prueba. Son datos reales, pero no implementan el control de la app. Cargá `seguidor_app.ino`. |
| Símbolos raros al arrancar, luego `Listo…` y `@STATE` | Normal: el arranque del ESP8266 puede escribir a otra velocidad. No cambies los 115200 baudios del firmware. |
| **En vivo · USB** | Llegó un `@STATE` válido; ahora se habilitan los controles. `@STATE` indica el estado de la placa; `@POS` actualiza la posición durante un giro; `@SND` corresponde a un sonido detectado. |
| **Datos desactualizados** | Pasaron unos **4 segundos** sin recibir un estado o una posición. Los valores que quedan en pantalla no son lecturas nuevas. |
| **Conexión USB interrumpida** o desaparece el puerto del sistema | Si no desenchufaste nada, revisá el cable, el hub y la alimentación antes de seguir probando el motor. |

La **Consola serie** muestra lo que recibe la app. Una línea con `>` es un comando **enviado**, no una confirmación. `?` consulta el estado sin ordenar movimiento. La app desactiva DTR y RTS al abrir el puerto, porque esas señales también se usan para reinicio y programación de la NodeMCU. El botón **Desconectar USB** libera el puerto; **no apaga la placa ni desactiva por sí solo el modo automático**.

### Pruebas rápidas

- **Micrófonos sin movimiento:** elegí **Manual**, activá **Escucha** y aplaudí a cada lado por separado. Mirá **Último sonido**, **Disparó primero** y los mensajes `@SND`.
- **Motor sin intervención de los micrófonos:** dejá **Escucha: OFF**, despejá el recorrido y usá primero un botón de **5°**. Para un movimiento más visible, el comando `J25` desde la consola ordena **25° relativos hacia la izquierda**. No repitas una orden si el USB se corta sin saber en qué posición quedó.
- **Posición:** `@MOVE` muestra el destino y `@STATE` la posición calculada por pasos. **No hay sensor de posición**: si el soporte patina o el motor pierde pasos, la cifra de la app no lo detecta. Después de reiniciar, el firmware toma la posición inicial como `0°`; no busca el centro físico.
- **Cortes:** con el USB desenchufado, desconectá temporalmente el motor y probá la placa sola. Si el corte está ligado al motor, una alternativa es alimentar el **driver ULN2003** con una fuente regulada de **5 V** para el motor de 5 V, con **GND común** entre fuente/driver y NodeMCU. Primero desconectá el cable que alimentaba el driver desde `Vin`: **no unas el +5 V externo a `Vin` mientras la placa esté por USB**.
- **Luces:** los LED del ULN2003 cambian mientras el motor da pasos y pueden apagarse al terminar porque el firmware corta la corriente de las bobinas. Si desaparece también el hub o se caen otros accesorios USB, no es ese comportamiento normal de los LED del driver.

## 5. Qué hace el seguimiento de sonido

Los KY-037 envían un **disparo digital** al superar su umbral, no una grabación. Con **Escucha** y **Automático** activados, el firmware compara los tiempos de los dos disparos, aplica el ajuste de calibración y estima un ángulo. Si la diferencia es muy chica, interpreta que el sonido viene de frente a los micrófonos. Si dispara uno solo, espera hasta **2 ms** y, si el otro no reacciona, estima **45°** hacia ese lado.

Con los valores iniciales, una detección aislada ordena hasta **15° de giro**; los eventos seguidos del mismo lado forman una **racha** y permiten corregir más. Durante el giro y la pausa posterior (por defecto **500 ms**) no procesa sonidos nuevos para evitar seguir el ruido del motor.

**Con dos personas hablando al mismo tiempo no se elige una voz ni se compara volumen.** El sistema se guía por qué micrófono cruzó el umbral primero, tras corregir el desfase de calibración. Puede incluso emparejar disparos de voces distintas. Si hablan de lados opuestos, puede favorecer un lado, alternar o no girar. Ajustá la sensibilidad y el desfase con el panel **Calibración de micrófonos**; para los pasos completos, ver la [sección de pruebas del manual](MANUAL_INSTALACION.md#12-probar-por-partes).

## 6. Registro de la puesta en marcha

- En macOS se detectó el puente **CP2102** detrás del hub, en `/dev/cu.usbserial-0001`. Antes de cargar el firmware del proyecto, la placa enviaba `IZQ: 0   DER: 0` a 115200 baudios, pero no respondía con `@STATE`.
- Se compiló y cargó `seguidor_app.ino` con Arduino CLI, **ESP8266 Community 3.1.2** y **Stepper 1.1.3**. La herramienta verificó la escritura y después se recibieron varios `@STATE` por terminal. Más tarde Chrome también registró ocho `@STATE` seguidos.
- El HTML pasó a distinguir **puerto abierto**, **solo lectura**, **datos reales en vivo**, **datos desactualizados** y **simulación**. Se agregó un cierre explícito del puerto, diagnóstico de bytes recibidos y filtro para el CP2102. Se probó el manejo de la conexión y la telemetría con un puerto simulado, además de la prueba del equipo real.
- Primera prueba de movimiento: se ordenaron **5°**, la placa reportó **4,9°** y se observó un movimiento físico pequeño. Poco después se perdió el puerto incluso **sin Chrome**. El hub y el CP2102 volvieron a enumerarse. Eso es compatible con un corte o reinicio USB, pero **no prueba** que lo haya provocado el motor.
- Segunda prueba: se ordenaron **25°**, la placa reportó el movimiento completo y el puerto siguió presente durante la prueba. Se confirmó visualmente que el soporte se movió.

La carga y las pruebas físicas se hicieron en **macOS**. Los pasos de Linux y Windows son instrucciones de instalación y uso; no se verificaron con este equipo en esos sistemas.
