#ifndef COMBAT_I1_HASH_TABLE_H
#define COMBAT_I1_HASH_TABLE_H

#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace combat::i1 {

// Separate chaining. The buckets and collision lookup are implemented here;
// std::vector only supplies sequence storage, not an associative container.
template <typename Clave, typename Valor, typename Hash = std::hash<Clave>,
          typename Igual = std::equal_to<Clave>>
class HashTable {
    using Entrada = std::pair<Clave, Valor>;
    using Bucket = std::vector<Entrada>;
    std::vector<Bucket> buckets_;
    std::size_t tamano_ = 0;
    Hash hash_;
    Igual igual_;

    void crecer() {
        if (buckets_.size() > buckets_.max_size() / 2)
            throw std::length_error("HashTable: capacidad maxima");
        std::vector<Bucket> nuevos(buckets_.size() * 2);
        // Copy first, then swap: allocation/copy failures preserve old entries.
        for (const auto& bucket : buckets_)
            for (const auto& entrada : bucket)
                nuevos[hash_(entrada.first) % nuevos.size()].push_back(entrada);
        buckets_.swap(nuevos);
    }

public:
    explicit HashTable(std::size_t capacidad = 8)
        : buckets_(capacidad == 0 ? 1 : capacidad) {}
    HashTable(const HashTable&) = default;
    HashTable& operator=(const HashTable&) = default;

    std::size_t tamano() const { return tamano_; }
    std::size_t capacidad() const { return buckets_.size(); }
    double factor_carga() const {
        return static_cast<double>(tamano_) / static_cast<double>(buckets_.size());
    }

    const Valor* buscar(const Clave& clave) const {
        const auto& bucket = buckets_[hash_(clave) % buckets_.size()];
        for (const auto& entrada : bucket)
            if (igual_(entrada.first, clave)) return &entrada.second;
        return nullptr;
    }

    bool insertar(const Clave& clave, const Valor& valor) {
        if (buscar(clave)) return false;
        if (tamano_ == std::numeric_limits<std::size_t>::max())
            throw std::length_error("HashTable: tamano maximo");
        // Inputs may refer to a value already stored in this table. Copy them
        // before a rehash invalidates the original bucket addresses.
        Entrada nueva(clave, valor);
        // A one-bucket table needs two growth steps for its first entry.
        while (static_cast<double>(tamano_ + 1) /
                   static_cast<double>(buckets_.size()) > 0.75)
            crecer();
        const auto indice = hash_(nueva.first) % buckets_.size();
        buckets_[indice].push_back(std::move(nueva));
        ++tamano_;
        return true;
    }

    bool verificar_invariantes() const {
        if (buckets_.empty() || factor_carga() > 0.75) return false;
        std::size_t contadas = 0;
        for (std::size_t b = 0; b < buckets_.size(); ++b) {
            const auto& bucket = buckets_[b];
            contadas += bucket.size();
            for (std::size_t i = 0; i < bucket.size(); ++i) {
                if (hash_(bucket[i].first) % buckets_.size() != b) return false;
                for (std::size_t j = 0; j < i; ++j)
                    if (igual_(bucket[i].first, bucket[j].first)) return false;
            }
        }
        return contadas == tamano_;
    }

    // Structural estimate only: excludes dynamic string buffers and allocator
    // metadata. This is not RSS or a complete memory measurement.
    std::size_t bytes_estructura() const {
        std::size_t bytes = sizeof(*this) + buckets_.capacity() * sizeof(Bucket);
        for (const auto& bucket : buckets_)
            bytes += bucket.capacity() * sizeof(Entrada);
        return bytes;
    }
};
}  // namespace combat::i1
#endif
