#include "registro_combate.h"
#include <cassert>
#include <stdexcept>

using namespace combat::i1;
int main() {
    RegistroPersonajesHash registro;
    PerfilPersonaje a{"a", "A", {10, 30, 20, 0}, {"golpe", "especial", 15}};
    PerfilPersonaje b{"b", "B", {10, 30, 3, 0}, {"patada", "onda", 20}};
    assert(registro.registrar_personaje(a) == ResultadoRegistro::registrado);
    assert(registro.registrar_personaje(b) == ResultadoRegistro::registrado);
    for (auto ids : {std::pair<std::string, std::string>{"a", "a"}, {"a", "ausente"}}) {
        bool fallo = false;
        try { (void)CombateRegistrado::crear(registro, ids.first, ids.second); }
        catch (const std::invalid_argument&) { fallo = true; }
        assert(fallo);
    }
    auto uno = CombateRegistrado::crear(registro, "a", "b");
    auto dos = CombateRegistrado::crear(registro, "a", "b");
    assert(uno->primero().getNataque() == "golpe");
    assert(uno->primero().getKiEspecial() == 15);
    assert(&uno->actorActual() == &uno->primero());
    auto invalida = uno->actuar(static_cast<AccionCombate>(99));
    assert(invalida.estado == EstadoTurno::accion_invalida && uno->turnosEjecutados() == 0);
    auto resultado = uno->actuar(AccionCombate::normal);
    assert(resultado.estado == EstadoTurno::ejecutado && uno->terminado());
    assert(uno->ganador() == &uno->primero());
    assert(dos->segundo().getHp() == 10 && !dos->terminado());
    assert(registro.buscar_personaje("a") == a && registro.buscar_personaje("b") == b);
    assert(registro.verificar_invariantes());

    // Los perfiles son snapshots: el combate sobrevive al registro original.
    std::unique_ptr<CombateRegistrado> independiente;
    {
        RegistroPersonajesHash temporal;
        a.atributos.ataque = 4;
        assert(temporal.registrar_personaje(a) == ResultadoRegistro::registrado);
        assert(temporal.registrar_personaje(b) == ResultadoRegistro::registrado);
        independiente = CombateRegistrado::crear(temporal, "a", "b");
    }
    assert(independiente->actuar(AccionCombate::normal).estado == EstadoTurno::ejecutado);
    assert(independiente->segundo().getHp() == 6);
    assert(&independiente->actorActual() == &independiente->segundo());
    assert(independiente->actuar(AccionCombate::defender).estado == EstadoTurno::ejecutado);
    assert(&independiente->actorActual() == &independiente->primero());
    assert(independiente->turnosEjecutados() == 2 && !independiente->terminado());
}
