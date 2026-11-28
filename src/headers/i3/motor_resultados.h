#ifndef COMBAT_I3_MOTOR_RESULTADOS_H
#define COMBAT_I3_MOTOR_RESULTADOS_H

// Motor de resultados de I3: estadisticas, historial acotado y ranking, con
// los tres indices mantenidos en coherencia despues de cada combate.
//
// Politica de casos borde (identica en la referencia lenta de tests/i3):
//   registrar_luchador(id)   false si id vacio o repetido.
//   registrar_resultado(r)   comprueba en este orden: ids_iguales, id_desconocido
//                            (cualquiera de los dos), ganador_invalido (no vacio y
//                            distinto de a y b), turnos_invalidos (0 o mayor que
//                            kMaxTurnosPorCombate). Si falla, NO cambia nada.
//   anular_resultado(id)     false si el id no esta en la ventana del historial;
//                            si esta, revierte estadisticas y ranking.
//   top_k(k)                 k=0 o motor vacio -> vacio; k > n -> n entradas.
//   posicion(id)             nullopt si el id no existe. 1 = primero.
//   estadisticas(id)         nullopt si el id no existe.
//   ultimos(m)               m=0 o sin eventos -> vacio.
//
// Plantilla sobre los candidatos para poder medir cada combinacion; ver
// ranking.h, historial.h e indice_ids.h.

#include "indice_ids.h"
#include "tipos.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace combat::i3 {

template <typename Ranking, typename Historial, typename Indice = IndiceStd>
class MotorResultados {
    enum class Papel { gana, pierde, empata };

    Indice indice_;
    Ranking ranking_;
    Historial historial_;
    std::vector<Estadisticas> stats_;
    std::uint64_t siguiente_ = 1;

    void aplicar(std::size_t slot, Papel papel, std::uint64_t turnos, bool sumar) {
        Estadisticas& s = stats_[slot];
        const auto mover = [sumar](std::uint64_t& campo, std::uint64_t delta) {
            if (sumar) campo += delta; else campo -= delta;
        };
        Puntos delta_puntos = 0;
        mover(s.combates, 1);
        mover(s.turnos_totales, turnos);
        switch (papel) {
            case Papel::gana: mover(s.victorias, 1); delta_puntos = kPuntosVictoria; break;
            case Papel::pierde: mover(s.derrotas, 1); delta_puntos = kPuntosDerrota; break;
            case Papel::empata: mover(s.empates, 1); delta_puntos = kPuntosEmpate; break;
        }
        s.puntos += sumar ? delta_puntos : -delta_puntos;
        ranking_.fijar_puntos(slot, s.puntos);
    }

    void aplicar_resultado(const ResultadoCombate& r, std::size_t sa, std::size_t sb, bool sumar) {
        if (r.id_ganador.empty()) {
            aplicar(sa, Papel::empata, r.turnos, sumar);
            aplicar(sb, Papel::empata, r.turnos, sumar);
        } else if (r.id_ganador == r.id_a) {
            aplicar(sa, Papel::gana, r.turnos, sumar);
            aplicar(sb, Papel::pierde, r.turnos, sumar);
        } else {
            aplicar(sa, Papel::pierde, r.turnos, sumar);
            aplicar(sb, Papel::gana, r.turnos, sumar);
        }
    }

public:
    explicit MotorResultados(std::size_t capacidad_historial) : historial_(capacidad_historial) {}

    std::size_t luchadores() const { return stats_.size(); }

    bool registrar_luchador(const std::string& id) {
        if (id.empty() || indice_.buscar(id) != nullptr) return false;
        const std::size_t slot = stats_.size();
        indice_.insertar(id, slot);
        stats_.emplace_back();
        ranking_.agregar(slot, id);
        return true;
    }

    RespuestaRegistro registrar_resultado(const ResultadoCombate& r) {
        if (r.id_a == r.id_b) return {EstadoRegistro::ids_iguales, 0};
        const std::size_t* pa = indice_.buscar(r.id_a);
        const std::size_t* pb = indice_.buscar(r.id_b);
        if (pa == nullptr || pb == nullptr) return {EstadoRegistro::id_desconocido, 0};
        if (!r.id_ganador.empty() && r.id_ganador != r.id_a && r.id_ganador != r.id_b)
            return {EstadoRegistro::ganador_invalido, 0};
        if (r.turnos == 0 || r.turnos > kMaxTurnosPorCombate)
            return {EstadoRegistro::turnos_invalidos, 0};
        const std::size_t sa = *pa;
        const std::size_t sb = *pb;
        aplicar_resultado(r, sa, sb, true);
        const std::uint64_t id = siguiente_++;
        historial_.registrar(EventoCombate{id, r});
        return {EstadoRegistro::registrado, id};
    }

    bool anular_resultado(std::uint64_t id_combate) {
        const std::optional<EventoCombate> ev = historial_.anular(id_combate);
        if (!ev) return false;
        const std::size_t* pa = indice_.buscar(ev->resultado.id_a);
        const std::size_t* pb = indice_.buscar(ev->resultado.id_b);
        if (pa == nullptr || pb == nullptr) return true;  // inalcanzable: ids ya validados
        aplicar_resultado(ev->resultado, *pa, *pb, false);
        return true;
    }

    std::vector<EntradaRanking> top_k(std::size_t k) const { return ranking_.top_k(k); }

    std::optional<std::size_t> posicion(const std::string& id) const {
        const std::size_t* p = indice_.buscar(id);
        if (p == nullptr) return std::nullopt;
        return ranking_.posicion(*p);
    }

    std::optional<Estadisticas> estadisticas(const std::string& id) const {
        const std::size_t* p = indice_.buscar(id);
        if (p == nullptr) return std::nullopt;
        return stats_[*p];
    }

    std::vector<EventoCombate> ultimos(std::size_t m) const { return historial_.ultimos(m); }

    // Coherencia entre estructuras compartidas (para pruebas y depuracion, O(n log n)).
    bool verificar_invariantes() const {
        if (!ranking_.verificar_invariantes() || !historial_.verificar_invariantes()) return false;
        if (ranking_.tamano() != stats_.size()) return false;
        for (std::size_t s = 0; s < stats_.size(); ++s) {
            const Estadisticas& e = stats_[s];
            if (e.combates != e.victorias + e.derrotas + e.empates) return false;
            const Puntos esperado = static_cast<Puntos>(e.victorias) * kPuntosVictoria +
                                    static_cast<Puntos>(e.empates) * kPuntosEmpate +
                                    static_cast<Puntos>(e.derrotas) * kPuntosDerrota;
            if (e.puntos != esperado || ranking_.puntos(s) != e.puntos) return false;
        }
        const auto todos = ranking_.top_k(stats_.size());
        if (todos.size() != stats_.size()) return false;
        for (std::size_t i = 0; i < todos.size(); ++i) {
            if (todos[i].posicion != i + 1) return false;
            if (i > 0 && !antes_en_ranking(todos[i - 1].puntos, todos[i - 1].id, todos[i].puntos,
                                           todos[i].id))
                return false;
            const std::size_t* slot = indice_.buscar(todos[i].id);
            if (slot == nullptr || stats_[*slot].puntos != todos[i].puntos) return false;
        }
        return true;
    }
};

}  // namespace combat::i3

#endif
