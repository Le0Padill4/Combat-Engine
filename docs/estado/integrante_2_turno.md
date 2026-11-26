# Transición de turno — Integrante 2

`src/combat/turno` es una propuesta aislada para ejecutar una acción sin pedir
entrada por consola. Usa métodos virtuales de `Personaje` para conservar los
ataques específicos. La alternancia, identidad estable y el resultado para
historial quedan fuera hasta acordar el contrato. No se ha sustituido
`ejecutarTurno` de `src/main.cpp` ni se han modificado la interfaz o el build.

Reglas observadas en `src/main.cpp` y `src/personajes/personaje.cpp`:
normal añade 15 de ki, defender añade 10, ambos limitados por `setKi`; al inicio
de la acción termina la defensa del actor; un especial sin ki no daña ni gasta
ki, pero la consola avanza el turno. La defensa reduce a la mitad el daño con
división entera; puede resultar en cero. Esta implementación delega el cálculo
a los métodos existentes. Rechazar personajes idénticos, acciones inválidas y
combate terminado es una condición nueva propuesta para la API; necesita revisión.

Verificación local (macOS, compilador C++ disponible):

```
c++ -std=c++17 -Wall -Wextra -Werror -Isrc/headers -Isrc/combat tests/i2/test_turno.cpp src/combat/turno.cpp src/personajes/personaje.cpp src/personajes/leo1.cpp -o /tmp/combat-i2-test
/tmp/combat-i2-test
```

Ambos comandos terminaron correctamente. Esta prueba no verifica la integración
con el juego completo ni CMake/CI, el contrato de I1/I3, prioridad, efectos,
selección de rival, referencia lenta o benchmark. `main` mantiene un include
faltante; su reparación está en el PR #6. Solicitar revisión antes de cambiar el
comportamiento o conectarlo a la consola.

## Continuidad del 25 de noviembre

Se añadió un CMake aislado para la prueba de turno y CI en Ubuntu, macOS y
Windows; los tres trabajos pasaron en el commit 67b8703. También pasó la
compilación C++17 directa en macOS. CMake no está disponible localmente. Se
solicitó revisión a I1 e I3; el PR #7 sigue en borrador. Los checks obligatorios
del build principal dependen del PR #6, aún sin integrar. El PR #8 conserva su
base en #7 hasta que este se integre y se pueda actualizar con seguridad.
