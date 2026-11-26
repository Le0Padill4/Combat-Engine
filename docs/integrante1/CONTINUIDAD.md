# Continuidad del integrante 1

Actualizado: 26 de noviembre de 2026. El usuario autorizo crear commit, subir
la rama y abrir un PR en borrador. Este avance se presenta para revision;
la integracion en main y la aprobacion del contrato siguen pendientes.

## Base y coordinacion

- Repo comprobado: `https://github.com/Le0Padill4/Combat-Engine`.
- Rama: `feat/i1-registro-personajes`, creada desde `main` a9e7562 despues de
  `git fetch origin` y comprobar que el arbol estaba limpio.
- Las ramas remotas de documentacion y baseline se consultaron sin mezclarlas
  ni modificar sus archivos. PRs #1 y #6 abiertos, Issues #2-#5 sin asignados.
- La consulta del 26 de noviembre encontro tambien los PRs en borrador #7 y #8
  del integrante 2. Sus archivos de combate no se incorporaron ni modificaron.
- El CI remoto satisfactorio del PR #6 pertenece a ese PR, no a esta rama local.
- No se editaron archivos existentes, contratos compartidos o trabajo ajeno.
  La continuidad individual evita conflictos sobre `docs/ESTADO_PROYECTO.md`.

## Trabajo realizado

1. Especificacion individual de registro/consulta, escenarios BDD y contrato
   ejecutable local, pendiente de revision del equipo.
2. Perfiles por ID con atributos/habilidades y adaptador de lectura de `Personaje`.
3. Hash propia con encadenamiento, colisiones, duplicados, rehash e invariantes.
4. Candidato con vector ordenado y referencia lenta lineal.
5. Pruebas de contrato, casos borde, snapshots y fallo simulado de copia en rehash.
6. Comparacion diferencial reproducible de 36.000 operaciones del registro.
7. Benchmark de busqueda por ID con dos candidatos, dos escenarios, cuatro
   tamanos y cinco repeticiones; CSV, entorno, medianas y graficos SVG/PNG.
8. Automatizacion local Node y build CMake independiente propuesto.
9. Workflow individual para ejecutar CMake/CTest y las pruebas Node en Linux,
   macOS y Windows. Consultar los checks de la rama/PR para el resultado remoto.

## Evidencia realmente ejecutada

Entorno: GCC TDM64 9.2.0, C++17, Windows x64. Las versiones detalladas y flags
estan en `benchmarks/i1/resultados/entorno.txt`.

| Comprobacion | Resultado |
| --- | --- |
| Primera compilacion de la suite antes de implementar | Fallo esperado: cabeceras i1 ausentes |
| `node scripts/i1/verificar.js all` | Ocho grupos C++ satisfactorios; benchmark y graficos generados |
| Compilacion adicional con `-O1 -g -D_GLIBCXX_DEBUG -Wall -Wextra -Wpedantic -Werror` | Los mismos ocho grupos satisfactorios con comprobaciones de contenedores de libstdc++ |
| Diferencial, semilla 7 | 12.000 operaciones; 2.804 consultas ausentes, 3.583 duplicados, 788 invalidos |
| Diferencial, semilla 42 | 12.000 operaciones; 2.769 consultas ausentes, 3.616 duplicados, 760 invalidos |
| Diferencial, semilla 20261125 | 12.000 operaciones; 2.798 consultas ausentes, 3.567 duplicados, 794 invalidos |
| `node --test tests/battle-contract.test.js tests/vercel-api.test.js` | Siete pruebas satisfactorias |
| Revalidacion antes de publicar, 26 de noviembre | Ocho grupos C++ y siete pruebas Node satisfactorios |
| Inspeccion del CSV | 80 filas: 2 candidatos x 2 escenarios x 4 tamanos x 5 repeticiones; un millon de consultas por fila |
| Revision visual de graficos PNG | Ejes, leyendas y textos revisados; SVG/PNG reproducidos desde el CSV |
| Revision local de archivos | 21 archivos nuevos, incluido el workflow individual; ningun archivo del baseline modificado |

Los contadores de ausentes/duplicados/invalidos describen subconjuntos del flujo,
no tres grupos de operaciones adicionales. Las 36.000 operaciones corresponden
solo al registro, no al motor completo. Los 80 millones de consultas del benchmark
son un experimento distinto y no sustituyen la comparacion diferencial global.

## Resultados observados y limites

Medianas, en ns por consulta, del CSV local:

| Perfiles | Uniforme: hash | Uniforme: vector | Concentrada: hash | Concentrada: vector |
| --- | --- | --- | --- | --- |
| 1.000 | 54,003 | 131,685 | 49,683 | 135,528 |
| 4.000 | 58,016 | 151,914 | 50,685 | 134,549 |
| 16.000 | 78,490 | 183,999 | 60,509 | 155,668 |
| 64.000 | 176,068 | 287,540 | 73,006 | 181,102 |

La hash obtuvo menor latencia en todas las celdas medidas. El vector uso entre
34,30% y 34,61% menos espacio **estructural estimado**. Ese calculo no incluye
buffers dinamicos de strings ni metadatos del asignador y no equivale a memoria
total o RSS. Los resultados pertenecen a este computador, compilador y cargas;
no prueban superioridad universal ni comportamiento frente a entradas adversarias.

El crecimiento de latencia de la hash al aumentar n es compatible con efectos de
cache/constantes, pero no se midieron contadores de hardware para atribuir una
causa. La cota esperada constante no garantiza tiempo fisico constante.

La construccion usa IDs ascendentes y se registra como dato diagnostico;
no representa altas aleatorias. No se ha medido una segunda operacion del motor.

## Pendientes y siguiente paso

- Publicar mediante la autorizacion expresa del 26 de noviembre. Descripcion
  revisable del PR en borrador: `PR_PROPUESTO.md`. No integrar en main.
- Revision cruzada del contrato con integrantes 2/3 y acuerdo de cambios
  compartidos. Usar este contrato como propuesta, no presentarlo como aprobado.
- Integracion de PRs #1 y #6 por el equipo; el juego en `main` todavia tiene el
  include obsoleto. No corregirlo otra vez desde esta rama.
- La ruta CMake no se ejecuto localmente; comprobar el resultado de CMake/CTest
  en el workflow propio del PR antes de integrar. No atribuirle el CI de otro PR.
- Cerrar operaciones contables, tercer escenario global, segunda estructura
  propia, mantenimiento de indices y benchmark de la otra operacion.
- Comparacion diferencial del motor completo, informe colectivo, ZIP y defensa.
- Confirmar fecha oficial y aceptacion del trio con el curso.

Para retomar: leer este archivo y el README individual, comprobar `git status`,
consultar GitHub y no descartar el trabajo local. Las ramas posteriores del plan
seran PRs separados cuando el equipo acuerde el contrato y una base integrada.
