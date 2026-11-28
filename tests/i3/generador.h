#ifndef COMBAT_I3_TESTS_GENERADOR_H
#define COMBAT_I3_TESTS_GENERADOR_H

// Generador reproducible de operaciones sinteticas. Usa std::mt19937, cuya
// secuencia esta fijada por el estandar y es identica en Windows, macOS y Linux.
// NO usa std::uniform_int_distribution (su resultado depende de la libreria):
// el mapeo a rangos se hace aqui con modulo (sesgo despreciable y reproducible).

#include "i3/tipos.h"

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace combat::i3::tests {

class Generador {
    std::mt19937 rng_;

public:
    explicit Generador(std::uint32_t semilla) : rng_(semilla) {}
    // Entero uniforme en [lo, hi], lo <= hi.
    std::uint64_t entero(std::uint64_t lo, std::uint64_t hi) {
        const std::uint64_t rango = hi - lo + 1;
        const std::uint64_t r = (static_cast<std::uint64_t>(rng_()) << 32) | rng_();
        return lo + r % rango;
    }
    bool prob(unsigned porcentaje) { return entero(0, 99) < porcentaje; }
};

enum class TipoOp { luchador, resultado, anular, top_k, posicion, estadisticas, ultimos };

struct Op {
    TipoOp tipo = TipoOp::top_k;
    std::string id;           // luchador / posicion / estadisticas
    ResultadoCombate res;     // resultado
    std::uint64_t numero = 0; // anular: id_combate; top_k / ultimos: k, m
};

struct PerfilEscenario {
    unsigned pool_luchadores;  // ids validos: L000..L(pool-1)
    // Mezcla (suma 100): luchador, resultado, anular, top_k, posicion, estadisticas, ultimos
    unsigned mezcla[7];
};

inline std::string id_luchador(std::uint64_t i) {
    std::string s = std::to_string(i);
    while (s.size() < 3) s = "0" + s;
    return "L" + s;
}

// Contexto compartido: ids de combate que la referencia acepto (candidatos a anular).
struct ContextoGen {
    std::vector<std::uint64_t> combates_aceptados;
    std::size_t luchadores_registrados = 0;
};

// Id de luchador: valido (del pool), desconocido ("X..") o vacio, con sesgo a validos.
inline std::string id_aleatorio(Generador& g, const PerfilEscenario& p, unsigned pct_basura) {
    if (g.prob(pct_basura)) {
        if (g.prob(25)) return "";
        return "X" + std::to_string(g.entero(0, 9));
    }
    return id_luchador(g.entero(0, p.pool_luchadores - 1));
}

inline Op siguiente_op(Generador& g, const PerfilEscenario& p, const ContextoGen& ctx) {
    Op op;
    unsigned tiro = static_cast<unsigned>(g.entero(0, 99));
    unsigned acum = 0;
    int cual = 0;
    for (int i = 0; i < 7; ++i) {
        acum += p.mezcla[i];
        if (tiro < acum) {
            cual = i;
            break;
        }
    }
    op.tipo = static_cast<TipoOp>(cual);
    switch (op.tipo) {
        case TipoOp::luchador:
            // Registra en orden con algo de repeticion/invalidos para probar bordes.
            if (g.prob(10)) op.id = "";
            else if (g.prob(15) && ctx.luchadores_registrados > 0)
                op.id = id_luchador(g.entero(0, ctx.luchadores_registrados - 1));  // duplicado
            else op.id = id_luchador(g.entero(0, p.pool_luchadores - 1));
            break;
        case TipoOp::resultado: {
            op.res.id_a = id_aleatorio(g, p, 8);
            op.res.id_b = g.prob(3) ? op.res.id_a : id_aleatorio(g, p, 8);
            const auto r = g.entero(0, 99);
            if (r < 40) op.res.id_ganador = op.res.id_a;
            else if (r < 80) op.res.id_ganador = op.res.id_b;
            else if (r < 94) op.res.id_ganador = "";  // empate
            else op.res.id_ganador = "Z9";            // ganador invalido
            const auto t = g.entero(0, 99);
            if (t < 3) op.res.turnos = 0;
            else if (t < 6) op.res.turnos = kMaxTurnosPorCombate + 1;
            else if (t < 8) op.res.turnos = kMaxTurnosPorCombate;
            else op.res.turnos = g.entero(1, 60);
            break;
        }
        case TipoOp::anular:
            if (!ctx.combates_aceptados.empty() && g.prob(80)) {
                // sesgo a eventos recientes, pero tambien a los viejos (fuera de ventana)
                const std::size_t n = ctx.combates_aceptados.size();
                const std::size_t i = g.prob(60) ? n - 1 - g.entero(0, n < 6 ? n - 1 : 5)
                                                  : g.entero(0, n - 1);
                op.numero = ctx.combates_aceptados[i];
            } else {
                op.numero = g.entero(0, ctx.combates_aceptados.size() + 5);  // incluye 0 y futuros
            }
            break;
        case TipoOp::top_k:
            op.numero = g.prob(10) ? 0 : g.entero(1, p.pool_luchadores + 3);
            break;
        case TipoOp::posicion:
        case TipoOp::estadisticas:
            op.id = id_aleatorio(g, p, 15);
            break;
        case TipoOp::ultimos:
            op.numero = g.prob(10) ? 0 : g.entero(1, 40);
            break;
    }
    return op;
}

}  // namespace combat::i3::tests

#endif
