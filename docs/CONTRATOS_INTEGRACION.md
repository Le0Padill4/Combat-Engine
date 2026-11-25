# Contratos de integración

Estado: borrador de equipo; antes de escribir código académico, completar las entradas, retornos y bordes de las 6–8 operaciones que cuentan.

## Dominios

- `Personaje`: id estable, alias ficticio, atributos acotados, estado de combate.
- `Combate`: dos combatientes distintos, actor, acción, recursos, resultado y eventos.
- `Registro/Ranking`: perfiles sintéticos, historial limitado y clave de ranking con desempate determinista.
- `ReferenciaLenta`: listas/vectores sencillos que entregan mismos resultados para operaciones equivalentes.

## Interfaces candidatas

`buscar_personaje(id)`, `registrar_personaje(...)`, `aplicar_accion(estado, accion)`, `registrar_resultado(...)`, `top_k(k)` y `buscar_rival(id, tolerancia)`. No se han aprobado ni contabilizado todavía. El equipo selecciona operaciones con dos candidatos por la misma operación y escenarios donde cada uno gane.

El frontend actual consume endpoints documentados en `vercel.json` y `api/game.js`. No cambiar sus contratos sin actualizar rutas, clientes y pruebas asociadas.
