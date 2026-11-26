#include "turno.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <sstream>

// Oráculo simple de una acción del Personaje base, no del motor completo.
int main() {
    std::mt19937 random(20261125);
    std::uniform_int_distribution<int> vida(0, 50), ki(0, 30), estadistica(1, 30),
        defensa(0, 30), accion(0, 3), booleano(0, 1);
    std::ostringstream silencio;
    auto* original = std::cout.rdbuf(silencio.rdbuf());
    for (int iteracion = 0; iteracion < 10000; ++iteracion) {
        const int ataque = estadistica(random), defensaObjetivo = defensa(random);
        const int hpActor = vida(random), hpObjetivo = vida(random), kiActor = ki(random);
        const bool protegeActor = booleano(random), protegeObjetivo = booleano(random);
        const int opcion = accion(random);
        Personaje actor("sintetico-a", 50, 30, ataque, 0);
        Personaje objetivo("sintetico-b", 50, 30, 1, defensaObjetivo);
        actor.setHp(hpActor); actor.setKi(kiActor); actor.setdefiende(protegeActor);
        objetivo.setHp(hpObjetivo); objetivo.setdefiende(protegeObjetivo);
        const auto resultado = aplicarTurno(actor, objetivo, static_cast<AccionCombate>(opcion));

        int hpEsperado = hpObjetivo, kiEsperado = kiActor;
        bool defensaEsperada = protegeActor, sinKi = false;
        EstadoTurno estado = EstadoTurno::ejecutado;
        if (hpActor == 0 || hpObjetivo == 0) {
            estado = EstadoTurno::combate_terminado;
        } else if (opcion == 3) {
            estado = EstadoTurno::accion_invalida;
        } else {
            defensaEsperada = opcion == 2;
            if (opcion == 0) {
                int dano = std::max(1, ataque - defensaObjetivo / 2);
                if (protegeObjetivo) dano /= 2;
                hpEsperado = std::max(0, hpObjetivo - dano);
                kiEsperado = std::min(30, kiActor + 15);
            } else if (opcion == 1) {
                if (kiActor < 20) sinKi = true;
                else {
                    int dano = std::max(1, ataque * 2 - defensaObjetivo);
                    if (protegeObjetivo) dano /= 2;
                    hpEsperado = std::max(0, hpObjetivo - dano);
                    kiEsperado -= 20;
                }
            } else {
                kiEsperado = std::min(30, kiActor + 10);
            }
        }
        assert(resultado.estado == estado && resultado.especial_sin_ki == sinKi);
        assert(actor.getHp() == hpActor && actor.getKi() == kiEsperado);
        assert(actor.getDefiende() == defensaEsperada);
        assert(objetivo.getHp() == hpEsperado && objetivo.getDefiende() == protegeObjetivo);
        assert(resultado.hp_objetivo_antes == hpObjetivo && resultado.hp_objetivo_despues == hpEsperado);
        assert(resultado.ki_actor_antes == kiActor && resultado.ki_actor_despues == kiEsperado);
    }
    std::cout.rdbuf(original);
    std::cout << "10000 casos sinteticos de turno base coinciden con el oraculo\n";
}
