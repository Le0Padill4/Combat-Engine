#ifndef COMBAT_I3_AVL_RANKING_H
#define COMBAT_I3_AVL_RANKING_H

#include "tipos.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace combat::i3 {

// AVL propio, aumentado con el tamano de cada subarbol. Clave: (puntos, id)
// con el orden de antes_en_ranking, de modo que el recorrido in-orden es el
// ranking de mejor a peor.
//
// Invariantes (los comprueba verificar_invariantes):
//   1. Orden BST estricto: todo nodo del hijo izquierdo va antes que el nodo y
//      todo nodo del derecho va despues. No hay claves repetidas.
//   2. Balance AVL: |altura(izq) - altura(der)| <= 1 en cada nodo.
//   3. altura(n) = 1 + max(altura(izq), altura(der)) almacenada y correcta.
//   4. tam(n) = 1 + tam(izq) + tam(der) almacenado y correcto.
class ArbolAVL {
    struct Nodo {
        Puntos puntos;
        std::string id;
        int altura = 1;
        std::size_t tam = 1;
        std::unique_ptr<Nodo> izq;
        std::unique_ptr<Nodo> der;
        Nodo(Puntos p, const std::string& i) : puntos(p), id(i) {}
    };
    using P = std::unique_ptr<Nodo>;

    P raiz_;

    static int alt(const P& n) { return n ? n->altura : 0; }
    static std::size_t tam(const P& n) { return n ? n->tam : 0; }
    static void actualizar(Nodo& n) {
        n.altura = 1 + std::max(alt(n.izq), alt(n.der));
        n.tam = 1 + tam(n.izq) + tam(n.der);
    }
    static P rotar_der(P y) {
        P x = std::move(y->izq);
        y->izq = std::move(x->der);
        actualizar(*y);
        x->der = std::move(y);
        actualizar(*x);
        return x;
    }
    static P rotar_izq(P x) {
        P y = std::move(x->der);
        x->der = std::move(y->izq);
        actualizar(*x);
        y->izq = std::move(x);
        actualizar(*y);
        return y;
    }
    static P balancear(P n) {
        actualizar(*n);
        const int b = alt(n->izq) - alt(n->der);
        if (b > 1) {
            if (alt(n->izq->izq) < alt(n->izq->der)) n->izq = rotar_izq(std::move(n->izq));
            return rotar_der(std::move(n));
        }
        if (b < -1) {
            if (alt(n->der->der) < alt(n->der->izq)) n->der = rotar_der(std::move(n->der));
            return rotar_izq(std::move(n));
        }
        return n;
    }
    static bool igual(Puntos p, const std::string& id, const Nodo& n) {
        return p == n.puntos && id == n.id;
    }

    static P insertar(P n, Puntos p, const std::string& id, bool& ok) {
        if (!n) {
            ok = true;
            return std::make_unique<Nodo>(p, id);
        }
        if (igual(p, id, *n)) {
            ok = false;
            return n;
        }
        if (antes_en_ranking(p, id, n->puntos, n->id))
            n->izq = insertar(std::move(n->izq), p, id, ok);
        else
            n->der = insertar(std::move(n->der), p, id, ok);
        return balancear(std::move(n));
    }

    static P extraer_min(P n, Puntos& p, std::string& id) {
        if (!n->izq) {
            p = n->puntos;
            id = std::move(n->id);
            return std::move(n->der);
        }
        n->izq = extraer_min(std::move(n->izq), p, id);
        return balancear(std::move(n));
    }

    static P eliminar(P n, Puntos p, const std::string& id, bool& ok) {
        if (!n) {
            ok = false;
            return n;
        }
        if (igual(p, id, *n)) {
            ok = true;
            if (!n->izq) return std::move(n->der);
            if (!n->der) return std::move(n->izq);
            Puntos sp = 0;
            std::string sid;
            n->der = extraer_min(std::move(n->der), sp, sid);
            n->puntos = sp;
            n->id = std::move(sid);
            return balancear(std::move(n));
        }
        if (antes_en_ranking(p, id, n->puntos, n->id))
            n->izq = eliminar(std::move(n->izq), p, id, ok);
        else
            n->der = eliminar(std::move(n->der), p, id, ok);
        return balancear(std::move(n));
    }

    template <typename F>
    static void recorrer(const Nodo* n, std::size_t& restantes, F& f) {
        if (n == nullptr || restantes == 0) return;
        recorrer(n->izq.get(), restantes, f);
        if (restantes == 0) return;
        f(n->puntos, n->id);
        --restantes;
        recorrer(n->der.get(), restantes, f);
    }

    // Comprueba las cuatro invariantes del subarbol; lo(hi) son cotas exclusivas.
    static bool verificar(const Nodo* n, const Nodo* lo, const Nodo* hi, int& altura,
                          std::size_t& tamano) {
        if (n == nullptr) {
            altura = 0;
            tamano = 0;
            return true;
        }
        if (lo && !antes_en_ranking(lo->puntos, lo->id, n->puntos, n->id)) return false;
        if (hi && !antes_en_ranking(n->puntos, n->id, hi->puntos, hi->id)) return false;
        int ai = 0, ad = 0;
        std::size_t ti = 0, td = 0;
        if (!verificar(n->izq.get(), lo, n, ai, ti)) return false;
        if (!verificar(n->der.get(), n, hi, ad, td)) return false;
        if (ai - ad > 1 || ad - ai > 1) return false;
        altura = 1 + std::max(ai, ad);
        tamano = 1 + ti + td;
        return n->altura == altura && n->tam == tamano;
    }

public:
    ArbolAVL() = default;
    ArbolAVL(const ArbolAVL&) = delete;
    ArbolAVL& operator=(const ArbolAVL&) = delete;

    std::size_t tamano() const { return tam(raiz_); }
    int altura() const { return alt(raiz_); }

    bool insertar(Puntos p, const std::string& id) {
        bool ok = false;
        raiz_ = insertar(std::move(raiz_), p, id, ok);
        return ok;
    }
    bool eliminar(Puntos p, const std::string& id) {
        bool ok = false;
        raiz_ = eliminar(std::move(raiz_), p, id, ok);
        return ok;
    }

    // Cantidad de claves que van antes que (p,id) si la clave existe; si no, nullopt.
    std::optional<std::size_t> posicion(Puntos p, const std::string& id) const {
        std::size_t acc = 0;
        const Nodo* n = raiz_.get();
        while (n != nullptr) {
            if (igual(p, id, *n)) return acc + tam(n->izq);
            if (antes_en_ranking(p, id, n->puntos, n->id)) {
                n = n->izq.get();
            } else {
                acc += tam(n->izq) + 1;
                n = n->der.get();
            }
        }
        return std::nullopt;
    }

    // Llama f(puntos, id) para las k primeras claves en orden de ranking.
    template <typename F>
    void primeros(std::size_t k, F f) const {
        std::size_t restantes = k;
        recorrer(raiz_.get(), restantes, f);
    }

    bool verificar_invariantes() const {
        int a = 0;
        std::size_t t = 0;
        return verificar(raiz_.get(), nullptr, nullptr, a, t);
    }

    // Estimacion estructural: nodos * sizeof(Nodo); excluye buffers de strings largos.
    std::size_t bytes_estructura() const { return sizeof(*this) + tamano() * sizeof(Nodo); }
};

}  // namespace combat::i3

#endif
