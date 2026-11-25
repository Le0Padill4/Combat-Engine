# Combat-Engine

Motor académico de un juego de combate por turnos en C++17, conservando el proyecto y su historia Git existentes. La interfaz visual actual se mantiene como experiencia secundaria y no define el alcance de CMP3002.

## Estado

La historia del repositorio anterior se conserva. El audit inicial encontró un `#include "clase.h"` inexistente y una ruta de archivo con `ç` que falla en MSVC. Un PR de corrección los resuelve y su CI pasó en Ubuntu, macOS y Windows; está pendiente de aprobación y merge. Existen pruebas Node para contratos visuales/API, no pruebas unitarias del motor C++ todavía.

## Compilar y ejecutar

```sh
cmake -S . -B build
cmake --build build
./build/juego
```

En Windows el ejecutable está en `build/juego.exe`. Comandos de pruebas académicas y benchmarks se publicarán cuando estén implementados; no hay resultados reportados todavía.

## Entrega CMP3002

El paquete incluirá informe, motor, referencia lenta, pruebas, código de benchmark, CSV/gráficos reproducibles y README. El ZIP final excluye ejecutables e historial Git. Revisa `docs/ESTADO_PROYECTO.md` y `docs/PLAN_MAESTRO.md`.
