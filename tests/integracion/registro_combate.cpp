#include "registro_combate.h"
#include "leo1.h"
#include <cassert>
#include <stdexcept>

using namespace combat::i1;
int main() {
    auto fabrica = [](const PerfilPersonaje& p) -> std::unique_ptr<Personaje> {
        auto x = std::make_unique<Personaje>(p.alias, p.atributos.hp_max, p.atributos.ki_max,
                                             p.atributos.ataque, p.atributos.defensa);
        x->setNataque(p.habilidades.normal);
        x->setNataqueEspecial(p.habilidades.especial);
        x->setKiEspecial(p.habilidades.costo_ki_especial);
        return x;
    };
    RegistroPersonajesHash registro;
    PerfilPersonaje a{"a", "A", {10, 30, 20, 0}, {"golpe", "especial", 15}};
    PerfilPersonaje b{"b", "B", {10, 30, 3, 0}, {"patada", "onda", 20}};
    assert(registro.registrar_personaje(a) == ResultadoRegistro::registrado);
    assert(registro.registrar_personaje(b) == ResultadoRegistro::registrado);
    for (auto ids : {std::pair<std::string, std::string>{"a", "a"}, {"a", "ausente"}}) {
        bool fallo = false;
        try { (void)CombateRegistrado::crear(registro, ids.first, ids.second, fabrica); }
        catch (const std::invalid_argument&) { fallo = true; }
        assert(fallo);
    }
    auto uno = CombateRegistrado::crear(registro, "a", "b", fabrica);
    auto dos = CombateRegistrado::crear(registro, "a", "b", fabrica);
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
        independiente = CombateRegistrado::crear(temporal, "a", "b", fabrica);
    }
    assert(independiente->actuar(AccionCombate::normal).estado == EstadoTurno::ejecutado);
    assert(independiente->segundo().getHp() == 6);
    assert(&independiente->actorActual() == &independiente->segundo());
    assert(independiente->actuar(AccionCombate::defender).estado == EstadoTurno::ejecutado);
    assert(&independiente->actorActual() == &independiente->primero());
    assert(independiente->turnosEjecutados() == 2 && !independiente->terminado());
    // La fábrica devuelve el tipo derivado, que sobrevive hasta el combate.
    Leo1 leo("Leo");
    RegistroPersonajesHash derivados;
    assert(derivados.registrar_personaje(perfil_desde_personaje("leo", leo)) == ResultadoRegistro::registrado);
    b.atributos.hp_max = 100;
    assert(derivados.registrar_personaje(b) == ResultadoRegistro::registrado);
    auto polimorfica = [&](const PerfilPersonaje& p) -> std::unique_ptr<Personaje> {
        if (p.id == "leo") return std::make_unique<Leo1>(p.alias);
        return fabrica(p);
    };
    auto batalla = CombateRegistrado::crear(derivados, "leo", "b", polimorfica);
    assert(dynamic_cast<const Leo1*>(&batalla->primero()) != nullptr);
    assert(batalla->actuar(AccionCombate::normal).estado == EstadoTurno::ejecutado);
    assert(batalla->actuar(AccionCombate::defender).estado == EstadoTurno::ejecutado);
    assert(batalla->actuar(AccionCombate::normal).estado == EstadoTurno::ejecutado);
    assert(batalla->actuar(AccionCombate::defender).estado == EstadoTurno::ejecutado);
    assert(batalla->actuar(AccionCombate::especial).estado == EstadoTurno::ejecutado);
    assert(batalla->primero().getKi() == 5 && batalla->segundo().getHp() == 50);
    auto incompatible = [](const PerfilPersonaje&) -> std::unique_ptr<Personaje> {
        return std::make_unique<Personaje>("distinto", 1, 0, 1, 0);
    };
    bool rechazo = false;
    try { (void)CombateRegistrado::crear(derivados, "leo", "b", incompatible); }
    catch (const std::invalid_argument&) { rechazo = true; }
    assert(rechazo);
}
