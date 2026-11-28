#ifndef COMBAT_REGISTRO_COMBATE_H
#define COMBAT_REGISTRO_COMBATE_H

#include "combate.h"
#include "i1/registro_personajes.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

// La fabrica conserva el tipo dinamico y debe reconstruir exactamente el perfil.
// Cada combate es propietario de sus personajes y no modifica el registro.
class CombateRegistrado {
    std::unique_ptr<Personaje> primero_;
    std::unique_ptr<Personaje> segundo_;
    Combate combate_;

    static combat::i1::PerfilPersonaje perfil(
        const combat::i1::RegistroPersonajesHash& registro, const std::string& id) {
        auto resultado = registro.buscar_personaje(id);
        if (!resultado) throw std::invalid_argument("personaje no registrado");
        return *resultado;
    }
    using Fabrica = std::function<std::unique_ptr<Personaje>(const combat::i1::PerfilPersonaje&)>;
    static std::unique_ptr<Personaje> construir(const combat::i1::PerfilPersonaje& p,
                                               const Fabrica& fabrica) {
        auto personaje = fabrica(p);
        if (!personaje || combat::i1::perfil_desde_personaje(p.id, *personaje) != p ||
            personaje->getHp() != personaje->getHpMax() || personaje->getKi() != 0 ||
            personaje->getDefiende()) {
            throw std::invalid_argument("fabrica incompatible con perfil o estado inicial");
        }
        return personaje;
    }
    CombateRegistrado(std::unique_ptr<Personaje> a, std::unique_ptr<Personaje> b)
        : primero_(std::move(a)), segundo_(std::move(b)), combate_(*primero_, *segundo_) {}

public:
    CombateRegistrado(const CombateRegistrado&) = delete;
    CombateRegistrado& operator=(const CombateRegistrado&) = delete;
    static std::unique_ptr<CombateRegistrado> crear(
        const combat::i1::RegistroPersonajesHash& registro,
        const std::string& a, const std::string& b, const Fabrica& fabrica) {
        if (a == b) throw std::invalid_argument("identidades iguales");
        auto primero = perfil(registro, a);
        auto segundo = perfil(registro, b);
        auto personaje_a = construir(primero, fabrica);
        auto personaje_b = construir(segundo, fabrica);
        return std::unique_ptr<CombateRegistrado>(
            new CombateRegistrado(std::move(personaje_a), std::move(personaje_b)));
    }
    ResultadoTurno actuar(AccionCombate accion) { return combate_.actuar(accion); }
    const Personaje& primero() const { return *primero_; }
    const Personaje& segundo() const { return *segundo_; }
    const Personaje& actorActual() const { return combate_.actorActual(); }
    const Personaje* ganador() const { return combate_.ganador(); }
    bool terminado() const { return combate_.terminado(); }
    unsigned long turnosEjecutados() const { return combate_.turnosEjecutados(); }
};
#endif
