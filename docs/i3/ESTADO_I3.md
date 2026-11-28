# Estado — Integrante 3

Actualizado: 27 de noviembre de 2026. Registro individual para evitar
conflictos sobre `docs/ESTADO_PROYECTO.md`; enlazarlo desde ese archivo solo con coordinación.

## Base y coordinación

- Repo comprobado: `origin = https://github.com/Le0Padill4/Combat-Engine`; `main` en `a9e7562`.
- Identidad de Git de los commits: `Soaks2112 <leonardogavilanes1234@gmail.com>` (confirmada por el usuario).
- Ramas y PRs consultados el 27/11: PR #1 (docs), #6 (build base; CI verde en Ubuntu, macOS y Windows),
  #7 y #8 (I2), #9 (I1); Issues #2–#5 sin asignados. No se fusionó ni modificó nada ajeno.
- Rama de trabajo: `feat/i3-ranking-avl`, creada desde `main` sin depender de `main.cpp` (que no
  compila hasta fusionar el PR #6). Los tests de I3 son un proyecto CMake aislado en `tests/i3`.
- No se tocaron `CMakeLists.txt` raíz, `api/`, `public/`, `vercel.json` ni contratos compartidos.

## Bloqueo vigente: publicación

El acceso de **escritura** a GitHub desde la sesión de trabajo fue rechazado (la app de Claude no está
instalada en `Le0Padill4/Combat-Engine`). Los commits existen solo localmente en el entorno de trabajo y se
entregan como parches (`git am`). **Todavía no hay PR ni CI remoto de esta rama.** Falta: aplicar los parches,
`git push`, abrir el PR de borrador y vincularlo al Issue #3/#4/#5.

## Trabajo realizado

1. `ArbolAVL` propio con tamaños de subárbol e invariantes (orden, balance, altura, tamaño).
2. Tres candidatos de ranking con la misma interfaz (`RankingAVL`, `RankingHeapSelect`, `RankingArreglo`).
3. Dos candidatos de historial acotado (`HistorialCircular`, `HistorialLista` con lista doble propia).
4. `MotorResultados` con estadísticas, historial y ranking coherentes; índice id→slot por adaptador
   (`IndiceStd` temporal; `IndiceHashI1` se activa solo si existe `i1/hash_table.h`).
5. Referencia lenta del motor completo, generador reproducible (`std::mt19937`, sin
   `uniform_int_distribution`, para reproducibilidad entre plataformas) y comparación diferencial.
6. Benchmarks de 3 operaciones × 3 escenarios × 2 candidatos × 4 tamaños × 5 repeticiones, con CSV,
   `entorno.txt`, `medianas.csv` y gráficos PNG/SVG.
7. Contrato propuesto y análisis: `docs/i3/CONTRATO_I3.md`.

## Evidencia realmente ejecutada

Ver la sección 8 de `CONTRATO_I3.md` (pruebas, comparaciones, sanitizers, mutaciones) y la 5–6 (mediciones).
Entorno de las mediciones: `benchmarks/i3/resultados/entorno.txt`. **No** se ejecutó el CI remoto ni MSVC.

## Correcciones al propio diseño durante el trabajo

- La primera medición mostró que el buffer circular no ganaba en ningún escenario con anulaciones; se
  añadió un escenario sin anulaciones y se documentó que su ventaja solo existe ahí (y en espacio).
- Los candidatos arreglo y heap leían los puntajes por indirección, lo que los perjudicaba; se
  reescribieron con la clave inline y se **repitió toda la medición**. Los resultados publicados son los de
  la segunda ejecución completa.

## Pendiente

| Tarea | Depende de |
|---|---|
| Publicar la rama, abrir PR de borrador, solicitar revisión cruzada | acceso de escritura |
| Cerrar operaciones y contrato con I1 e I2 (hueco para llegar a 6–8) | Issue #3 |
| Adaptador `Combate` → `ResultadoCombate` y ranking sobre `RegistroPersonajes` | PR #7/#8/#9 en `main` |
| Reemplazar `IndiceStd` por la `HashTable` de I1 | PR #9 en `main` |
| Pruebas diferenciales del motor completo de los tres integrantes | integración de I1 e I2 |
| Informe, README final y ZIP (`proyecto-<apellidos>.zip`, sin ejecutables ni historial) | todo lo anterior |
