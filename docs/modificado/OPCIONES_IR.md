# Opciones de control infrarrojo (para decidir)

El profe sugirió sumar "control por infrarrojo". Hay tres caminos razonables. Ninguno está decidido; el firmware ya trae el **límite por software** activo y el **control remoto** listo para encender si lo eligen.

## Resumen rápido

| | A. Control remoto + receptor | B. Sensor fin de carrera | C. Sólo límite por software |
| :---- | :---- | :---- | :---- |
| Módulo | KY-022 (receptor VS1838B) + control remoto 38 kHz | KY-010 (horquilla) o KY-032 / TCRT5000 (obstáculo) | Ninguno |
| ¿Qué hace? | Una persona aprieta un botón y el soporte para, gira, centra, activa la escucha… | El soporte frena **solo** al llegar a un tope; además sirve para saber dónde está el "cero" | El firmware cuenta pasos y no deja pasar de ±120° |
| ¿Resuelve lo de los cables? | Parcialmente (alguien tiene que apretar "parar") | **Sí**, automáticamente | **Sí**, si el motor no pierde pasos |
| Pin | D3 | D3 (o RX si D3 está ocupado) | — |
| Código | **Ya hecho** (`USAR_IR 1`) | Hay que agregarlo (simple) | **Ya hecho**, activo |
| Biblioteca extra | IRremoteESP8266 | No | No |
| Dificultad | Baja | Media (hay que armar una pestaña física) | Nula |
| Suma al informe | "Interfaz de usuario a distancia" | "Sensor de posición / referencia: lazo cerrado de posición" | "Saturación del actuador" |

## A. Control remoto + receptor KY-022

**Qué es:** el receptor es un componente negro con una "burbuja", de 3 patas, sobre una plaquita. Muchos kits Arduino traen un control remoto chiquito de 17–21 botones.

**Cómo se conecta:** `S` → D3, `+` (medio) → 3V3, `−` → GND.
Algunas plaquitas vienen con los pines rotulados distinto: si no funciona, revisar la serigrafía.

**Cómo se activa en el código:**
1. En Arduino IDE: *Herramientas → Administrar bibliotecas* → buscar **IRremoteESP8266** → Instalar.
2. En `seguidor_app.ino` cambiar `#define USAR_IR 0` por `#define USAR_IR 1` y subir el programa.
3. Abrir el monitor serie (115200). Apretar cada botón del control: aparece `IR codigo = 0x...`.
4. Copiar esos números en las constantes `IR_STOP`, `IR_IZQ`, `IR_DER`, `IR_CENTRO`, `IR_ESCUCHA`, `IR_MODO` y volver a subir.

**Pro:** se puede parar o mover la cámara desde lejos, sin la compu.
**Contra:** no es automático; si nadie aprieta "parar", no para (por eso conviene combinarlo con C).

## B. Sensor IR como fin de carrera

**Qué es:**
- **KY-010 (horquilla / photo-interrupter):** una "U" con un emisor de un lado y un receptor del otro. Cuando algo pasa por el medio, corta el haz.
- **KY-032 o TCRT5000 (obstáculo):** emite IR y detecta el reflejo de algo cercano.

**Cómo se usaría:** se pega una pestañita (cartón, plástico) en la parte que gira. El sensor queda fijo en la base. Cuando la pestaña llega al sensor, la placa frena el motor. Con un sensor en el centro, además, al encender el soporte puede girar hasta encontrar la pestaña y así saber exactamente dónde está el 0 ("homing"). Esto **cierra el lazo de posición** aunque los micrófonos estén fijos.

**Conexión típica:** `S`/`OUT` → D3, `+`/`VCC` → 3V3, `−`/`GND` → GND.

**Pro:** automático, y arregla el problema de los pasos perdidos del modo "mics fijos".
**Contra:** hay que fabricar la pestaña y alinear el sensor; el código no está hecho todavía (son ~20 líneas).

## C. Sólo límite por software

Ya funciona: el firmware no permite girar más de ±120° (se cambia desde la app en *Configuración*). Basta con encender la placa con el soporte mirando al frente, o usar "Fijar centro aquí".

**Contra:** si el motor patina o pierde pasos, la cuenta se desfasa.

## Recomendación

- Si el kit trae control remoto: **A + C** (mínimo esfuerzo, se ve bien en la presentación).
- Si van con micrófonos fijos y quieren algo más sólido desde control: **B + C**.
- Si no hay módulos IR: **C** y mencionar A/B como mejoras futuras en las conclusiones.
