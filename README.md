# CW-CONTROL — Seguidor de sonido (TPI Tecnología de la Automatización)

Soporte motorizado para celular que gira hacia donde viene el sonido.
NodeMCU ESP8266 + 2 micrófonos KY-037 + motor 28BYJ-48 (driver ULN2003) + pulsador KY-004.

## Estructura

| Carpeta / archivo | Contenido |
| :---- | :---- |
| `docs/MANUAL_INSTALACION.md` | **Empezar acá.** Armado, cableado, instalación y uso paso a paso |
| `firmware/base/` | Programa original del grupo (sin cambios) |
| `firmware/modificado/` | Programa con app, modo manual, PARAR, límite de giro, mics fijos/girando e IR opcional |
| `app/index.html` | App de monitoreo y control (abrir en Chrome/Edge, conecta por USB) |
| `docs/base/` | Informe original |
| `docs/modificado/` | Informe completo (marco teórico, conexionado, app, plantilla de resultados) |
| `docs/modificado/OPCIONES_IR.md` | Comparación de alternativas de control infrarrojo (a decidir) |

## Uso rápido

1. Cablear según `docs/MANUAL_INSTALACION.md`.
2. Subir `firmware/modificado/seguidor_app/seguidor_app.ino` con Arduino IDE (placa *NodeMCU 1.0 (ESP-12E Module)*).
3. Cerrar el monitor serie, abrir `app/index.html` en Chrome y tocar **Conectar placa (USB)**.

Sin hardware: abrir la app y tocar **Modo simulación**.
