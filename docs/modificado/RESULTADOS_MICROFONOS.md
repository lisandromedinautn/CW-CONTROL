# Micrófonos: diagnóstico, resultados y modo voz

Registro de las pruebas del 23/11/2026 sobre el desbalance entre los micrófonos izquierdo y derecho, qué se cambió en el firmware y qué falta para terminar.

## El problema

- Para que el micrófono **derecho** escuchara había que subirle mucho la sensibilidad. Cuando lo hacía, las lecturas se iban a los extremos (±90° o ±45°).
- Con el **izquierdo** pasaba lo contrario: tomaba sonidos más bajos, pero daba lecturas más chicas.

## Cómo funciona la medición (y por qué falla con voz)

Los módulos (KY-037/038) tienen salida digital (DO). Mandan un pulso cuando el sonido supera el umbral del potenciómetro, no la amplitud. El firmware original toma el **primer pulso** de cada micro y calcula el ángulo con la diferencia de tiempo:

```
ángulo = asin( c · dt / separación )
```

Con la separación real (13,7 cm, el alto de un Galaxy J2), el `dt` máximo físicamente posible es de **unos 400 µs**.

## Mediciones (monitor serie, modo calibración `Q1`, hablando desde el centro)

| Prueba | Pulsos IZQ / 250 ms | Pulsos DER / 250 ms | Con los dos | Solo un micro | Observación |
|---|---|---|---|---|---|
| 1. Potenciómetros originales, 60 s | 100–200 | **0** | 0 | 23 solo IZQ | El derecho no dio ni un pulso en todo el minuto |
| 2. DER ajustado, "OOOO" sostenido, 30 s | 50–200 (≈50 en silencio) | 500–4800 | 31 | 72 solo IZQ | Los 31 `dt` dieron **imposibles** (±800 a ±1500 µs) y con signo al azar |
| 3. Con detección de ataque, "O" cortas y de cerca, 30 s | 0–190 | 0–3500 | 2 | 25 DER, 15 IZQ | Los 2 emparejados dieron bien (−94 y −132 µs, cerca del centro) |

En la prueba 3 hubo que hablar más cerca porque, con los potenciómetros bajos para que el izquierdo no se disparara en silencio, desde la distancia de antes no detectaba ninguno de los dos.

## Conclusiones

1. **Sensibilidades muy distintas.** Con la misma voz, el derecho da de 5 a 10 veces más pulsos que el izquierdo. Cuando está al borde del umbral, el comparador "tiembla" y larga ráfagas de miles de pulsos.
2. **Con voz sostenida, el primer pulso no sirve.** Una vocal es una onda periódica y el comparador dispara en cada ciclo. Cada micro engancha un ciclo distinto, entonces el `dt` sale al azar.
3. **Aun tomando solo el ataque, la voz no da.** El volumen de la voz sube a lo largo de 10 a 50 ms. El micro más sensible pasa su umbral varios milisegundos antes que el otro. Ese error es mucho más grande que la señal (400 µs). Por eso casi nunca caen los dos dentro de la ventana de 2 ms.
4. **Por tiempos funciona con aplausos** (ataque instantáneo), **no con voz**, mientras se usen módulos de salida digital.
5. **Con voz conviene comparar actividad, no tiempos.** Como los micros giran con el soporte (lazo cerrado), no hace falta el ángulo exacto: alcanza con saber qué lado escucha más y girar de a poco hasta que queden parejos. La diferencia de sensibilidad se compensa con un factor de balance calibrado desde el centro.

## Cambios en el firmware (`firmware/modificado/seguidor_app`)

| Cambio | Para qué |
|---|---|
| `SEPARACION_M = 0.137` | La separación real entre micros (antes 0,20) |
| Descarte de `dt` imposibles (`\|dt\|` > separación/c × 1,25) | Ya no satura a ±90° por un micro que se disparó con ruido |
| `U1` / `U0`: usar o ignorar sonidos de un solo micro (por defecto se ignoran) | Evita las lecturas fijas de ±45°. En modo calibración se siguen informando |
| Detección de ataque: solo cuenta el primer pulso después de 15 ms de silencio de ese micro | Los dos micros miden el mismo instante (sirve para aplausos) |
| **Modo voz por actividad** (`E1`) | Ver abajo |
| Balance guardado en EEPROM (`S`), compatible con la calibración guardada antes | Persistir el factor de balance |

### Modo voz

Cada 200 ms:

```
dn  = pulsosDER / balance           // derecho normalizado
des = (pulsosIZQ - dn) / (pulsosIZQ + dn)    // +1 = solo IZQ, -1 = solo DER
si (pulsosIZQ + dn) < 20       -> silencio, no hace nada
si |des| < 0,30                -> al frente, no gira
si no                          -> gira hasta 6° hacia el lado con más actividad
```

**Suavizado:** decide con la suma de las últimas 3 ventanas (`VOZ_N`, unos 0,6 s). En silencio y después de cada giro, esa suma vuelve a cero.

Informa cada ventana con `@VOZ {"i":..,"d":..,"des":..,"giro":..}`. Solo mueve el motor en modo automático, con micros que giran (`R`) y fuera de calibración (`Q0`).

### Comandos nuevos

| Comando | Qué hace |
|---|---|
| `E1` / `E0` | Prende o apaga el modo voz |
| `I` | Calibra el balance: hablar unos 3 s **desde el centro y a la distancia de uso**. Después `S` para guardar |
| `N<grados>` | Giro máximo por decisión en modo voz (2 a 30, por defecto 10) |
| `H<factor>` | Pone el balance a mano (pulsos DER / pulsos IZQ al centro). Ej: `H6` |
| `U1` / `U0` | Usa o ignora sonidos que oyó un solo micro |

`@STATE` ahora también incluye `"unico"`, `"voz"` y `"bal"`.

## Prueba del modo voz (con motor)

Primero se subió el potenciómetro del **izquierdo al máximo**. Después se calibró hablando desde el centro: dio **balance = 6,21** (IZQ 731, DER 4541 pulsos en 3 s), y quedó guardado en la EEPROM.

Prueba de 45 s con el motor activo y suavizado de 3 ventanas:

| Tramo | Resultado |
|---|---|
| Hablando desde el centro (0–14 s) | Quieto casi todo el tiempo (un solo giro de 4,7°) |
| Hablando desde la izquierda (15–25 s) | Giró a la izquierda de a 6°, hasta 52,6° |
| Hablando desde la derecha (26–45 s) | Volvió de forma continua a la derecha, hasta 3,2° (la prueba terminó mientras seguía girando) |

Sigue la voz hacia los dos lados, en la dirección correcta y sin oscilar. Con estos parámetros va a unos 6°/s.

## Estado

- El firmware con modo voz y suavizado está **cargado en la placa y probado**.
- En la EEPROM quedaron guardados la separación (0,137 m) y el balance (6,21).
- El modo voz **no** se guarda: después de reiniciar hay que mandar `E1` (o agregarlo a la app).

## App

En la tarjeta **Control** hay un bloque **Seguir voz**:
- **Voz / Aplausos:** cambia entre el modo voz (`E1`, y además activa micros que giran) y la medición por tiempos (`E0`).
- **Calibrar balance:** manda `I`. Hay que hablar 3 s desde el frente. Si sale bien, guarda solo (`S`).
- **Barra IZQ–DER:** muestra el desbalance de cada lectura `@VOZ`. La zona punteada del medio es "al frente" y la marca se pone naranja cuando gira.
- **Giro por paso:** manda `N<grados>` (por defecto 10°, antes 6° fijos).

## Pendiente para terminar (Marti)

1. **Revisar el módulo derecho.** En la prueba 1 dio cero pulsos y después tuvo un comportamiento errático. Intercambiar los módulos de lado para ver si el problema es el módulo o el cable/pin D2. Revisar la soldadura de la cápsula.
2. **Ajustar los potenciómetros** a la distancia de uso: los dos con el LED apagado en silencio, y los dos reaccionando a la voz. No hace falta que queden parejos, eso lo corrige el balance.
3. **Afinar el modo voz** (ya funciona): cargar el firmware y mandar `R`, `A`, `E1`, `L1`. Calibrar con `I` hablando desde el centro y guardar con `S`. Hablar desde los costados y verificar que gire hacia el lado correcto. Si gira al revés, cambiar `SENTIDO`.
4. **Ajustar las constantes** si hace falta (`VOZ_MIN_PULSOS`, `VOZ_ZONA`, `VOZ_PASO_DEG`, `VOZ_VENTANA_MS`). Si oscila alrededor del centro, subir `VOZ_ZONA` o bajar `VOZ_PASO_DEG`. Si no reacciona, bajar `VOZ_MIN_PULSOS`.
5. **App (`app/index.html`):** todavía no conoce el modo voz. Falta un botón para `E1`/`E0`, otro para calibrar balance (`I` + `S`), mostrar `bal`/`voz` de `@STATE` y graficar las líneas `@VOZ`.
6. **Si con estos módulos la voz no alcanza desde la distancia de uso:** cambiarlos por micrófonos con salida analógica y control automático de ganancia (MAX9814) o más ganancia (MAX4466). El ESP8266 tiene un solo ADC, así que para leer dos analógicos haría falta otra placa (ESP32) o un ADC externo.
