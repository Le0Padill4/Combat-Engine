# Estado — Combat-Engine

Revisión inicial: 24 nov 2026. Repo renombrado desde `Proyecto_Final`; público, rama remota `main`, ocho commits preservados. Respaldo espejo previo: `backups/Combat-Engine-pre-rename-20261124.git`; clon local sin cambios antes del renombrado y remoto `origin` actualizado a `Le0Padill4/Combat-Engine`. El dominio actual responde HTTP 200 y los PRs generaron previews Vercel.

Hallazgos iniciales: `main.cpp` incluía `clase.h`, ausente del árbol; nombres de archivo con `ç` fallaban en MSVC. PR #6 elimina la inclusión obsoleta, renombra archivos a ASCII y fija UTF-8. CI pasó en Ubuntu, macOS y Windows tras la corrección. En esta máquina no está instalado CMake; la compilación local no se ejecutó.

Las 7 pruebas Node existentes pasaron localmente. No hay pruebas unitarias C++ todavía. El archivo `juego.exe` sigue rastreado. No se removió funcionalidad. PR #1 contiene esta documentación; PR #6 contiene baseline/CI. Ambos esperan revisión de un integrante.

Motor académico, invariantes, referencia lenta, comparación aleatoria, benchmarks, informe y ZIP: pendientes. El usuario autorizó el trío; la guía CMP3002 indica parejas, por lo que la aceptación del curso aún debe confirmarse.
