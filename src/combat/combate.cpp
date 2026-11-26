#include "combate.h"
#include <stdexcept>

Combate::Combate(Personaje& primero, Personaje& segundo)
    : jugadores_{&primero, &segundo} {
    if (&primero == &segundo || !primero.vive() || !segundo.vive()) {
        throw std::invalid_argument("se requieren dos personajes distintos y vivos");
    }
}

ResultadoTurno Combate::actuar(AccionCombate accion) {
    auto resultado = aplicarTurno(*jugadores_[actual_], *jugadores_[1 - actual_], accion);
    if (resultado.estado == EstadoTurno::ejecutado) {
        ++turnos_;
        if (!terminado()) {
            actual_ = 1 - actual_;
        }
    }
    return resultado;
}

const Personaje& Combate::actorActual() const {
    return *jugadores_[actual_];
}

const Personaje* Combate::ganador() const {
    // No atribuir ganador si ambos personajes fueron alterados externamente.
    if (jugadores_[0]->vive() == jugadores_[1]->vive()) return nullptr;
    return jugadores_[0]->vive() ? jugadores_[0] : jugadores_[1];
}

bool Combate::terminado() const {
    return !jugadores_[0]->vive() || !jugadores_[1]->vive();
}

unsigned long Combate::turnosEjecutados() const {
    return turnos_;
}
