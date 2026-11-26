#include "turno.h"

ResultadoTurno aplicarTurno(Personaje& actor, Personaje& objetivo, AccionCombate accion) {
    ResultadoTurno resultado{EstadoTurno::ejecutado, objetivo.getHp(), objetivo.getHp(),
                             actor.getKi(), actor.getKi(), false};
    if (&actor == &objetivo) {
        resultado.estado = EstadoTurno::mismo_personaje;
        return resultado;
    }
    if (!actor.vive() || !objetivo.vive()) {
        resultado.estado = EstadoTurno::combate_terminado;
        return resultado;
    }
    if (accion != AccionCombate::normal && accion != AccionCombate::especial &&
        accion != AccionCombate::defender) {
        resultado.estado = EstadoTurno::accion_invalida;
        return resultado;
    }
    // Replica ejecutarTurno en main: incluso un especial sin ki consume el turno.
    actor.setdefiende(false);
    switch (accion) {
    case AccionCombate::normal:
        actor.atacarNormal(objetivo);
        actor.setKi(actor.getKi() + 15);
        break;
    case AccionCombate::especial:
        resultado.especial_sin_ki = actor.getKi() < actor.getKiEspecial();
        actor.atacarEspecial(objetivo);
        break;
    case AccionCombate::defender:
        actor.setdefiende(true);
        actor.setKi(actor.getKi() + 10);
        break;
    }
    resultado.hp_objetivo_despues = objetivo.getHp();
    resultado.ki_actor_despues = actor.getKi();
    return resultado;
}
