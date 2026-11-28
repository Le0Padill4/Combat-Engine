#ifndef COMBAT_I3_TIPOS_H
#define COMBAT_I3_TIPOS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>

namespace combat::i3 {

using Puntos = std::int64_t;

// Reglas de puntaje propuestas (pendientes de aprobacion en el contrato).
constexpr Puntos kPuntosVictoria = 3;
constexpr Puntos kPuntosEmpate = 1;
constexpr Puntos kPuntosDerrota = 0;
// Limite de turnos por combate: evita desbordes al acumular turnos_totales.
constexpr std::uint64_t kMaxTurnosPorCombate = 1000000000ULL;

// Evento minimo que I3 necesita de un combate terminado. Los ids son los de
// PerfilPersonaje (I1). id_ganador vacio significa empate.
struct ResultadoCombate {
    std::string id_a;
    std::string id_b;
    std::string id_ganador;
    std::uint64_t turnos = 0;
};

struct EventoCombate {
    std::uint64_t id_combate = 0;
    ResultadoCombate resultado;
};

struct EntradaRanking {
    std::size_t posicion = 0;  // 1 = primero
    std::string id;
    Puntos puntos = 0;
};

struct Estadisticas {
    std::uint64_t combates = 0;
    std::uint64_t victorias = 0;
    std::uint64_t derrotas = 0;
    std::uint64_t empates = 0;
    std::uint64_t turnos_totales = 0;
    Puntos puntos = 0;
};

enum class EstadoRegistro {
    registrado,
    ids_iguales,
    id_desconocido,
    ganador_invalido,
    turnos_invalidos
};

struct RespuestaRegistro {
    EstadoRegistro estado;
    std::uint64_t id_combate;  // 0 si no se registro
};

// Orden total del ranking: mas puntos primero; empate de puntos -> id menor.
inline bool antes_en_ranking(Puntos p1, const std::string& id1, Puntos p2,
                             const std::string& id2) {
    if (p1 != p2) return p1 > p2;
    return id1 < id2;
}

inline bool operator==(const ResultadoCombate& a, const ResultadoCombate& b) {
    return std::tie(a.id_a, a.id_b, a.id_ganador, a.turnos) ==
           std::tie(b.id_a, b.id_b, b.id_ganador, b.turnos);
}
inline bool operator==(const EventoCombate& a, const EventoCombate& b) {
    return a.id_combate == b.id_combate && a.resultado == b.resultado;
}
inline bool operator==(const EntradaRanking& a, const EntradaRanking& b) {
    return a.posicion == b.posicion && a.id == b.id && a.puntos == b.puntos;
}
inline bool operator==(const Estadisticas& a, const Estadisticas& b) {
    return a.combates == b.combates && a.victorias == b.victorias &&
           a.derrotas == b.derrotas && a.empates == b.empates &&
           a.turnos_totales == b.turnos_totales && a.puntos == b.puntos;
}

}  // namespace combat::i3

#endif
