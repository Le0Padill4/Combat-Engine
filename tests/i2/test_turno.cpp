#include "turno.h"
#include "leo1.h"
#include <cassert>

int main() {
    Personaje a("A", 100, 30, 20, 8);
    Personaje b("B", 100, 30, 20, 8);
    auto r = aplicarTurno(a, b, AccionCombate::normal);
    assert(r.estado == EstadoTurno::ejecutado && b.getHp() == 84 && a.getKi() == 15);
    r = aplicarTurno(b, a, AccionCombate::defender);
    assert(b.getDefiende() && b.getKi() == 10);
    r = aplicarTurno(a, b, AccionCombate::especial);
    assert(r.especial_sin_ki && b.getHp() == 84 && a.getKi() == 15);
    a.setKi(30);
    r = aplicarTurno(a, b, AccionCombate::especial);
    assert(!r.especial_sin_ki && b.getHp() == 68 && a.getKi() == 10);
    b.setKi(30);
    aplicarTurno(b, a, AccionCombate::normal);
    assert(!b.getDefiende() && b.getKi() == 30);
    int hp = a.getHp();
    assert(aplicarTurno(b, a, static_cast<AccionCombate>(99)).estado == EstadoTurno::accion_invalida);
    assert(a.getHp() == hp);
    assert(aplicarTurno(a, a, AccionCombate::normal).estado == EstadoTurno::mismo_personaje);
    b.setHp(0);
    assert(aplicarTurno(a, b, AccionCombate::normal).estado == EstadoTurno::combate_terminado);

    Leo1 personaje("Sintetico");
    Personaje objetivo("Objetivo", 100, 30, 20, 8);
    personaje.setKi(25);
    Personaje& polimorfico = personaje;
    r = aplicarTurno(polimorfico, objetivo, AccionCombate::especial);
    assert(r.estado == EstadoTurno::ejecutado && !r.especial_sin_ki);
    assert(objetivo.getHp() == 68 && personaje.getKi() == 0);

    Personaje debil("Debil", 10, 30, 1, 0);
    Personaje protegido("Protegido", 10, 30, 1, 100);
    protegido.setdefiende(true);
    aplicarTurno(debil, protegido, AccionCombate::normal);
    assert(protegido.getHp() == 10);  // 1 / 2 = 0 con división entera.
    return 0;
}
