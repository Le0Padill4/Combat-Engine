#ifndef COMBAT_REGISTRO_COMBATE_H
#define COMBAT_REGISTRO_COMBATE_H

#include "combate.h"
#include "i1/registro_personajes.h"
#include <memory>
#include <stdexcept>
#include <string>

// Adaptador experimental para perfiles genéricos; no reconstruye subclases.
// Cada combate posee copias de estado y no modifica el registro de perfiles.
class CombateRegistrado {
    Personaje primero_;
    Personaje segundo_;
    Combate combate_;

    static combat::i1::PerfilPersonaje perfil(
        const combat::i1::RegistroPersonajesHash& registro, const std::string& id) {
        auto resultado = registro.buscar_personaje(id);
        if (!resultado) throw std::invalid_argument("personaje no registrado");
        return *resultado;
    }
    static void configurar(Personaje& personaje, const combat::i1::PerfilPersonaje& p) {
        personaje.setNataque(p.habilidades.normal);
        personaje.setNataqueEspecial(p.habilidades.especial);
        personaje.setKiEspecial(p.habilidades.costo_ki_especial);
    }
    CombateRegistrado(const combat::i1::PerfilPersonaje& a,
                      const combat::i1::PerfilPersonaje& b)
        : primero_(a.alias, a.atributos.hp_max, a.atributos.ki_max,
                   a.atributos.ataque, a.atributos.defensa),
          segundo_(b.alias, b.atributos.hp_max, b.atributos.ki_max,
                   b.atributos.ataque, b.atributos.defensa),
          combate_(primero_, segundo_) {
        configurar(primero_, a);
        configurar(segundo_, b);
    }
public:
    CombateRegistrado(const CombateRegistrado&) = delete;
    CombateRegistrado& operator=(const CombateRegistrado&) = delete;
    static std::unique_ptr<CombateRegistrado> crear(
        const combat::i1::RegistroPersonajesHash& registro,
        const std::string& a, const std::string& b) {
        if (a == b) throw std::invalid_argument("identidades iguales");
        auto primero = perfil(registro, a);
        auto segundo = perfil(registro, b);
        return std::unique_ptr<CombateRegistrado>(new CombateRegistrado(primero, segundo));
    }
    ResultadoTurno actuar(AccionCombate accion) { return combate_.actuar(accion); }
    const Personaje& primero() const { return primero_; }
    const Personaje& segundo() const { return segundo_; }
    const Personaje& actorActual() const { return combate_.actorActual(); }
    const Personaje* ganador() const { return combate_.ganador(); }
    bool terminado() const { return combate_.terminado(); }
    unsigned long turnosEjecutados() const { return combate_.turnosEjecutados(); }
};
#endif
