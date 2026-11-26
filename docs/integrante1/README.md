# Integrante 1: registro de personajes

Estado al 26 de noviembre de 2026: implementacion aislada en
`feat/i1-registro-personajes`, basada en `main` a9e7562. El usuario autorizo
commit, push y PR en borrador para revision. La publicacion no supone integrar
el modulo ni aprobar el contrato del equipo.

## Alcance y acuerdos pendientes

Este modulo aporta definiciones de personajes, registro por ID, consulta por ID,
una hash propia con encadenamiento, un candidato con vector ordenado y una
referencia lenta. No sustituye el combate ni modifica sus clases, reglas, UI,
API Vercel, CMake raiz o documentos compartidos.

El plan aprobado por el usuario autoriza este avance individual. El contrato
ejecutable de abajo es una propuesta para revisar con integrantes 2 y 3; no
constituye aprobacion de las 6-8 operaciones del motor completo. Los PRs #1
(documentacion) y #6 (baseline/CI) seguian abiertos en la consulta de GitHub.

El PDF oficial aportado indica 29 de noviembre de 2026 a las 23:59 y equipos
de dos. El Markdown del curso y el prompt individual indican 30 de noviembre;
la organizacion del repositorio contempla tres integrantes. Ambas discrepancias
requieren confirmacion del curso. Se conserva el plazo preventivo del 29.

## Archivos y responsabilidades

| Ruta | Proposito |
| --- | --- |
| `src/headers/i1/perfil_personaje.h` | Perfil, atributos, habilidades, validacion y adaptador de lectura |
| `src/headers/i1/hash_table.h` | Hash propia con encadenamiento y verificador de invariantes |
| `src/headers/i1/registro_personajes.h` | Dos candidatos con la misma API de registro/consulta |
| `tests/i1/soporte/registro_lento.h` | Referencia lineal solo del registro |
| `tests/i1/registro_tests.cpp` | Casos borde, invariantes y comparacion diferencial |
| `tests/i1/CMakeLists.txt` | Build independiente del CMake compartido |
| `.github/workflows/i1-registro-tests.yml` | CMake/CTest y pruebas Node en Linux, macOS y Windows |
| `spec/features/i1/registro_personajes.feature` | Escenarios BDD respaldados por las pruebas C++ |
| `scripts/i1/verificar.js` | Compilacion, ejecucion y mediciones sin paquetes Node adicionales |
| `benchmarks/i1/` | Experimento de busqueda por ID, resultados y graficos |
| `docs/integrante1/` | Contrato propuesto, limites, evidencia y continuidad individual |

## Contrato local propuesto

Namespace: `combat::i1`. C++17. No hay persistencia ni acceso concurrente.

`PerfilPersonaje` contiene `id`, `alias`, `atributos` y `habilidades`:

- Atributos: `hp_max >= 1`, `ki_max >= 0`, `ataque >= 1`, `defensa >= 0`.
- Habilidades: nombres normal/especial no vacios y `costo_ki_especial >= 0`.
- ID, alias y nombres deben contener al menos un caracter que no sea espacio
  segun `std::isspace` del proceso. No se normaliza Unicode ni se recortan textos.
- Todos los numeros usan `int`. No se impone un limite superior adicional.
  Guardar un valor grande no demuestra que la aritmetica del combate lo soporte.
- El costo especial puede exceder el KI maximo: el registro no cambia esa regla
  del modelo existente. Cualquier restriccion adicional requiere acuerdo.
- El ID es opaco, exacto y sensible a mayusculas; los alias pueden repetirse.
  No hay reasignacion automatica ni cambios de ID, edicion o borrado en esta API.
- Un perfil por defecto es invalido. Los atributos tienen inicializacion definida.

| Operacion | Retorno | Efecto y bordes |
| --- | --- | --- |
| `registrar_personaje(const PerfilPersonaje&)` | `ResultadoRegistro` | Primero valida; si falla devuelve `perfil_invalido`. Si el ID existe devuelve `id_duplicado`. Solo `registrado` agrega una entrada. No reemplaza perfiles. |
| `buscar_personaje(const std::string&) const` | `std::optional<PerfilPersonaje>` | Devuelve copia completa o `nullopt`, incluso en registro vacio. Sin mutacion. |
| `tamano() const` | `size_t` | Operacion auxiliar, no se cuenta como operacion academica. |
| `verificar_invariantes() const` | `bool` | Instrumentacion de pruebas, fuera de los intervalos cronometrados. |

La validacion precede a la deteccion de duplicados. Un perfil invalido con ID
ya existente devuelve `perfil_invalido`. Los rechazos no cambian datos logicos.
Errores de memoria/copia se propagan como excepciones; no se convierten en una
respuesta de negocio. Una insercion fallida puede haber ampliado capacidad sin
agregar un perfil. El test de fallo durante rehash comprueba que un fallo al
copiar entradas conserva el contenido previo.

`perfil_desde_personaje(id, personaje)` lee maxima vida/KI, ataque, defensa,
nombres y costo de habilidades mediante getters existentes. No consulta como
definicion la vida/KI actuales ni cambia defensa, turnos o ataques. Produce una
copia; no fabrica instancias de subclases ni ejecuta habilidades.

El adaptador no valida por si solo el resultado: registrar realiza la validacion.
El llamador acuerda el ID. La correspondencia existente a preservar al integrar
es `personaje-1` Sexitar, `personaje-2` Francois, `personaje-3` Leo2 y
`personaje-4` Leo1, manteniendo los nombres UTF-8 reales del proyecto.

## Estructura propia e invariantes

La hash implementa seleccion de bucket, recorrido de colisiones, rechazo de
duplicados y rehash. `std::vector` solo aporta almacenamiento secuencial.
Capacidad inicial 8; una capacidad solicitada de cero se transforma en uno.
Crece duplicando hasta que la nueva entrada respete carga maxima 0.75.

Invariantes comprobados:

1. Hay al menos un bucket.
2. Cada clave pertenece a `hash(clave) % capacidad`.
3. No hay claves equivalentes repetidas dentro de un bucket.
4. La suma de longitudes de buckets coincide con `tamano`.
5. El factor de carga no excede 0.75.

Como en otros contenedores hash, el hasher debe ser determinista y las claves
equivalentes deben producir el mismo hash. Las pruebas fuerzan un hash constante
para verificar el peor caso de colisiones, independientemente de `std::hash`.

La API generica de la hash devuelve punteros constantes internos; pueden
invalidarse tras cualquier insercion o asignacion. El registro publico siempre
devuelve snapshots por valor. La insercion copia sus argumentos antes del
rehash para admitir un argumento que apunte a un valor de la propia tabla.

El vector alternativo mantiene IDs estrictamente crecientes y perfiles validos.
La referencia usa recorrido lineal e insercion al final. Comparte el tipo y la
validacion del dominio; por eso la suite tambien prueba la validacion contra
expectativas explicitas y no solo contra la referencia.

## Complejidad y mantenimiento compartido

Sean `n` perfiles, `m` buckets, `L` longitud de ID y `P` costo de copiar/validar
un perfil. El analisis no supone strings de longitud cero.

| Operacion | Hash propia | Vector ordenado | Referencia lineal |
| --- | --- | --- | --- |
| Buscar | Esperado `O(L + P)`; peor `O(nL + P)` | Peor `O(L log n + P)` | Peor `O(nL + P)` |
| Registrar sin crecimiento | Esperado `O(L + P)`; peor `O(nL + P)` | Peor `O(L log n + nP)` por desplazamientos | `O(nL + P)` mas eventual realocacion |
| Rehash / realocacion | `O(m + n(L + P))`; insercion amortizada esperada `O(L + P)` | Puede copiar/mover `O(nP)` | Puede copiar/mover `O(nP)` |
| Espacio | `O(m + nP)` | `O(nP)` | `O(nP)` |

Con crecimiento geometrico automatico, `m = O(n)` para registros grandes.
Las comprobaciones de unicidad de un bucket pueden costar `O(n^2 L)` cuando
todas las claves colisionan. No son parte del costo de la operacion publica.

El registro almacena definiciones y no estado de combate. El integrante 2 debe
acordar como resolver/copiar un perfil al crear combatientes; el integrante 3,
como validar un ID al registrar resultados. Un indice adicional de ranking
incrementa el costo de cada alta que deba mantenerlo. Todavia no existe contrato
de transaccion/rollback entre indices; esta version no asegura atomicidad entre
modulos ni expone borrado para simular una solucion no acordada.

## Ejecutar pruebas

Desde la raiz, con Node y g++/clang++ C++17:

```sh
node scripts/i1/verificar.js test
node --test tests/battle-contract.test.js tests/vercel-api.test.js
```

Si el compilador no esta en PATH:

```sh
node scripts/i1/verificar.js test --cxx "ruta/al/g++.exe"
```

El script reconoce la ubicacion local de Dev-Cpp como alternativa en Windows;
no instala dependencias ni modifica la politica de ejecucion de PowerShell.
Los ejecutables se guardan en `build/i1`, ya ignorado por el repositorio.

Tambien se ofrece un proyecto CMake independiente. No se ejecuto localmente
por ausencia de CMake; el workflow individual lo configura, compila y prueba
en tres sistemas. Los resultados remotos se consultan en los checks del PR:

```sh
cmake -S tests/i1 -B build/i1-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/i1-cmake --config Release
ctest --test-dir build/i1-cmake -C Release --output-on-failure
```

Se escribio la suite antes de implementar las cabeceras; la primera compilacion
fallo por su ausencia. Luego se implemento el contrato y se ejecutaron las
pruebas. Los escenarios BDD son especificaciones legibles; no se afirma haber
ejecutado Cucumber ni otro runner de Gherkin.

La suite ejecuta 12.000 operaciones por semilla (7, 42 y 20261125): 36.000 en
total, verificando respuesta, tamano e invariantes tras cada paso. Compara hash,
vector ordenado y referencia, y comprueba los perfiles finales. Hay rechazos por
duplicados, datos invalidos y consultas de IDs ausentes. Incluye pruebas de
snapshots, copia de hash, fallo de copia durante rehash y lectura no invasiva de
`Personaje`.

Esto verifica registro/consulta; no las 10.000 operaciones requeridas sobre el
motor completo. No demuestra ausencia de todos los errores de memoria,
correctitud concurrente ni seguridad aritmetica de los ataques actuales.

## Repetir las mediciones

```sh
node scripts/i1/verificar.js benchmark --repeticiones 5
```

R base crea graficos SVG y PNG, sin paquetes adicionales. Si R no esta instalado,
el CSV se genera igualmente; los graficos se pueden reproducir despues con:

```sh
Rscript --vanilla benchmarks/i1/graficar.R benchmarks/i1/resultados
```

El experimento mide **una operacion**, `buscar_personaje`, sobre dos candidatos,
dos escenarios, cuatro tamanos (1.000, 4.000, 16.000 y 64.000) y cinco repeticiones.
Cada lote contiene un millon de consultas. Los tamanos son una seleccion practica
para esta maquina; no se afirma haber medido el ejemplo de un millon de perfiles
de la guia. La construccion recibe IDs ascendentes identicos en ambos candidatos.

- `uniforme_80_aciertos`: 80% esperado de consultas presentes uniformes, resto
  IDs ausentes. El numero exacto de aciertos queda en el CSV.
- `popular_90_por_ciento`: todos los IDs existen; 90% esperado de consultas se
  concentra en 32 IDs y el resto es uniforme.

Fixtures y consultas se generan fuera del intervalo medido. Se comparan perfiles
completos de ambos candidatos antes de medir cada escenario por primera vez.
Hay calentamiento de 1.000 consultas; se alterna el orden de candidatos por
repeticion. El retorno por copia, la busqueda y la acumulacion de checksum estan
dentro del intervalo. Cada lote verifica aciertos y checksum conocidos por el
generador, no por la hash. Se conserva el entorno de compilacion en `entorno.txt`.

La columna `construccion_ns` es diagnostica, una medicion por reconstruccion;
puede ser cero en tamanos pequenos por resolucion del reloj. No se usa para
proclamar un benchmark independiente de registro. Los lotes de consulta se
ampliaron desde 50.000 a un millon tras observar cuantizacion en los intervalos
cortos del compilador instalado.

`bytes_estructura_estimados` suma objetos y capacidades de contenedores. Excluye
buffers dinamicos de strings y metadatos del asignador; no mide RSS, picos del
rehash ni el espacio total del proceso. Ambos candidatos guardan el mismo
payload, pero la hash ademas duplica la clave del perfil como clave del indice.

La hash se propone para consultas exactas frecuentes; el vector tiene una
ventaja de espacio estructural estimado en estas cargas. No se promete una
ventaja temporal del vector. Para contar esta operacion academicamente, el equipo
debe justificar el escenario donde ese menor espacio importe al producto y
evaluar el costo total de mantenimiento. Los resultados no cierran el Issue #3.

Faltan un tercer escenario global y el benchmark de otra operacion, a coordinar
con el integrante 3. Propuestas para el contrato global, aun no medidas:

| Escenario | Mezcla propuesta | Datos |
| --- | --- | --- |
| Catalogo de consulta | 95% busquedas, 5% altas | 1.000-64.000 perfiles, IDs uniformes |
| Expansion del catalogo | 60% altas, 40% busquedas | 1.000-64.000 perfiles, altas desordenadas y duplicados |
| Sesion de torneo | 60% acciones, 20% resultados, 15% busquedas, 5% top-K | Escala y distribucion por acordar con integrantes 2/3 |

## Limites de integracion y entrega

- No se modificaron `Personaje`, el frontend ni la API; su comportamiento no se
  redisenia. Sus pruebas Node son una comprobacion adicional de regresion.
- `main` conserva el error de `clase.h`; la correccion corresponde al PR #6.
  No se declara compilacion del juego completo por probar este modulo aislado.
- `datos.txt` declara ocho campos pero las filas tienen siete y omiten el costo
  especial. Este modulo no lo importa ni inventa esos valores.
- El acuerdo de operaciones, la segunda estructura propia, las tres familias y
  las pruebas del motor completo siguen siendo tareas de equipo.
- Antes de integrar: actualizar GitHub, acordar interfaces/archivos compartidos,
  revisar este PR con otro integrante y exigir CI satisfactorio.
- La entrega academica debe excluir ejecutables e historial Git y contener el
  mismo ZIP para todos. Este avance individual no se presenta como ZIP final.

Las herramientas utilizadas fueron Codex para implementacion y revision local,
GCC para compilar, Node para automatizacion/pruebas existentes y R base para
graficos. No se atribuyen contribuciones de este modulo a otros integrantes.

La evidencia y el siguiente paso estan en `CONTINUIDAD.md`; el borrador de PR
revisable esta en `PR_PROPUESTO.md`.
