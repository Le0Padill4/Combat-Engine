#include "combate.h"
#include <cassert>
#include <stdexcept>

int main() {
    Personaje a("A", 100, 30, 20, 8);
    Personaje b("B", 100, 30, 20, 8);
    try { Combate invalido(a, a); assert(false); }
    catch (const std::invalid_argument&) {}
    Combate combate(a, b);
    assert(&combate.actorActual() == &a && !combate.terminado());
    assert(combate.ganador() == nullptr && combate.turnosEjecutados() == 0);
    assert(combate.actuar(static_cast<AccionCombate>(99)).estado == EstadoTurno::accion_invalida);
    assert(&combate.actorActual() == &a && combate.turnosEjecutados() == 0);
    auto resultado = combate.actuar(AccionCombate::especial);
    assert(resultado.especial_sin_ki && &combate.actorActual() == &b);
    assert(combate.turnosEjecutados() == 1 && b.getHp() == 100);
    combate.actuar(AccionCombate::defender);
    assert(&combate.actorActual() == &a && b.getDefiende());
    b.setHp(1);
    combate.actuar(AccionCombate::normal);
    // El daño mínimo de 1 se reduce a 0 si se defiende; con 20 de ataque es 8.
    assert(combate.terminado() && combate.ganador() == &a);
    assert(combate.turnosEjecutados() == 3);
    assert(combate.actuar(AccionCombate::normal).estado == EstadoTurno::combate_terminado);
    assert(combate.turnosEjecutados() == 3);
    Personaje muerto("Muerto", 1, 0, 1, 0);
    muerto.setHp(0);
    try { Combate invalido(a, muerto); assert(false); }
    catch (const std::invalid_argument&) {}
    a.setHp(0);
    assert(combate.terminado() && combate.ganador() == nullptr);
}
