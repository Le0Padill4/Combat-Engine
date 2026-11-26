# Secuencia de combate — propuesta I2

`Combate` alterna dos referencias a personajes distintos y vivos, con el primero
como actor inicial. El llamador conserva la propiedad y debe mantener ambos
objetos con vida durante toda la instancia. La clase no copia personajes, no
inicia ni reinicia el combate, y usa `aplicarTurno` del PR #7. Una acción inválida
no avanza; un especial sin ki sí. La operación informa ganador cuando solo un
personaje vive; si ambos se modifican externamente hasta quedar sin vida, no
atribuye un ganador. `actorActual()` tras finalizar identifica al último actor,
no garantiza un turno disponible.

Es una propuesta de integración: no sustituye la consola y las reglas de
identidad, propiedad de personajes, mutaciones externas y eventos para historial
requieren revisión con I1 e I3. No agrega prioridad, efectos temporales ni
selección de rival. Si se permite modificar personajes desde fuera, el llamador
es responsable de sincronizar el estado; la clase no es segura para concurrencia.

Prueba local C++17:

```
c++ -std=c++17 -Wall -Wextra -Werror -Isrc/headers -Isrc/combat tests/i2/test_combate.cpp src/combat/combate.cpp src/combat/turno.cpp src/personajes/personaje.cpp -o /tmp/combat-i2-secuencia
/tmp/combat-i2-secuencia
```

Además, `tests/i2/test_turno_diferencial.cpp` ejecuta 10.000 acciones
sintéticas independientes de `Personaje` base con semilla fija y compara los
cambios con un oráculo aritmético. Comando ejecutado:

```
c++ -std=c++17 -Wall -Wextra -Werror -Isrc/headers -Isrc/combat tests/i2/test_turno_diferencial.cpp src/combat/turno.cpp src/personajes/personaje.cpp -o /tmp/combat-i2-diferencial
/tmp/combat-i2-diferencial
```

Resultado: 10.000 coincidencias. No prueba personajes derivados, secuencias de
operaciones del motor completo, ranking ni rendimiento; no reemplaza la prueba
exigida por CMP3002 para el motor completo.
