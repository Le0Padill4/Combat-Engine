# Integración propuesta entre registro y combate

`CombateRegistrado` obtiene perfiles de I1 y entrega su construcción a una fábrica
inyectada. El combate posee las instancias polimórficas; cada instancia debe
reproducir el perfil registrado y comenzar con HP máximo, ki cero y sin defensa.
Se rechazan identificadores iguales o ausentes, una fábrica que devuelva nulo o
un objeto cuyo perfil o estado inicial no corresponda. El registro no se modifica.

Prueba ejecutada el 27 nov 2026:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -Isrc/headers -Isrc/combat tests/integracion/registro_combate.cpp src/combat/combate.cpp src/combat/turno.cpp src/personajes/personaje.cpp src/personajes/leo1.cpp -o /tmp/combat-registro-integracion
/tmp/combat-registro-integracion
```

Resultado: salida 0. Cubre identidades inválidas, estado aislado, vida posterior
al registro, rechazo de fábrica incompatible y un `Leo1` dinámico cuyo ataque
especial consume ki y aplica daño a través del controlador.

**Límites:** `PerfilPersonaje` aún no identifica el tipo concreto; la fábrica debe
recibir una asociación externa acordada con I1. Comprobar igualdad de atributos
no demuestra que la fábrica haya elegido la subclase correcta. Falta formalizar
el evento de fin y su procesamiento idempotente con ranking, historial y
estadísticas de I3. No hay prueba diferencial del motor completo ni integración
con el flujo principal o CI de proyecto. Esta rama combina PR aún sin fusionar:
no constituye aprobación de contrato ni una integración lista para main.
