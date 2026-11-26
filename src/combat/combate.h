#ifndef COMBAT_COMBATE_H
#define COMBAT_COMBATE_H

#include "turno.h"

// Los personajes deben permanecer vivos en memoria durante toda la instancia.
// El combate no es propietario de ellos y no puede copiarse.
class Combate {
public:
    Combate(Personaje& primero, Personaje& segundo);
    Combate(const Combate&) = delete;
    Combate& operator=(const Combate&) = delete;

    ResultadoTurno actuar(AccionCombate accion);
    const Personaje& actorActual() const;
    const Personaje* ganador() const;
    bool terminado() const;
    unsigned long turnosEjecutados() const;

private:
    Personaje* jugadores_[2];
    unsigned int actual_ = 0;
    unsigned long turnos_ = 0;
};

#endif
