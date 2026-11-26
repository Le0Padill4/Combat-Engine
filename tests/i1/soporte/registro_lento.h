#ifndef COMBAT_I1_REGISTRO_LENTO_H
#define COMBAT_I1_REGISTRO_LENTO_H

#include "i1/perfil_personaje.h"
#include "i1/registro_personajes.h"
#include <optional>
#include <vector>

namespace combat::i1::soporte {
// Deliberately linear and independent of both optimized storage algorithms.
class RegistroPersonajesLento {
    std::vector<PerfilPersonaje> perfiles_;
public:
    ResultadoRegistro registrar_personaje(const PerfilPersonaje& p) {
        if (!perfil_valido(p)) return ResultadoRegistro::perfil_invalido;
        for (const auto& existente : perfiles_)
            if (existente.id == p.id) return ResultadoRegistro::id_duplicado;
        perfiles_.push_back(p);
        return ResultadoRegistro::registrado;
    }
    std::optional<PerfilPersonaje> buscar_personaje(const std::string& id) const {
        for (const auto& p : perfiles_)
            if (p.id == id) return p;
        return std::nullopt;
    }
    std::size_t tamano() const { return perfiles_.size(); }
    bool verificar_invariantes() const {
        for (std::size_t i = 0; i < perfiles_.size(); ++i) {
            if (!perfil_valido(perfiles_[i])) return false;
            for (std::size_t j = 0; j < i; ++j)
                if (perfiles_[i].id == perfiles_[j].id) return false;
        }
        return true;
    }
};
}  // namespace combat::i1::soporte
#endif
