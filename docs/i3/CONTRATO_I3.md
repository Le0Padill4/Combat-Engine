# Contrato propuesto de I3: ranking, historial y estadísticas

Estado: **propuesta para revisión de I1 e I2** (Issue #3). No es un contrato aprobado.
Código: `src/headers/i3/`. Pruebas y referencia lenta: `tests/i3/`. Mediciones: `benchmarks/i3/`.

## 1. Evento mínimo que I3 necesita de un combate

`ResultadoCombate { id_a, id_b, id_ganador, turnos, id_externo }`

- `id_a`, `id_b`: ids de `PerfilPersonaje` (I1), tipo `std::string`.
- `id_ganador`: uno de los dos ids; **vacío = empate**.
- `turnos`: número de turnos ejecutados, entre 1 y 1.000.000.000.
- `id_externo`: **clave de idempotencia** que asigna quien emite el resultado (p. ej. el controlador del
  combate). 0 = sin clave (cada registro cuenta como nuevo). Con clave ≠ 0, el mismo combate no se cuenta dos
  veces: un reenvío idéntico devuelve `duplicado` (con el `id_combate` original) y no cambia nada; la misma
  clave con contenido distinto devuelve `conflicto_clave` y no cambia nada. Anular un resultado libera su
  clave, de modo que una corrección puede registrarse con la misma clave. Esto cubre el "procesamiento
  idempotente del evento de fin" que `docs/estado/integracion_registro_combate.md` deja como pendiente.

Mapeo previsto desde I2 (a confirmar): `turnos = Combate::turnosEjecutados()`, `id_ganador` desde
`Combate::ganador()`. `ganador() == nullptr` ocurre **también con el combate sin terminar**, así que el
adaptador debe comprobar `terminado()` antes de registrar; si ambos están sin vida, es empate.
I3 no conoce `Personaje`: el adaptador vive en la integración, no en estos módulos.

Reglas de puntaje (propuestas): victoria 3, empate 1, derrota 0. Ranking: más puntos primero; a igual
puntaje, id menor (orden de bytes de `std::string`).

## 2. Operaciones y casos borde (política idéntica en el motor y en la referencia lenta)

| Operación | Entrada | Devuelve | Cambia | Casos borde |
|---|---|---|---|---|
| `registrar_luchador(id)` | id | `bool` | alta con 0 puntos | id vacío o repetido → `false`, sin cambios |
| `registrar_resultado(r)` | `ResultadoCombate` | `{estado, id_combate}` | estadísticas de ambos, ranking, historial | se valida en este orden: `ids_iguales`, `id_desconocido` (cualquiera de los dos), `ganador_invalido` (no vacío y distinto de a y b), `turnos_invalidos` (0 o > 10⁹) y, por último, la idempotencia (`duplicado`, `conflicto_clave`). Si falla **no cambia nada**; `id_combate = 0`, salvo en `duplicado` (id original) |
| `anular_resultado(id_combate)` | id | `bool` | revierte estadísticas y ranking | fuera de la ventana (inexistente, ya anulado o expulsado) → `false`, sin cambios |
| `top_k(k)` | k | `vector<EntradaRanking>` | — | k = 0 o motor vacío → vacío; k > n → n entradas |
| `posicion(id)` | id | `optional<size_t>` (1 = primero) | — | id inexistente → `nullopt` |
| `estadisticas(id)` | id | `optional<Estadisticas>` | — | id inexistente → `nullopt` (operación simple, no se analiza) |
| `ultimos(m)` | m | eventos, del más reciente al más antiguo | — | m = 0 o sin eventos → vacío |

Constructor: capacidad de historial 0 lanza `std::invalid_argument`. Los `id_combate` son crecientes y
los asigna el motor, empezando en 1. Anular un combate no reutiliza su id.
Las claves de idempotencia no se acotan con la ventana del historial: un resultado ya expulsado de la ventana
sigue contando como `duplicado` si se reenvía, aunque ya no se pueda anular. Su costo de espacio es O(resultados
con clave).
Límite de 10⁹ turnos por combate: evita desbordes en `turnos_totales` (uint64) y permite revertir con exactitud.

## 3. Operaciones que cuentan (propuesta de I3) y candidatos

| Operación | Candidato 1 | Candidato 2 | Mismo contrato |
|---|---|---|---|
| `top_k(k)` con puntajes que cambian | **AVL** propio con tamaños de subárbol | **Heap en la consulta** (`std::make_heap` sobre las claves) | sí: mismos resultados, comprobado por checksum en cada medición |
| `posicion(id)` con puntajes que cambian | **AVL** aumentado | **Arreglo ordenado** + búsqueda binaria | sí |
| historial acotado: `registrar` / `ultimos` / `anular` | **Buffer circular** (arreglo fijo) | **Lista doblemente enlazada** propia + índice id→nodo | sí, incluida la expulsión del más antiguo y la anulación |

La comparación buffer circular vs lista **solo es honesta si incluye `anular(id)`**: sin esa operación
ambos son O(1) y el buffer gana siempre. Con ella cada uno tiene un escenario propio (sección 5).
Las estadísticas por id (alta y consulta) son una operación simple: se implementan y se prueban, pero
no se analizan. El índice id→slot reutiliza la tabla hash de I1 (PR #9) mediante un adaptador; mientras
no esté en `main` se usa `std::unordered_map` (`IndiceStd`). I3 **no duplica** la tabla hash.

Falta para llegar a 6–8 operaciones del equipo: las de I1 (búsqueda por id, hash vs vector ordenado) y las
de I2 (el análisis del heap de I2 concluye que alternar dos actores no justifica un heap). **Hay un hueco que el
equipo debe cerrar en el Issue #3.**

## 4. Complejidad (peor caso salvo indicación) y costo compartido

| Estructura / operación | Tiempo | Espacio extra |
|---|---|---|
| AVL: insertar, eliminar, `posicion` | O(log n) | O(n): 72 B por nodo (+40 B del almacén común por luchador; 112 B por luchador en total, estimación estructural) |
| AVL: `top_k(k)` | O(log n + k) | O(k) para la salida |
| Heap en consulta: `fijar_puntos` | O(1) | — |
| Heap en consulta: `top_k(k)` | O(n + k log n) | O(n) temporal por consulta (16 B por luchador) |
| Heap en consulta: `posicion` | O(n) | — |
| Arreglo ordenado: `posicion` | O(log n) | O(n): 16 B por luchador |
| Arreglo ordenado: `fijar_puntos` | O(log n + d), d = distancia que se mueve la clave, hasta O(n) | — |
| Arreglo ordenado: `agregar` luchador | O(n) | — |
| Buffer circular: `registrar` | O(1) | O(capacidad) reservado desde el inicio |
| Buffer circular: `anular` | O(log N) buscar + O(N) desplazar | — |
| Lista doble: `registrar`, `anular` | O(1) esperado (índice hash) | O(N): nodo + entrada del índice |
| `ultimos(m)` (ambos) | O(m) | O(m) |

**Costo compartido de un combate:** `registrar_resultado` hace 2 actualizaciones de ranking (con AVL,
2 × O(log n) cada una porque es eliminar + insertar), 2 de estadísticas O(1) y 1 `registrar` en el
historial. `anular_resultado` repite lo mismo al revés. Mantener ranking, estadísticas e historial
coherentes es lo que `MotorResultados::verificar_invariantes()` comprueba.

## 5. Escenarios y resultados medidos

Mediciones reales en `benchmarks/i3/resultados/` (360 filas: 3 operaciones × 3 escenarios ×
2 candidatos × 4 tamaños × 5 repeticiones). Cada medición ejecuta 2.000 operaciones con el mismo flujo
para ambos candidatos; la carga inicial no se cuenta. Cociente = tiempo del perdedor / tiempo del ganador,
sobre medianas.

| Operación | Escenario | Gana | Evidencia medida |
|---|---|---|---|
| `top_k` | torneo: 1999 cambios y **1 consulta** al final | Heap en consulta | n=10³: 54×, 10⁴: 10×, 10⁵: 2,7×. **A n=10⁶ gana el AVL (2,25×)** |
| `top_k` | ráfaga: 1 % de consultas | Heap solo a n=10³ (3,2×); AVL desde 10⁴ (1,6×, 6×, 43×) | |
| `top_k` | ranking en vivo: 50 % de consultas | AVL | 11× a n=10³ hasta ~4.000× a 10⁶ |
| `posicion` | solo lectura (0 cambios) | Arreglo ordenado | n=10⁴: 1,14×, 10⁵: 1,41×, 10⁶: 1,47× (a 10³ empate) |
| `posicion` | 95 % lectura / 90 % cambios | AVL desde n=10⁴ (hasta 43× con 90 % de cambios); **el arreglo gana a n=10³ con 90 % de cambios (1,42×)** | |
| historial | sin anulaciones | Buffer circular | 1,9× a 2,4× en los cuatro tamaños |
| historial | 1 % de anulaciones | Lista | n=10³: 1,4×; 10⁴: 8×; 10⁶: ~990× |
| historial | 30 % de anulaciones | Lista | 39× a n=10³ hasta ~9.000× a 10⁶ |
| espacio (estimación estructural) | — | Arreglo ordenado: 0,5× del AVL; heap en consulta: 0,36×; buffer circular: 0,68× de la lista (n=10⁶) | |

Estas son las ventajas **demostradas**; ninguna se afirma sin medirla. Un hallazgo que corrige la hipótesis
inicial: con anulaciones uniformes en la ventana, el buffer circular pierde ya con un 1 %; solo gana cuando
casi no hay anulaciones (el punto de equilibrio baja como ~1/N).

## 6. Medición: qué confirma y qué contradice el análisis

- **Confirma:** el heap en consulta crece linealmente con n (`ranking en vivo`, tiempo medio por operación: 3,3 µs → 8,3 ms de n=10³ a 10⁶);
  el buffer circular al anular crece linealmente con N; el AVL crece de forma logarítmica.
- **Contradice o matiza:** (a) a n=10³ el arreglo ordenado actualiza más rápido que el AVL pese a ser O(n):
  las constantes pesan más que el crecimiento; (b) el heap en consulta no gana el torneo a n=10⁶: el punto de
  cruce depende de cuántas actualizaciones hay entre consultas (aquí 2.000), no solo de n; (c) la ventaja del
  arreglo en solo lectura es modesta (≤ 1,5×), no un orden de magnitud.

## 7. Qué NO demuestran las pruebas ni las mediciones

- Las mediciones se hicieron **en una sola máquina virtual** (Linux, 2 vCPU Intel Xeon 2,1 GHz, GCC 13.3 `-O2`),
  no en los equipos del equipo; no sirven para comparar con los benchmarks de I1, hechos en otro computador.
  Hay dispersión visible (barras min–max) y valores atípicos aislados a n=10³.
- Claves con muchos empates (puntajes 0..1000 entre hasta 10⁶ luchadores) e ids de 8 caracteres; otros
  tamaños de id o distribuciones podrían mover los puntos de cruce.
- Los bytes son una **estimación estructural**, no memoria residente: excluyen buffers de strings largos y
  metadatos del asignador.
- Las pruebas no cubren concurrencia, la integración real con `Combate` (I2) ni con `RegistroPersonajes` (I1),
  ni compilación con MSVC en local (la cubre el CI propuesto).
- La comparación diferencial valida equivalencia con la referencia lenta, no que las reglas de puntaje sean las
  correctas para el juego: esas reglas son una propuesta.

## 8. Verificación realizada (localmente, en la máquina virtual de trabajo)

- 400.000 operaciones aleatorias comparadas: 360.000 del motor completo contra la referencia lenta
  (6 combinaciones de candidatos × 6 escenarios × 10.000, semillas fijas) y 40.000 del historial solo contra
  un modelo con `std::vector`; 0 divergencias, 0 fallos de invariantes. Con el adaptador de la tabla hash de I1
  (copia temporal de `hash_table.h` del PR #9, no incluida en el repositorio): 460.000 operaciones, 0 fallos.
- Compilación sin avisos con `-Wall -Wextra -Wpedantic -Werror` (GCC y clang, además `-Wconversion`
  `-Wshadow`), con ASan/UBSan y con `-D_GLIBCXX_DEBUG`.
- Comprobación de la potencia de las pruebas: se introdujeron 9 errores deliberados en copias del código
  (balance del AVL, desplazamiento del arreglo, índice de la lista, reversión de puntos, avance del buffer,
  desempate del heap, conflicto tratado como duplicado, anulación sin liberar la clave y clave sin guardar) y la
  suite detectó los 9. Esas copias no forman parte del repositorio.
- Pendiente: ejecución del CI remoto (Ubuntu, macOS, Windows/MSVC), que aún no se ha disparado.

## 9. Reproducir

```sh
cmake -S tests/i3 -B build/i3 -DCMAKE_BUILD_TYPE=Release
cmake --build build/i3 --config Release
ctest --test-dir build/i3 -C Release --output-on-failure      # pruebas + humo del benchmark
python3 benchmarks/i3/ejecutar.py                             # mediciones completas (≈ 6 min)
```

`ejecutar.py` compila, mide, escribe `mediciones.csv`, `entorno.txt` y `medianas.csv`, y grafica con
matplotlib (dependencia **solo de desarrollo**; el motor C++ no la usa).
