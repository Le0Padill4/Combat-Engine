# Combat-Engine

Motor académico de un juego de combate por turnos en C++17, conservando el proyecto y su historia Git existentes. La interfaz visual actual se mantiene como experiencia secundaria y no define el alcance de CMP3002.

## Estado

La rama principal conserva la historia del repositorio anterior. El audit inicial encontró un `#include "clase.h"` inexistente en `src/main.cpp`; por tanto, el build C++ necesita una corrección y verificación antes de declarar que compila. Hay pruebas Node para contratos visuales/API, pero no pruebas del motor C++ ni workflow de CI en la revisión inicial.

## Compilar y ejecutar

```sh
cmake -S . -B build
cmake --build build
./build/juego
```

En Windows el ejecutable está en `build/juego.exe`. Comandos de pruebas académicas y benchmarks se publicarán cuando estén implementados; no hay resultados reportados todavía.

## Entrega CMP3002

El paquete incluirá informe, motor, referencia lenta, pruebas, código de benchmark, CSV/gráficos reproducibles y README. El ZIP final excluye ejecutables e historial Git. Revisa `docs/ESTADO_PROYECTO.md` y `docs/PLAN_MAESTRO.md`.
