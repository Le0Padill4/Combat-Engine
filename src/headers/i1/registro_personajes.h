#ifndef COMBAT_I1_REGISTRO_PERSONAJES_H
#define COMBAT_I1_REGISTRO_PERSONAJES_H

#include "hash_table.h"
#include "perfil_personaje.h"
#include <algorithm>
#include <optional>
#include <vector>

namespace combat::i1 {
enum class ResultadoRegistro { registrado, id_duplicado, perfil_invalido };

class RegistroPersonajesHash {
    HashTable<std::string, PerfilPersonaje> tabla_;
public:
    ResultadoRegistro registrar_personaje(const PerfilPersonaje& p) {
        if (!perfil_valido(p)) return ResultadoRegistro::perfil_invalido;
        return tabla_.insertar(p.id, p) ? ResultadoRegistro::registrado
                                       : ResultadoRegistro::id_duplicado;
    }
    std::optional<PerfilPersonaje> buscar_personaje(const std::string& id) const {
        const auto* p = tabla_.buscar(id);
        return p ? std::optional<PerfilPersonaje>(*p) : std::nullopt;
    }
    std::size_t tamano() const { return tabla_.tamano(); }
    bool verificar_invariantes() const { return tabla_.verificar_invariantes(); }
    std::size_t bytes_estructura() const { return tabla_.bytes_estructura(); }
};

// Alternative candidate with exactly the same public operation semantics.
class RegistroPersonajesOrdenado {
    std::vector<PerfilPersonaje> perfiles_;
    auto posicion(const std::string& id) const {
        return std::lower_bound(perfiles_.begin(), perfiles_.end(), id,
            [](const PerfilPersonaje& p, const std::string& clave) { return p.id < clave; });
    }
public:
    ResultadoRegistro registrar_personaje(const PerfilPersonaje& p) {
        if (!perfil_valido(p)) return ResultadoRegistro::perfil_invalido;
        const auto it = posicion(p.id);
        if (it != perfiles_.end() && it->id == p.id) return ResultadoRegistro::id_duplicado;
        perfiles_.insert(it, p);
        return ResultadoRegistro::registrado;
    }
    std::optional<PerfilPersonaje> buscar_personaje(const std::string& id) const {
        const auto it = posicion(id);
        return it != perfiles_.end() && it->id == id
            ? std::optional<PerfilPersonaje>(*it) : std::nullopt;
    }
    std::size_t tamano() const { return perfiles_.size(); }
    bool verificar_invariantes() const {
        for (std::size_t i = 0; i < perfiles_.size(); ++i) {
            if (!perfil_valido(perfiles_[i])) return false;
            if (i && !(perfiles_[i - 1].id < perfiles_[i].id)) return false;
        }
        return true;
    }
    std::size_t bytes_estructura() const {
        return sizeof(*this) + perfiles_.capacity() * sizeof(PerfilPersonaje);
    }
};
}  // namespace combat::i1
#endif
