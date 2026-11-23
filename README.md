# CW-CONTROL — Seguidor de sonido (TPI Tecnología de la Automatización)

Soporte motorizado para celular que gira hacia donde viene el sonido.
NodeMCU ESP8266 + 2 micrófonos KY-037 + motor 28BYJ-48 (driver ULN2003) + pulsador KY-004.

## Estructura

| Carpeta / archivo | Contenido |
| :---- | :---- |
| `docs/MANUAL_INSTALACION.md` | **Empezar acá.** Armado, cableado, instalación y uso paso a paso |
| `docs/GUIA_USB.md` | Cómo arrancar en macOS, Linux y Windows; carga por CLI, diagnóstico USB y pruebas realizadas |
| `firmware/base/` | Programa original del grupo (sin cambios) |
| `firmware/modificado/` | Programa con app, modo manual, PARAR, límite de giro, mics fijos/girando e IR opcional |
| `app/index.html` | App de monitoreo y control (abrir en Chrome/Edge, conecta por USB) |
| `docs/base/` | Informe original |
| `docs/modificado/` | Informe completo (marco teórico, conexionado, app, plantilla de resultados) |
| `docs/modificado/OPCIONES_IR.md` | Comparación de alternativas de control infrarrojo (a decidir) |
| `docs/modificado/RESULTADOS_MICROFONOS.md` | Diagnóstico del desbalance de micrófonos, mediciones, modo voz y pendientes |

## Uso rápido

1. Cableá según el [manual de armado](docs/MANUAL_INSTALACION.md).
2. Si la placa aún no tiene el firmware de la app, cargá [`seguidor_app.ino`](firmware/modificado/seguidor_app/seguidor_app.ino) usando la placa **NodeMCU 1.0 (ESP-12E Module)**. Seguí la [guía USB para macOS, Linux y Windows](docs/GUIA_USB.md).
3. Cerrá el Monitor Serie, abrí [`app/index.html`](app/index.html) en Chrome o Edge de escritorio, tocá **Conectar placa (USB)** y elegí el **CP2102**. **En vivo · USB** y un contador que sube confirman que están llegando datos reales.

Sin hardware: abrir la app y tocar **Modo simulación**.
