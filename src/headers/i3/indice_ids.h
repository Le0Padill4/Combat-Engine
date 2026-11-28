#ifndef COMBAT_I3_INDICE_IDS_H
#define COMBAT_I3_INDICE_IDS_H

// Indice id -> slot denso que usa el motor de I3. Interfaz minima:
//   const std::size_t* buscar(const std::string& id) const   nullptr si no existe
//   bool insertar(const std::string& id, std::size_t slot)   false si ya existia
//   std::size_t tamano() const
//
// IndiceStd es el adaptador temporal sobre std::unordered_map. Cuando la
// HashTable propia de I1 (PR #9, src/headers/i1/hash_table.h) este en main,
// IndiceHashI1 queda disponible automaticamente (no se duplica su estructura).

#include <cstddef>
#include <string>
#include <unordered_map>

namespace combat::i3 {

class IndiceStd {
    std::unordered_map<std::string, std::size_t> m_;

public:
    const std::size_t* buscar(const std::string& id) const {
        const auto it = m_.find(id);
        return it == m_.end() ? nullptr : &it->second;
    }
    bool insertar(const std::string& id, std::size_t slot) { return m_.emplace(id, slot).second; }
    std::size_t tamano() const { return m_.size(); }
};

}  // namespace combat::i3

#if defined(__has_include)
#if __has_include("i1/hash_table.h")
#include "i1/hash_table.h"
#define COMBAT_I3_HAY_HASH_I1 1

namespace combat::i3 {
// Adaptador sobre la HashTable de I1 (solo insertar y buscar; no se modifica).
class IndiceHashI1 {
    combat::i1::HashTable<std::string, std::size_t> t_;

public:
    const std::size_t* buscar(const std::string& id) const { return t_.buscar(id); }
    bool insertar(const std::string& id, std::size_t slot) { return t_.insertar(id, slot); }
    std::size_t tamano() const { return t_.tamano(); }
    bool verificar_invariantes() const { return t_.verificar_invariantes(); }
};
}  // namespace combat::i3
#endif
#endif

#endif
