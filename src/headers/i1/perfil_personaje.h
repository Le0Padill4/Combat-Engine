#ifndef COMBAT_I1_PERFIL_PERSONAJE_H
#define COMBAT_I1_PERFIL_PERSONAJE_H

#include "personaje.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <tuple>

namespace combat::i1 {
struct AtributosPersonaje {
    int hp_max = 0;
    int ki_max = 0;
    int ataque = 0;
    int defensa = 0;
};

struct HabilidadesPersonaje {
    std::string normal;
    std::string especial;
    int costo_ki_especial = 0;
};

// A profile stores the definition, not live HP/KI/turn/defense state.
struct PerfilPersonaje {
    std::string id;
    std::string alias;
    AtributosPersonaje atributos;
    HabilidadesPersonaje habilidades;
};

inline bool operator==(const PerfilPersonaje& a, const PerfilPersonaje& b) {
    return std::tie(a.id, a.alias, a.atributos.hp_max, a.atributos.ki_max,
                    a.atributos.ataque, a.atributos.defensa, a.habilidades.normal,
                    a.habilidades.especial, a.habilidades.costo_ki_especial) ==
           std::tie(b.id, b.alias, b.atributos.hp_max, b.atributos.ki_max,
                    b.atributos.ataque, b.atributos.defensa, b.habilidades.normal,
                    b.habilidades.especial, b.habilidades.costo_ki_especial);
}

inline bool operator!=(const PerfilPersonaje& a, const PerfilPersonaje& b) {
    return !(a == b);
}

inline bool texto_no_vacio(const std::string& texto) {
    return std::any_of(texto.begin(), texto.end(), [](unsigned char c) {
        return !std::isspace(c);
    });
}

inline bool perfil_valido(const PerfilPersonaje& p) {
    return texto_no_vacio(p.id) && texto_no_vacio(p.alias) &&
           texto_no_vacio(p.habilidades.normal) && texto_no_vacio(p.habilidades.especial) &&
           p.atributos.hp_max >= 1 && p.atributos.ki_max >= 0 &&
           p.atributos.ataque >= 1 && p.atributos.defensa >= 0 &&
           p.habilidades.costo_ki_especial >= 0;
}

inline PerfilPersonaje perfil_desde_personaje(const std::string& id, const Personaje& p) {
    return {id, p.getNombre(), {p.getHpMax(), p.getKiMax(), p.getAtaque(), p.getDefensa()},
            {p.getNataque(), p.getNataqueEspecial(), p.getKiEspecial()}};
}
}  // namespace combat::i1
#endif
