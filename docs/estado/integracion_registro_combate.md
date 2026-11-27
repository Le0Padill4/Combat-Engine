# Adaptador experimental entre registro y combate

En la rama de integración se propone `CombateRegistrado`: busca dos perfiles por
ID en el registro de I1, rechaza IDs iguales o ausentes y crea dos instancias
independientes de `Personaje` base, propiedad del combate. Cada combate empieza
con HP máximo y ki cero. Las acciones delegan al controlador de I2; el registro
permanece inalterado. Se exponen únicamente vistas constantes del estado.

Esta adaptación de perfiles genéricos **no reconstruye subclases ni sus ataques
virtuales específicos**. Es una propuesta de contrato para I1/I2/I3, no una
sustitución del juego existente ni integración de ranking o historial. Falta
acordar identidad, reglas de personajes derivados y un evento de fin idempotente
para actualizar estadísticas e índices compartidos. Depende de #6, #7, #8 y #9;
la rama de integración no debe fusionarse sin resolver y revisar esas bases.

Prueba ejecutada localmente:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -Isrc/headers -Isrc/combat tests/integracion/registro_combate.cpp src/combat/combate.cpp src/combat/turno.cpp src/personajes/personaje.cpp -o /tmp/combat-registro-integracion
/tmp/combat-registro-integracion
```

Resultado: código de salida 0; cubre perfil ausente, identidad repetida, acción
inválida, combate terminado, aislamiento entre partidas y registro sin cambios.
No es el diferencial del motor completo ni una prueba de personajes derivados.
