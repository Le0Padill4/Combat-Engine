#ifndef COMBAT_I3_ADAPTADOR_COMBATE_H
#define COMBAT_I3_ADAPTADOR_COMBATE_H

#include <cstdint>
#include <optional>
#include <string>

#include "tipos.h"

// Adaptador entre un combate terminado (I2) y el evento que consume I3.
// Es una plantilla "duck-typed": no incluye ni depende de headers de I2.
// Exige del tipo C los metodos de Combate: primero()/segundo() (o los
// punteros que se pasen), ganador(), terminado(), turnosEjecutados().
//
// El combate real no conoce los ids de I1 (solo el nombre), asi que quien
// llama los pasa explicitamente: id_primero e id_segundo en el mismo orden
// en que se construyo el Combate. La comparacion del ganador es por
// identidad de objeto (&ganador == &primero), no por nombre.
namespace combat::i3 {

// Devuelve nullopt si el combate no ha terminado (no hay resultado que
// registrar). Si termino sin ganador unico (ambos caidos) se registra empate.
// Nota: Combate::ganador() == nullptr tambien ocurre si no termino; por eso
// se consulta terminado() primero.
template <typename C, typename P>
std::optional<ResultadoCombate> resultado_desde_combate(
    const C& combate, const P& primero, const P& segundo,
    const std::string& id_primero, const std::string& id_segundo,
    std::uint64_t id_externo = 0) {
    if (!combate.terminado()) return std::nullopt;
    ResultadoCombate r;
    r.id_a = id_primero;
    r.id_b = id_segundo;
    r.turnos = static_cast<std::uint64_t>(combate.turnosEjecutados());
    r.id_externo = id_externo;
    const auto* g = combate.ganador();
    if (g == nullptr) {
        r.id_ganador.clear();  // empate
    } else if (static_cast<const void*>(g) == static_cast<const void*>(&primero)) {
        r.id_ganador = id_primero;
    } else if (static_cast<const void*>(g) == static_cast<const void*>(&segundo)) {
        r.id_ganador = id_segundo;
    } else {
        // Ganador que no es ninguno de los dos: se deja un id imposible para
        // que el motor responda ganador_invalido en vez de adivinar.
        r.id_ganador = "\x01<desconocido>";
    }
    return r;
}

}  // namespace combat::i3

#endif
