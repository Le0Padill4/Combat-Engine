#ifndef COMBAT_I3_TESTS_MOTOR_LENTO_H
#define COMBAT_I3_TESTS_MOTOR_LENTO_H

// Referencia lenta del motor de resultados de I3. Solo vectores y busquedas
// lineales; no reutiliza ninguna estructura ni funcion de produccion salvo los
// tipos de datos de tipos.h. Esta pensada para revisarse a mano.

#include "i3/tipos.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace combat::i3::tests {

class MotorLento {
    struct Luchador {
        std::string id;
        Estadisticas st;
    };
    std::vector<Luchador> luchadores_;
    std::vector<EventoCombate> historial_;  // del mas antiguo al mas reciente
    struct ClaveVigente {
        std::uint64_t ext;
        std::uint64_t id_combate;
        ResultadoCombate resultado;
    };
    std::vector<ClaveVigente> claves_;  // busqueda lineal
    std::size_t capacidad_;
    std::uint64_t siguiente_ = 1;

    int buscar(const std::string& id) const {
        for (std::size_t i = 0; i < luchadores_.size(); ++i)
            if (luchadores_[i].id == id) return static_cast<int>(i);
        return -1;
    }
    // delta = +1 aplica el resultado, -1 lo revierte.
    void aplicar(Luchador& l, int resultado, std::uint64_t turnos, int delta) {
        // resultado: 1 gana, 0 empata, -1 pierde
        const auto d = static_cast<std::int64_t>(delta);
        l.st.combates = static_cast<std::uint64_t>(static_cast<std::int64_t>(l.st.combates) + d);
        l.st.turnos_totales =
            static_cast<std::uint64_t>(static_cast<std::int64_t>(l.st.turnos_totales) +
                                       d * static_cast<std::int64_t>(turnos));
        if (resultado == 1) {
            l.st.victorias = static_cast<std::uint64_t>(static_cast<std::int64_t>(l.st.victorias) + d);
            l.st.puntos += d * 3;
        } else if (resultado == 0) {
            l.st.empates = static_cast<std::uint64_t>(static_cast<std::int64_t>(l.st.empates) + d);
            l.st.puntos += d * 1;
        } else {
            l.st.derrotas = static_cast<std::uint64_t>(static_cast<std::int64_t>(l.st.derrotas) + d);
        }
    }
    void aplicar_resultado(const ResultadoCombate& r, int delta) {
        Luchador& a = luchadores_[static_cast<std::size_t>(buscar(r.id_a))];
        Luchador& b = luchadores_[static_cast<std::size_t>(buscar(r.id_b))];
        if (r.id_ganador.empty()) {
            aplicar(a, 0, r.turnos, delta);
            aplicar(b, 0, r.turnos, delta);
        } else if (r.id_ganador == r.id_a) {
            aplicar(a, 1, r.turnos, delta);
            aplicar(b, -1, r.turnos, delta);
        } else {
            aplicar(a, -1, r.turnos, delta);
            aplicar(b, 1, r.turnos, delta);
        }
    }

public:
    explicit MotorLento(std::size_t capacidad_historial) : capacidad_(capacidad_historial) {}

    std::size_t luchadores() const { return luchadores_.size(); }

    bool registrar_luchador(const std::string& id) {
        if (id.empty() || buscar(id) >= 0) return false;
        luchadores_.push_back({id, Estadisticas{}});
        return true;
    }

    RespuestaRegistro registrar_resultado(const ResultadoCombate& r) {
        if (r.id_a == r.id_b) return {EstadoRegistro::ids_iguales, 0};
        if (buscar(r.id_a) < 0 || buscar(r.id_b) < 0) return {EstadoRegistro::id_desconocido, 0};
        if (!r.id_ganador.empty() && r.id_ganador != r.id_a && r.id_ganador != r.id_b)
            return {EstadoRegistro::ganador_invalido, 0};
        if (r.turnos == 0 || r.turnos > kMaxTurnosPorCombate)
            return {EstadoRegistro::turnos_invalidos, 0};
        if (r.id_externo != 0) {
            for (const auto& c : claves_) {
                if (c.ext != r.id_externo) continue;
                const bool igual = c.resultado.id_a == r.id_a && c.resultado.id_b == r.id_b &&
                                   c.resultado.id_ganador == r.id_ganador &&
                                   c.resultado.turnos == r.turnos;
                if (igual) return {EstadoRegistro::duplicado, c.id_combate};
                return {EstadoRegistro::conflicto_clave, 0};
            }
        }
        aplicar_resultado(r, +1);
        const std::uint64_t id = siguiente_++;
        historial_.push_back(EventoCombate{id, r});
        if (historial_.size() > capacidad_) historial_.erase(historial_.begin());
        if (r.id_externo != 0) claves_.push_back({r.id_externo, id, r});
        return {EstadoRegistro::registrado, id};
    }

    bool anular_resultado(std::uint64_t id) {
        for (std::size_t i = 0; i < historial_.size(); ++i) {
            if (historial_[i].id_combate == id) {
                const ResultadoCombate r = historial_[i].resultado;
                historial_.erase(historial_.begin() + static_cast<std::ptrdiff_t>(i));
                aplicar_resultado(r, -1);
                for (std::size_t k = 0; k < claves_.size(); ++k) {
                    if (claves_[k].ext == r.id_externo && claves_[k].id_combate == id) {
                        claves_.erase(claves_.begin() + static_cast<std::ptrdiff_t>(k));
                        break;
                    }
                }
                return true;
            }
        }
        return false;
    }

    std::vector<EntradaRanking> top_k(std::size_t k) const {
        std::vector<EntradaRanking> todos;
        for (const auto& l : luchadores_) todos.push_back({0, l.id, l.st.puntos});
        std::sort(todos.begin(), todos.end(), [](const EntradaRanking& x, const EntradaRanking& y) {
            if (x.puntos != y.puntos) return x.puntos > y.puntos;
            return x.id < y.id;
        });
        if (todos.size() > k) todos.resize(k);
        for (std::size_t i = 0; i < todos.size(); ++i) todos[i].posicion = i + 1;
        return todos;
    }

    std::optional<std::size_t> posicion(const std::string& id) const {
        const int s = buscar(id);
        if (s < 0) return std::nullopt;
        const Luchador& yo = luchadores_[static_cast<std::size_t>(s)];
        std::size_t antes = 0;
        for (const auto& l : luchadores_) {
            if (l.st.puntos > yo.st.puntos || (l.st.puntos == yo.st.puntos && l.id < yo.id)) ++antes;
        }
        return antes + 1;
    }

    std::optional<Estadisticas> estadisticas(const std::string& id) const {
        const int s = buscar(id);
        if (s < 0) return std::nullopt;
        return luchadores_[static_cast<std::size_t>(s)].st;
    }

    std::vector<EventoCombate> ultimos(std::size_t m) const {
        std::vector<EventoCombate> out;
        for (std::size_t i = historial_.size(); i > 0 && out.size() < m; --i)
            out.push_back(historial_[i - 1]);
        return out;
    }
};

}  // namespace combat::i3::tests

#endif
