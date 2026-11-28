#ifndef COMBAT_I3_RANKING_H
#define COMBAT_I3_RANKING_H

// Tres candidatos con EXACTAMENTE la misma interfaz y semantica para las
// operaciones top_k y posicion del ranking:
//
//   RankingAVL          AVL propio aumentado (avl_ranking.h).
//   RankingHeapSelect   puntajes sin ordenar; top_k selecciona con un heap
//                       binario construido en la consulta (std::make_heap).
//   RankingArreglo      vector de slots ordenado por ranking + busqueda binaria.
//
// Un "slot" es el indice denso 0..n-1 asignado por el motor al registrar un
// luchador. Interfaz comun:
//   void   agregar(slot, id)             slot == tamano(); puntos iniciales 0
//   void   cargar(ids, puntos)           solo si esta vacio (carga masiva)
//   void   fijar_puntos(slot, nuevos)
//   Puntos puntos(slot) const
//   std::vector<EntradaRanking> top_k(k) const   posicion 1-based; k > n devuelve n
//   std::optional<size_t> posicion(slot) const   1-based
//   size_t tamano() const; bool verificar_invariantes() const;
//   size_t bytes_estructura() const

#include "avl_ranking.h"
#include "tipos.h"

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace combat::i3 {

namespace detalle {
// Almacen comun por slot que usan los tres candidatos.
struct Almacen {
    std::vector<std::string> ids;
    std::vector<Puntos> puntos;

    void agregar(std::size_t slot, const std::string& id) {
        if (slot != ids.size()) throw std::invalid_argument("Ranking: slot no consecutivo");
        ids.push_back(id);
        puntos.push_back(0);
    }
    void cargar(const std::vector<std::string>& i, const std::vector<Puntos>& p) {
        if (!ids.empty()) throw std::invalid_argument("Ranking: cargar requiere ranking vacio");
        if (i.size() != p.size()) throw std::invalid_argument("Ranking: tamanos distintos");
        ids = i;
        puntos = p;
    }
    bool antes(std::size_t a, std::size_t b) const {
        return antes_en_ranking(puntos[a], ids[a], puntos[b], ids[b]);
    }
    std::size_t bytes() const {
        std::size_t b = ids.capacity() * sizeof(std::string) + puntos.capacity() * sizeof(Puntos);
        return b;
    }
};
}  // namespace detalle

// ---------------------------------------------------------------- AVL
class RankingAVL {
    detalle::Almacen a_;
    ArbolAVL arbol_;

public:
    std::size_t tamano() const { return a_.ids.size(); }
    Puntos puntos(std::size_t slot) const { return a_.puntos[slot]; }

    void agregar(std::size_t slot, const std::string& id) {
        a_.agregar(slot, id);
        arbol_.insertar(0, id);
    }
    void cargar(const std::vector<std::string>& ids_in, const std::vector<Puntos>& puntos_in) {
        a_.cargar(ids_in, puntos_in);
        for (std::size_t i = 0; i < ids_in.size(); ++i) arbol_.insertar(puntos_in[i], ids_in[i]);
    }
    void fijar_puntos(std::size_t slot, Puntos nuevos) {
        if (a_.puntos[slot] == nuevos) return;
        arbol_.eliminar(a_.puntos[slot], a_.ids[slot]);
        arbol_.insertar(nuevos, a_.ids[slot]);
        a_.puntos[slot] = nuevos;
    }
    std::vector<EntradaRanking> top_k(std::size_t k) const {
        std::vector<EntradaRanking> out;
        out.reserve(std::min(k, tamano()));
        arbol_.primeros(k, [&out](Puntos p, const std::string& id) {
            out.push_back({out.size() + 1, id, p});
        });
        return out;
    }
    std::optional<std::size_t> posicion(std::size_t slot) const {
        if (slot >= tamano()) return std::nullopt;
        const auto cuantos = arbol_.posicion(a_.puntos[slot], a_.ids[slot]);
        if (!cuantos) return std::nullopt;
        return *cuantos + 1;
    }
    bool verificar_invariantes() const {
        if (!arbol_.verificar_invariantes() || arbol_.tamano() != tamano()) return false;
        for (std::size_t s = 0; s < tamano(); ++s)
            if (!arbol_.posicion(a_.puntos[s], a_.ids[s])) return false;
        return true;
    }
    std::size_t bytes_estructura() const {
        return sizeof(*this) + a_.bytes() + arbol_.bytes_estructura();
    }
};

// ---------------------------------------------------------- Heap en consulta
class RankingHeapSelect {
    detalle::Almacen a_;

public:
    std::size_t tamano() const { return a_.ids.size(); }
    Puntos puntos(std::size_t slot) const { return a_.puntos[slot]; }

    void agregar(std::size_t slot, const std::string& id) { a_.agregar(slot, id); }
    void cargar(const std::vector<std::string>& ids_in, const std::vector<Puntos>& puntos_in) {
        a_.cargar(ids_in, puntos_in);
    }
    void fijar_puntos(std::size_t slot, Puntos nuevos) { a_.puntos[slot] = nuevos; }

    // O(n + k log n): heapify de los slots y k extracciones.
    std::vector<EntradaRanking> top_k(std::size_t k) const {
        const std::size_t n = tamano();
        const std::size_t cuantos = std::min(k, n);
        std::vector<EntradaRanking> out;
        if (cuantos == 0) return out;
        std::vector<std::size_t> heap(n);
        std::iota(heap.begin(), heap.end(), std::size_t{0});
        // make_heap deja en el frente el "mayor" segun cmp; "mayor" = mejor rankeado.
        const auto cmp = [this](std::size_t x, std::size_t y) { return a_.antes(y, x); };
        std::make_heap(heap.begin(), heap.end(), cmp);
        out.reserve(cuantos);
        auto fin = heap.end();
        for (std::size_t i = 0; i < cuantos; ++i) {
            std::pop_heap(heap.begin(), fin, cmp);
            --fin;
            out.push_back({i + 1, a_.ids[*fin], a_.puntos[*fin]});
        }
        return out;
    }
    // O(n): cuenta los luchadores que van antes.
    std::optional<std::size_t> posicion(std::size_t slot) const {
        if (slot >= tamano()) return std::nullopt;
        std::size_t antes = 0;
        for (std::size_t s = 0; s < tamano(); ++s)
            if (a_.antes(s, slot)) ++antes;
        return antes + 1;
    }
    bool verificar_invariantes() const { return a_.ids.size() == a_.puntos.size(); }
    std::size_t bytes_estructura() const { return sizeof(*this) + a_.bytes(); }
};

// ------------------------------------------------------------ Arreglo ordenado
// Invariante: orden_ es una permutacion de 0..n-1 ordenada estrictamente por
// antes_en_ranking segun los puntos actuales.
class RankingArreglo {
    detalle::Almacen a_;
    std::vector<std::size_t> orden_;

    // Primer indice cuyo elemento NO va antes que `slot` (slot puede no estar en orden_).
    std::size_t limite_inferior(std::size_t slot, std::size_t desde, std::size_t hasta) const {
        const auto it = std::lower_bound(
            orden_.begin() + static_cast<std::ptrdiff_t>(desde),
            orden_.begin() + static_cast<std::ptrdiff_t>(hasta), slot,
            [this](std::size_t x, std::size_t s) { return a_.antes(x, s); });
        return static_cast<std::size_t>(it - orden_.begin());
    }

public:
    std::size_t tamano() const { return a_.ids.size(); }
    Puntos puntos(std::size_t slot) const { return a_.puntos[slot]; }

    // O(n): inserta en su posicion ordenada (el registro de luchadores es raro).
    void agregar(std::size_t slot, const std::string& id) {
        a_.agregar(slot, id);
        const std::size_t idx = limite_inferior(slot, 0, orden_.size());
        orden_.insert(orden_.begin() + static_cast<std::ptrdiff_t>(idx), slot);
    }
    void cargar(const std::vector<std::string>& ids_in, const std::vector<Puntos>& puntos_in) {
        a_.cargar(ids_in, puntos_in);
        orden_.resize(ids_in.size());
        std::iota(orden_.begin(), orden_.end(), std::size_t{0});
        std::sort(orden_.begin(), orden_.end(),
                  [this](std::size_t x, std::size_t y) { return a_.antes(x, y); });
    }
    // O(log n) para localizar + O(|distancia|) para mover el slot con rotate.
    void fijar_puntos(std::size_t slot, Puntos nuevos) {
        if (a_.puntos[slot] == nuevos) return;
        const std::size_t p = limite_inferior(slot, 0, orden_.size());  // con puntos viejos
        a_.puntos[slot] = nuevos;
        const std::size_t n = orden_.size();
        const auto b = orden_.begin();
        if (p > 0 && a_.antes(slot, orden_[p - 1])) {
            const std::size_t q = limite_inferior(slot, 0, p);
            std::rotate(b + static_cast<std::ptrdiff_t>(q), b + static_cast<std::ptrdiff_t>(p),
                        b + static_cast<std::ptrdiff_t>(p) + 1);
        } else if (p + 1 < n && a_.antes(orden_[p + 1], slot)) {
            // primer elemento posterior a p que va despues de slot
            const auto it = std::lower_bound(
                b + static_cast<std::ptrdiff_t>(p) + 1, orden_.end(), slot,
                [this](std::size_t x, std::size_t s) { return a_.antes(x, s); });
            std::rotate(b + static_cast<std::ptrdiff_t>(p), b + static_cast<std::ptrdiff_t>(p) + 1,
                        it);
        }
    }
    std::vector<EntradaRanking> top_k(std::size_t k) const {
        const std::size_t cuantos = std::min(k, tamano());
        std::vector<EntradaRanking> out;
        out.reserve(cuantos);
        for (std::size_t i = 0; i < cuantos; ++i)
            out.push_back({i + 1, a_.ids[orden_[i]], a_.puntos[orden_[i]]});
        return out;
    }
    std::optional<std::size_t> posicion(std::size_t slot) const {
        if (slot >= tamano()) return std::nullopt;
        return limite_inferior(slot, 0, orden_.size()) + 1;
    }
    bool verificar_invariantes() const {
        if (orden_.size() != tamano()) return false;
        std::vector<bool> visto(tamano(), false);
        for (std::size_t i = 0; i < orden_.size(); ++i) {
            if (orden_[i] >= tamano() || visto[orden_[i]]) return false;
            visto[orden_[i]] = true;
            if (i > 0 && !a_.antes(orden_[i - 1], orden_[i])) return false;
        }
        return true;
    }
    std::size_t bytes_estructura() const {
        return sizeof(*this) + a_.bytes() + orden_.capacity() * sizeof(std::size_t);
    }
};

}  // namespace combat::i3

#endif
