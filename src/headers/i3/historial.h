#ifndef COMBAT_I3_HISTORIAL_H
#define COMBAT_I3_HISTORIAL_H

// Historial acotado de combates: dos candidatos con la MISMA operacion y
// semantica (ventana de los ultimos `capacidad` eventos):
//
//   registrar(evento)  agrega como el mas reciente; si ya hay `capacidad`
//                      eventos, expulsa el mas antiguo.
//   ultimos(m)         hasta m eventos, del mas reciente al mas antiguo.
//   anular(id)         quita el evento con ese id_combate y lo devuelve; si no
//                      esta en la ventana (nunca existio, ya anulado o
//                      expulsado) devuelve nullopt y no cambia nada.
//
// Precondicion (la garantiza el motor): los id_combate registrados son
// estrictamente crecientes. Se comprueba y lanza std::invalid_argument.
// Capacidad 0 lanza std::invalid_argument.
//
//   HistorialCircular   arreglo de tamano fijo usado como buffer circular.
//   HistorialLista      lista doblemente enlazada propia + indice id->nodo.

#include "tipos.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace combat::i3 {

class HistorialCircular {
    std::vector<EventoCombate> buf_;
    std::size_t cap_;
    std::size_t cabeza_ = 0;  // indice fisico del mas antiguo
    std::size_t n_ = 0;
    std::uint64_t ultimo_id_ = 0;

    std::size_t fisico(std::size_t logico) const { return (cabeza_ + logico) % cap_; }

public:
    explicit HistorialCircular(std::size_t cap) : buf_(cap), cap_(cap) {
        if (cap == 0) throw std::invalid_argument("Historial: capacidad 0");
    }

    std::size_t tamano() const { return n_; }
    std::size_t capacidad() const { return cap_; }

    void registrar(EventoCombate e) {
        if (e.id_combate <= ultimo_id_) throw std::invalid_argument("Historial: id no creciente");
        ultimo_id_ = e.id_combate;
        if (n_ == cap_) {
            buf_[cabeza_] = std::move(e);
            cabeza_ = (cabeza_ + 1) % cap_;
        } else {
            buf_[fisico(n_)] = std::move(e);
            ++n_;
        }
    }

    std::vector<EventoCombate> ultimos(std::size_t m) const {
        const std::size_t cuantos = m < n_ ? m : n_;
        std::vector<EventoCombate> out;
        out.reserve(cuantos);
        for (std::size_t i = 0; i < cuantos; ++i) out.push_back(buf_[fisico(n_ - 1 - i)]);
        return out;
    }

    // Busqueda binaria (ids crecientes) O(log N) + desplazar los mas recientes O(N).
    std::optional<EventoCombate> anular(std::uint64_t id) {
        std::size_t lo = 0, hi = n_;
        while (lo < hi) {
            const std::size_t mid = lo + (hi - lo) / 2;
            if (buf_[fisico(mid)].id_combate < id)
                lo = mid + 1;
            else
                hi = mid;
        }
        if (lo == n_ || buf_[fisico(lo)].id_combate != id) return std::nullopt;
        EventoCombate quitado = std::move(buf_[fisico(lo)]);
        for (std::size_t j = lo; j + 1 < n_; ++j) buf_[fisico(j)] = std::move(buf_[fisico(j + 1)]);
        --n_;
        return quitado;
    }

    // Invariantes: n <= capacidad, cabeza < capacidad, ids estrictamente crecientes
    // en orden logico y todos <= ultimo id registrado.
    bool verificar_invariantes() const {
        if (n_ > cap_ || cabeza_ >= cap_ || buf_.size() != cap_) return false;
        for (std::size_t i = 0; i < n_; ++i) {
            const auto id = buf_[fisico(i)].id_combate;
            if (id == 0 || id > ultimo_id_) return false;
            if (i > 0 && buf_[fisico(i - 1)].id_combate >= id) return false;
        }
        return true;
    }
    std::size_t bytes_estructura() const {
        return sizeof(*this) + buf_.capacity() * sizeof(EventoCombate);
    }
};

class HistorialLista {
    struct Nodo {
        EventoCombate ev;
        Nodo* ant = nullptr;
        Nodo* sig = nullptr;
        explicit Nodo(EventoCombate e) : ev(std::move(e)) {}
    };

    std::size_t cap_;
    Nodo* cabeza_ = nullptr;  // mas antiguo
    Nodo* cola_ = nullptr;    // mas reciente
    std::size_t n_ = 0;
    std::uint64_t ultimo_id_ = 0;
    std::unordered_map<std::uint64_t, Nodo*> indice_;

    void desenlazar(Nodo* x) {
        if (x->ant) x->ant->sig = x->sig; else cabeza_ = x->sig;
        if (x->sig) x->sig->ant = x->ant; else cola_ = x->ant;
        --n_;
    }

public:
    explicit HistorialLista(std::size_t cap) : cap_(cap) {
        if (cap == 0) throw std::invalid_argument("Historial: capacidad 0");
    }
    HistorialLista(const HistorialLista&) = delete;
    HistorialLista& operator=(const HistorialLista&) = delete;
    ~HistorialLista() {
        Nodo* x = cabeza_;
        while (x != nullptr) {
            Nodo* s = x->sig;
            delete x;
            x = s;
        }
    }

    std::size_t tamano() const { return n_; }
    std::size_t capacidad() const { return cap_; }

    void registrar(EventoCombate e) {
        if (e.id_combate <= ultimo_id_) throw std::invalid_argument("Historial: id no creciente");
        std::unique_ptr<Nodo> nuevo(new Nodo(std::move(e)));
        if (n_ == cap_) {
            Nodo* viejo = cabeza_;
            indice_.erase(viejo->ev.id_combate);
            desenlazar(viejo);
            delete viejo;
        }
        indice_.emplace(nuevo->ev.id_combate, nuevo.get());
        ultimo_id_ = nuevo->ev.id_combate;
        nuevo->ant = cola_;
        if (cola_) cola_->sig = nuevo.get(); else cabeza_ = nuevo.get();
        cola_ = nuevo.release();
        ++n_;
    }

    std::vector<EventoCombate> ultimos(std::size_t m) const {
        std::vector<EventoCombate> out;
        out.reserve(m < n_ ? m : n_);
        for (const Nodo* x = cola_; x != nullptr && out.size() < m; x = x->ant)
            out.push_back(x->ev);
        return out;
    }

    // O(1) esperado gracias al indice.
    std::optional<EventoCombate> anular(std::uint64_t id) {
        const auto it = indice_.find(id);
        if (it == indice_.end()) return std::nullopt;
        Nodo* x = it->second;
        indice_.erase(it);
        desenlazar(x);
        EventoCombate quitado = std::move(x->ev);
        delete x;
        return quitado;
    }

    // Invariantes: enlaces ant/sig coherentes, n_ = nodos recorridos <= capacidad,
    // ids estrictamente crecientes de cabeza a cola, indice con exactamente un
    // puntero correcto por nodo.
    bool verificar_invariantes() const {
        if (n_ > cap_ || indice_.size() != n_) return false;
        if ((cabeza_ == nullptr) != (cola_ == nullptr) || (n_ == 0) != (cabeza_ == nullptr))
            return false;
        std::size_t vistos = 0;
        const Nodo* previo = nullptr;
        for (const Nodo* x = cabeza_; x != nullptr; x = x->sig) {
            if (x->ant != previo) return false;
            if (previo && previo->ev.id_combate >= x->ev.id_combate) return false;
            if (x->ev.id_combate == 0 || x->ev.id_combate > ultimo_id_) return false;
            const auto it = indice_.find(x->ev.id_combate);
            if (it == indice_.end() || it->second != x) return false;
            previo = x;
            if (++vistos > n_) return false;
        }
        return vistos == n_ && previo == cola_;
    }
    std::size_t bytes_estructura() const {
        // nodos + estimacion del indice (bucket + nodo de la tabla): solo estructural.
        return sizeof(*this) + n_ * sizeof(Nodo) +
               indice_.bucket_count() * sizeof(void*) +
               indice_.size() * (sizeof(std::uint64_t) + sizeof(Nodo*) + sizeof(void*));
    }
};

}  // namespace combat::i3

#endif
