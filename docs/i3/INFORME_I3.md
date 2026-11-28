# Informe individual — Integrante 3 (ranking, historial y estadísticas)

> Borrador de la sección I3 para el informe del grupo. Las cifras salen de `benchmarks/i3/resultados/` y de las
> pruebas de `tests/i3`. Lo que no se midió ni se probó se dice en la sección 9. Las secciones de I1 e I2 las
> redactan sus autores; aquí no se resumen sus resultados.

## 1. Problema y alcance
Dados los resultados de combates terminados, el motor debe mantener estadísticas por luchador, un ranking
(puntos descendente, empates por id ascendente), el historial de los últimos combates y permitir anular un resultado.
Contrato y casos borde: `CONTRATO_I3.md`.

## 2. Operaciones que cuentan (propuesta, pendiente de acuerdo con I1 e I2)
`top_k(k)`, `posicion(id)` e `historial.anular(id)`. Las tres tienen candidatos con la misma operación y el
mismo contrato. Con I1 e I2 el equipo aún debe completar 6–8 operaciones; esto no está resuelto (Issue #3).

## 3. Candidatos
| Operación | Candidato A | Candidato B |
|---|---|---|
| `top_k` | AVL con tamaños de subárbol (propio) | heap construido en la consulta (propio) |
| `posicion` | AVL con tamaños | arreglo ordenado por (puntos, id) |
| historial | buffer circular | lista doblemente enlazada propia + índice id→nodo |

El AVL es estructura obligatoria del curso; sus invariantes (orden, balance, altura, tamaño) se verifican en las pruebas.

## 4. Escenarios y comparación
Tres escenarios por operación, 4 tamaños (10³–10⁶), 5 repeticiones, mismo flujo de operaciones para ambos candidatos,
checksum igual como prueba de equivalencia. Tabla de resultados, cocientes y gráficos: `CONTRATO_I3.md` §5 y
`benchmarks/i3/resultados/*.png|svg`. Resumen de lo medido: el heap gana `top_k` cuando hay muy pocas consultas y n
pequeño (a n=10⁶ gana el AVL aun en el torneo); el AVL gana en cuanto hay consultas y cambios intercalados; el
arreglo gana `posicion` solo lectura (≤1,5×) y con n=10³; el buffer circular gana sin anulaciones (1,9–2,4×) y en
espacio, pero pierde desde 1 % de anulaciones. Dos hipótesis iniciales quedaron corregidas por la medición (§6 del contrato).

## 5. Motor completo y decisión de integración
`MotorResultados` mantiene estadísticas, historial y ranking coherentes; `anular_resultado` revierte estadísticas y
ranking. El evento de fin de combate lleva `id_externo` para ser idempotente (`duplicado` / `conflicto_clave`).
`resultado_desde_combate` adapta un `Combate` terminado a ese evento.

## 6. Implementación
`src/headers/i3/` (solo headers, C++17, `std::` explícito). Índice id→slot intercambiable: `IndiceStd` o `IndiceHashI1`
(se activa si existe `i1/hash_table.h`; verificado contra la copia del PR #9: 460.000 operaciones, 0 fallos).

## 7. Pruebas
- Referencia lenta (`tests/i3/motor_lento.h`) y comparación diferencial: 6 combinaciones × 6 escenarios × 10.000
  operaciones + 40.000 de historial = 400.000 operaciones comparadas, 0 diferencias.
- Casos borde del motor (vacío, empates, anular, ids repetidos/desconocidos, desbordes, ids con acentos, idempotencia).
- Pruebas de mutación: 9 errores introducidos a propósito, los 9 detectados.
- Sanitizers (ASan/UBSan) y clang estricto sin advertencias. CI propuesto en `.github/workflows/i3-tests.yml`
  (ubuntu/macos/windows); **no se ha ejecutado en remoto**.

## 8. Mediciones: cómo reproducir
`python3 benchmarks/i3/ejecutar.py` (completo, sobrescribe resultados) o `--rapido` (ver `README_ENTREGA.md`). Entorno de las cifras publicadas:
`benchmarks/i3/resultados/entorno.txt` (una sola VM de 2 vCPU).

## 9. Límites
Medido en una sola máquina virtual; no comparable con benchmarks de otros integrantes. Las reglas de puntaje
(3/1/0) son propuesta. No hay concurrencia. La integración real I1+I2+I3 se probó solo en una copia desechable
(1821 combates, 0 diferencias; ninguno terminó en empate). MSVC y CI remoto sin ejecutar.

## 10. Herramientas y ayuda utilizada
Parte del código, las pruebas, los scripts y estos documentos se elaboraron con la ayuda de Claude (Claude Code)
bajo la dirección del integrante, quien los revisó. Declarar esto según la política de la asignatura.
