# Manual de armado e instalación (para gente que nunca hizo esto)

Este manual cubre todo el camino, desde las piezas sueltas hasta ver el mapa en la compu. Seguí los pasos **en orden** y no te saltees los "✅ Control".

> **Regla de oro:** conectá y desconectá cables **siempre con el USB desenchufado**.

---

## 0. Lo que necesitás

| Cosa | Cómo se ve |
| :---- | :---- |
| NodeMCU ESP8266 | Plaquita azul o negra con un chip metálico plateado y un conector micro-USB |
| 2 × KY-037 (micrófonos) | Plaquita con un cilindro plateado (el micrófono) y un cubito azul con un tornillito (potenciómetro). 4 pines: `AO`, `G`, `+`, `D0` |
| KY-004 (pulsador) | Plaquita con un botón. 3 pines: `S`, medio, `−` |
| Motor 28BYJ-48 | Motor redondo plateado/amarillo con 5 cables de colores que terminan en un conector blanco |
| Driver ULN2003 | Plaquita verde con un chip negro largo, 4 LEDs, un zócalo blanco y pines `IN1`–`IN4`, `+`, `−` |
| Protoboard | Tablero blanco lleno de agujeritos |
| Cables "dupont" | Cablecitos con puntas. **Macho-macho** (punta en los dos lados) y **hembra-macho** (agujerito de un lado) |
| Cable micro-USB **de datos** | ⚠️ Algunos cables sólo cargan y no pasan datos. Si la compu no detecta la placa, probá con otro cable |
| Destornillador chiquito | Para los potenciómetros de los micrófonos |
| (Opcional) KY-022 + control remoto | Ver `docs/modificado/OPCIONES_IR.md` |

---

## 1. Entender la protoboard (2 minutos)

```
  +  −   a b c d e   f g h i j   +  −
  │  │   ─────────   ─────────   │  │
  │  │   ● ● ● ● ●   ● ● ● ● ●   │  │   ← fila 1: a-e están unidos entre sí; f-j entre sí
  │  │   ● ● ● ● ●   ● ● ● ● ●   │  │   ← fila 2: otro grupo independiente
  │  │      ...         ...      │  │
  rieles                         rieles
```

- Las **columnas largas de los costados** (marcadas `+` rojo y `−` azul) están unidas **de arriba a abajo**. Las vamos a usar para repartir la alimentación: el riel `+` va a ser **3V3** y el riel `−` va a ser **GND**.
- En el centro, cada **fila de 5 agujeritos** está unida entre sí (a-b-c-d-e), pero **no** cruza la canaleta del medio.
- Ojo: en algunas protoboards largas los rieles están cortados a la mitad. Si algo no anda, poné un cable puente entre las dos mitades.

---

## 2. Conocer los pines de la NodeMCU

Mirá la placa con el USB hacia abajo. Los nombres están impresos al lado de cada pin (a veces del lado de abajo de la placa).

```
            ┌─────────────┐
       A0 ──┤             ├── D0   ← motor IN1
      RSV ──┤             ├── D1   ← micrófono IZQUIERDO
      RSV ──┤   ESP8266   ├── D2   ← micrófono DERECHO
      SD3 ──┤             ├── D3   ← (opcional) receptor IR
      SD2 ──┤             ├── D4   (LED de la placa, no usar)
      SD1 ──┤             ├── 3V3  ← alimentación de los módulos
      CMD ──┤             ├── GND  ← masa
      SD0 ──┤             ├── D5   ← pulsador
      CLK ──┤             ├── D6   ← motor IN2
      GND ──┤             ├── D7   ← motor IN3
      3V3 ──┤             ├── D8   ← motor IN4
       EN ──┤             ├── RX
      RST ──┤             ├── TX
      GND ──┤             ├── GND
      Vin ──┤             ├── 3V3
            └──── USB ────┘
     Vin = 5 V del USB → alimentación del motor
```

> La distribución puede variar un poco según el fabricante: **guiate siempre por lo impreso en la placa**, no por el dibujo.

**Ubicación:** pinchá la NodeMCU en la protoboard a caballo de la canaleta central (una fila de pines de cada lado). Si no entra (algunas son anchas), dejala afuera y conectá con cables hembra-macho.

---

## 3. Alimentación (2 cables)

| # | Desde | Hasta |
| :---- | :---- | :---- |
| 1 | Pin **3V3** de la NodeMCU | Riel **+** (rojo) de la protoboard |
| 2 | Pin **GND** de la NodeMCU | Riel **−** (azul) de la protoboard |

> ⚠️ **Nunca** conectes los micrófonos ni el pulsador a **Vin** (5 V): la placa trabaja con 3,3 V y la podés dañar.

---

## 4. Micrófono IZQUIERDO (KY-037 n.º 1)

Marcalo con cinta o un fibrón: "IZQ". *Izquierdo* significa a la izquierda mirando **desde atrás del soporte**, o sea, como si fueras la cámara.

| # | Pin del micrófono | Hasta |
| :---- | :---- | :---- |
| 3 | `+` | Riel **+** (3V3) |
| 4 | `G` | Riel **−** (GND) |
| 5 | `D0` | Pin **D1** de la NodeMCU |
| — | `AO` | **Nada** (queda libre) |

> No confundas el `D0` del micrófono (su salida) con el `D0` de la NodeMCU. Son cosas distintas.

## 5. Micrófono DERECHO (KY-037 n.º 2)

| # | Pin del micrófono | Hasta |
| :---- | :---- | :---- |
| 6 | `+` | Riel **+** (3V3) |
| 7 | `G` | Riel **−** (GND) |
| 8 | `D0` | Pin **D2** de la NodeMCU |
| — | `AO` | **Nada** |

**Montaje:** los dos micrófonos tienen que quedar **separados 20 cm** (de centro a centro de los cilindros plateados), mirando hacia el mismo lado (el frente). Si los ponen a otra distancia, cambien `SEPARACION_M` en el código.

## 6. Pulsador (KY-004)

| # | Pin del pulsador | Hasta |
| :---- | :---- | :---- |
| 9 | `S` | Pin **D5** de la NodeMCU |
| 10 | pin del **medio** (+) | Riel **+** (3V3) |
| 11 | `−` | Riel **−** (GND) |

## 7. Driver ULN2003 y motor

| # | Desde | Hasta |
| :---- | :---- | :---- |
| 12 | Pin **D0** de la NodeMCU | `IN1` del driver |
| 13 | Pin **D6** de la NodeMCU | `IN2` del driver |
| 14 | Pin **D7** de la NodeMCU | `IN3` del driver |
| 15 | Pin **D8** de la NodeMCU | `IN4` del driver |
| 16 | Pin **Vin** de la NodeMCU | `+` del driver (a veces dice `5-12V`) |
| 17 | Pin **GND** de la NodeMCU (o riel −) | `−` del driver |
| 18 | Conector blanco del motor | Zócalo blanco del driver. Entra en una sola posición; no lo fuerces |

> ⚠️ El motor va a **Vin** (5 V), **no** a 3V3. Es lo único que va a 5 V.
> Si el driver tiene un jumper (puentecito) al lado de `+`/`−`, dejalo puesto.

## 8. (Opcional) Receptor IR KY-022

Sólo si deciden usarlo (ver `docs/modificado/OPCIONES_IR.md`): `S` → **D3**, medio → riel **+**, `−` → riel **−**.

---

## 9. ✅ Control antes de enchufar

Revisá uno por uno:

- [ ] Ningún cable va de **Vin** a un micrófono o al pulsador.
- [ ] El riel `+` está conectado a **3V3** y el riel `−` a **GND**.
- [ ] Mic IZQ → D1, mic DER → D2, pulsador → D5.
- [ ] D0 → IN1, D6 → IN2, D7 → IN3, D8 → IN4 (ojo: **no** es D0-D1-D2-D3).
- [ ] No hay dos cables en el mismo agujerito por error ni puntas peladas tocándose.

---

## 10. Instalar el programa en la computadora (se hace una sola vez)

Si la placa ya está programada, para usar la app no necesitás Arduino IDE. La [guía USB](GUIA_USB.md) incluye pasos por sistema operativo, carga con Arduino CLI y diagnóstico de cortes.

### 10.1 Arduino IDE
1. Descargar **Arduino IDE 2** desde la página oficial de Arduino (arduino.cc → Software) e instalarlo.

### 10.2 Soporte para la placa ESP8266
1. Abrir Arduino IDE → **Archivo → Preferencias**.
2. En **"URLs adicionales del gestor de placas"** pegar:
   `https://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. Aceptar. Ir a **Herramientas → Placa → Gestor de placas**, buscar **esp8266** e instalar el de *ESP8266 Community*.
4. En el gestor de bibliotecas, buscá e instalá **Stepper** si todavía no está instalada.

### 10.3 Driver USB (sólo si la compu no reconoce la placa)
- **Windows:** la placa usa el chip **CP2102**. Si al enchufarla no aparece ningún puerto COM nuevo, instalar el driver *"CP210x USB to UART Bridge"* de Silicon Labs.
- **Linux:** en una terminal ejecutar `sudo usermod -a -G dialout $USER`, después cerrar la sesión y volver a entrar.
- **Mac:** normalmente no hace falta nada; si no aparece el puerto, instalar el mismo driver CP210x.

### 10.4 Elegir placa y puerto
1. Enchufar la NodeMCU por USB.
2. **Herramientas → Placa → ESP8266 Boards → "NodeMCU 1.0 (ESP-12E Module)"**.
3. **Herramientas → Puerto** → elegir el que apareció al enchufar (en Windows `COMx`; en Linux `/dev/ttyUSB0`). Si no sabés cuál es: desenchufá, mirá la lista, enchufá y fijate cuál apareció.

---

## 11. Subir el programa a la placa

Hay dos versiones:

| Archivo | Para qué |
| :---- | :---- |
| `firmware/base/seguidor_base/seguidor_base.ino` | La versión original. Funciona sólo con el monitor serie. |
| `firmware/modificado/seguidor_app/seguidor_app.ino` | **La recomendada.** Funciona con la app, tiene modo manual, PARAR y límite de giro. |

1. **Archivo → Abrir** → elegir el `.ino`.
2. Botón **→ (Subir)**. Tarda alrededor de 1 minuto. Abajo tiene que decir *"Done uploading"* / *"Carga completada"*.
3. Si falla con "Failed to connect": volvé a intentar y, mientras dice *"Connecting…"*, mantené apretado el botón **FLASH** de la placa.

> Importante: el monitor serie del IDE y la app **no pueden usar el puerto a la vez**. Cerrá uno antes de abrir el otro.

---

## 12. Probar por partes

Abrí **Herramientas → Monitor serie** y poné la velocidad en **115200** (abajo a la derecha).

### 12.1 Placa
Apretá el botón **RST** de la placa. Tiene que aparecer: `Listo. Presiona el boton para activar la escucha.`
(Antes puede salir texto raro: es normal.)

### 12.2 Pulsador y LED
Apretá el pulsador → `Escucha ACTIVADA` y se prende el **LED azul** de la placa. Apretá otra vez → se apaga.

### 12.3 Calibrar los micrófonos (lo más importante)
Cada KY-037 tiene **dos LEDs**: uno de encendido y otro de señal (L2), que se prende cuando "escucha" algo.

1. Con la sala en silencio, girá el tornillito del cubo azul con el destornillador **de a poco** hasta encontrar el punto en que el LED de señal **justo se apaga**.
2. Desde ahí, girá un poquito más hacia el lado en que queda apagado.
3. Aplaudí a un metro: el LED tiene que **parpadear**.
4. Hacé lo mismo con el otro micrófono. Tienen que quedar **parecidos**: si uno es mucho más sensible, el ángulo sale torcido.

> 💡 **Más fácil con la app:** con el firmware modificado, abrí la app → **Calibración de micrófonos → Iniciar calibración**. Vas a ver una barra por micrófono que muestra cuánto "escucha" cada uno: girá el tornillo hasta que en silencio queden en 0 y que al aplaudir suban las dos. Después seguí los pasos 2 a 4 del panel (10 aplausos al frente, 10 de cada costado) y tocá **Guardar en la placa**. Mientras calibrás, el motor no se mueve.

Con la escucha activada, aplaudí de frente al soporte. Lo ideal es que diga `Sonido al frente` o un `dt` chico. Si dice siempre `Solo reacciono el micrófono ...`, ese micrófono está poco sensible (o mal conectado).

### 12.4 Sentido del motor
Aplaudí del lado **izquierdo**. El soporte tiene que girar **hacia la izquierda**. Si gira para el otro lado, en el código cambiá `const int SENTIDO = 1;` por `-1` y volvé a subir.

Si el motor **vibra pero no gira**, o gira para un solo lado: revisá el orden D0→IN1, D6→IN2, D7→IN3, D8→IN4.

---

## 13. Usar la app

1. Subí la versión **modificada** del firmware y **cerrá el monitor serie** del Arduino IDE.
2. Abrí `app/index.html` con **Google Chrome** o **Microsoft Edge** en la compu (doble clic o arrastrarlo al navegador). Firefox y Safari no sirven porque no tienen Web Serial.
3. Clic en **"Conectar placa (USB)"** → elegir el puerto (CP2102 / COMx / ttyUSB0) → **Conectar**.
4. Esperá **Placa conectada** y que el contador de **En vivo · USB** empiece a subir. Clic en **Escucha: OFF** para activarla (o apretá el pulsador físico). Para probar los micrófonos sin movimiento automático, elegí **Manual** antes de activar la escucha.
5. Aplaudí: en el mapa aparece la **línea naranja** hacia el sonido, el micrófono que reaccionó primero se pinta de naranja y la **flecha azul** (la cámara) gira.

| Botón | Qué hace |
| :---- | :---- |
| Automático / Manual | En **Manual** el soporte no sigue el sonido (pero lo sigue mostrando); se mueve sólo con los botones |
| ◀ 15° · ◀ 5° · 5° ▶ · 15° ▶ | Gira de a poco |
| Ir a + deslizador | Va a un ángulo exacto |
| Centro | Vuelve a 0° |
| **PARAR** | Frena el motor en el acto |
| Configuración → Micrófonos | "Giran con el soporte" si van arriba de la parte que gira; "Fijos" si quedaron quietos en la base |
| Configuración → Límite | Máximo giro hacia cada lado (protege los cables) |
| Configuración → Fijar centro aquí | La posición actual pasa a ser 0° (usalo al empezar, con el soporte mirando al frente) |
| Configuración → Ganancia | Qué parte del error corrige cada sonido. 0,5 = la mitad. Más alto = más rápido pero más brusco |
| Configuración → Giro máx. por sonido | Lo máximo que gira por un solo sonido (15° por defecto). Así un aplauso suelto no hace girar todo de una |
| Configuración → Velocidad motor | 5 a 15 rpm (12 por defecto). Más de 15 y el motor se saltea pasos |
| Configuración → Pausa tras girar | Tiempo que ignora sonidos después de moverse, para no "escucharse" a sí mismo. Si gira solo después de cada movimiento, subila |
| Dato "Racha" | Cuántos sonidos seguidos llegaron del mismo lado. Con racha ×2, ×3, ×4 cada sonido corrige más: por eso hablar sostenido lo mueve rápido y un aplauso suelto, poco |

**Sin la placa:** tocá **"Modo simulación"** (la escucha arranca activada).
- **Clic** en el mapa = un **aplauso** desde ese punto: el soporte se mueve *un poco* (como mucho el "giro máx.").
- **Mantener apretado** = una persona **hablando** ahí: aparecen ondas violetas y el soporte la va siguiendo de a poco. Si arrastrás el cursor sin soltar, la "persona" camina y el soporte la acompaña.

> Si el navegador no deja conectar abriendo el archivo con doble clic, en VS Code instalá la extensión **Live Server**, hacé clic derecho en `index.html` → *Open with Live Server*.

---

## 14. Problemas típicos

| Problema | Solución |
| :---- | :---- |
| La compu no detecta la placa | Otro cable USB (de datos); instalar el driver CP210x (10.3) |
| "Failed to connect" al subir | Mantener apretado **FLASH** durante "Connecting…" |
| La placa no arranca / se reinicia sola | Algo conectado a D3, D4 o D8 la está trabando al encender: desconectar, encender y volver a conectar. Revisar que el motor esté en Vin y no en 3V3 |
| Texto raro en el monitor serie | Poner la velocidad en 115200 |
| El motor gira sin parar ante cualquier ruido | Micrófonos demasiado sensibles: bajarlos (12.3) |
| Nunca detecta nada | Micrófonos poco sensibles o mal conectados (¿`D0` del mic a D1/D2?). ¿Está la escucha activada (LED azul prendido)? |
| Gira hacia el lado contrario | `SENTIDO = -1` |
| Vibra y no gira | Orden de IN1–IN4 |
| La app dice "Este navegador no soporta Web Serial" | Usar Chrome o Edge en una computadora |
| La app no conecta | Cerrar el monitor serie del Arduino IDE (sólo un programa puede usar el puerto) |
| Gira demasiado brusco / muy lento | Bajar o subir la **Ganancia** y el **Giro máx. por sonido** en Configuración |
| En simulación el clic no hace nada | ¿Tocaste "Modo simulación"? ¿La escucha está en ON? |
| Gira y después vuelve a girar solo | El ruido del motor dispara los micrófonos: bajar sensibilidad o subir `ESPERA_POST_MS` (por ejemplo, a 1200) |
