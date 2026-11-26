#ifndef COMBAT_TURNO_H
#define COMBAT_TURNO_H

#include "personaje.h"

enum class AccionCombate { normal, especial, defender };
enum class EstadoTurno { ejecutado, combate_terminado, accion_invalida, mismo_personaje };

struct ResultadoTurno {
    EstadoTurno estado;
    int hp_objetivo_antes;
    int hp_objetivo_despues;
    int ki_actor_antes;
    int ki_actor_despues;
    bool especial_sin_ki;
};

// No controla alternancia: el llamador decide quién actúa.
ResultadoTurno aplicarTurno(Personaje& actor, Personaje& objetivo, AccionCombate accion);

#endif
