# Estado — Combat-Engine

Revisión inicial: 24 nov 2026. Repo renombrado desde `Proyecto_Final`; público, rama remota `main`, ocho commits preservados. Respaldo espejo previo: `backups/Combat-Engine-pre-rename-20261124.git`; clon local sin cambios antes del renombrado y remoto `origin` actualizado a `Le0Padill4/Combat-Engine`. Dominio Vercel devolvió HTTP 200 tras renombrar; todavía no se ha verificado el repo conectado en Vercel.

Hallazgos: `main.cpp` incluye `clase.h`, ausente del árbol; no se ejecutó compilación. Existe `juego.exe` rastreado. Pruebas existentes: `tests/battle-contract.test.js`, `tests/vercel-api.test.js`; no se ejecutaron en esta fase. No se encontró workflow CI ni pruebas del C++ en el audit inicial. No se removió funcionalidad.

Trabajo del motor CMP3002, invariantes, referencia lenta, pruebas diferenciales, benchmarks, informe y ZIP: pendiente. Equipo autorizado por usuario como trío; aprobación del curso no constatada.
