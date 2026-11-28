// Benchmarks de I3. Genera un CSV con una fila por (operacion, escenario,
// candidato, tamano, repeticion). Ambos candidatos de una operacion reciben
// EXACTAMENTE el mismo flujo de operaciones (misma semilla) y datos iniciales.
//
// Uso:  bench_i3 <salida.csv> [--rapido]
//   --rapido: tamanos 10^2..10^3 y 2 repeticiones (solo para comprobar que corre;
//             NO produce resultados reportables).
//
// Operaciones medidas:
//   top_k      RankingAVL            vs RankingHeapSelect   (top 10)  3 escenarios
//   posicion   RankingAVL            vs RankingArreglo      (rango de un luchador)
//   historial  HistorialCircular     vs HistorialLista      (registrar + anular)
// El tiempo medido es solo el de las operaciones; la carga inicial no cuenta.

#include "../../tests/i3/generador.h"
#include "i3/historial.h"
#include "i3/ranking.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace combat::i3;
using namespace combat::i3::tests;

namespace {

using Reloj = std::chrono::steady_clock;

constexpr std::size_t kOpsPorMedicion = 2000;
constexpr std::size_t kK = 10;

std::string id_largo(std::size_t i) {
    std::string s = std::to_string(i);
    while (s.size() < 7) s = "0" + s;
    return "p" + s;
}

struct Fila {
    std::string operacion, escenario, candidato;
    std::size_t n;
    int repeticion;
    std::size_t ops;
    double ns_totales;
    unsigned long long checksum;
    std::size_t bytes;
};

std::ofstream g_csv;

void escribir(const Fila& f) {
    g_csv << f.operacion << ',' << f.escenario << ',' << f.candidato << ',' << f.n << ','
          << f.repeticion << ',' << f.ops << ',' << static_cast<long long>(f.ns_totales) << ','
          << (f.ns_totales / static_cast<double>(f.ops)) << ',' << f.checksum << ',' << f.bytes
          << '\n';
    g_csv.flush();
}

// ------------------------------------------------------------ ranking
struct OpRanking {
    bool consulta;
    std::size_t slot;
    Puntos nuevo;
};

struct DatosRanking {
    std::vector<std::string> ids;
    std::vector<Puntos> puntos;
    std::vector<OpRanking> ops;
};

DatosRanking generar_ranking(std::size_t n, unsigned pct_consulta, bool consulta_final,
                             std::uint32_t semilla) {
    Generador g(semilla);
    DatosRanking d;
    d.ids.reserve(n);
    d.puntos.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        d.ids.push_back(id_largo(i));
        d.puntos.push_back(static_cast<Puntos>(g.entero(0, 1000)));
    }
    for (std::size_t i = 0; i < kOpsPorMedicion; ++i) {
        const bool consulta = g.prob(pct_consulta);
        d.ops.push_back({consulta, static_cast<std::size_t>(g.entero(0, n - 1)),
                         static_cast<Puntos>(g.entero(0, 1000))});
    }
    if (consulta_final) d.ops.back().consulta = true;
    return d;
}

template <typename R>
Fila medir_ranking(const char* op, const char* escenario, const char* candidato,
                   const DatosRanking& d, bool por_posicion, int rep) {
    R r;
    r.cargar(d.ids, d.puntos);
    unsigned long long sink = 0;
    const auto t0 = Reloj::now();
    for (const OpRanking& o : d.ops) {
        if (!o.consulta) {
            r.fijar_puntos(o.slot, o.nuevo);
        } else if (por_posicion) {
            sink += *r.posicion(o.slot);
        } else {
            const auto v = r.top_k(kK);
            sink += static_cast<unsigned long long>(v.back().puntos) + v.size();
        }
    }
    const auto t1 = Reloj::now();
    const double ns = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    return {op, escenario, candidato, d.ids.size(), rep, d.ops.size(), ns, sink, r.bytes_estructura()};
}

// ---------------------------------------------------------- historial
struct OpHist {
    bool anular;
    std::uint64_t id;
};

struct DatosHist {
    std::vector<OpHist> ops;
    std::vector<EventoCombate> eventos;  // para los registrar, en orden
};

EventoCombate evento(std::uint64_t id) {
    return EventoCombate{id, ResultadoCombate{"p0000001", "p0000002", "p0000001", id % 50 + 1}};
}

// Simula la ventana para elegir como objetivo de anular SIEMPRE un evento vivo.
DatosHist generar_hist(std::size_t cap, unsigned pct_anular, std::uint32_t semilla) {
    Generador g(semilla);
    DatosHist d;
    std::vector<std::uint64_t> vivos(cap);
    for (std::size_t i = 0; i < cap; ++i) vivos[i] = i + 1;
    std::uint64_t siguiente = cap + 1;
    for (std::size_t i = 0; i < kOpsPorMedicion; ++i) {
        if (!vivos.empty() && g.prob(pct_anular)) {
            const std::size_t idx = static_cast<std::size_t>(g.entero(0, vivos.size() - 1));
            d.ops.push_back({true, vivos[idx]});
            vivos.erase(vivos.begin() + static_cast<std::ptrdiff_t>(idx));
        } else {
            if (vivos.size() == cap) vivos.erase(vivos.begin());
            vivos.push_back(siguiente);
            d.ops.push_back({false, siguiente});
            d.eventos.push_back(evento(siguiente));
            ++siguiente;
        }
    }
    return d;
}

template <typename H>
Fila medir_hist(const char* escenario, const char* candidato, std::size_t cap,
                const DatosHist& d, int rep) {
    H h(cap);
    for (std::size_t i = 1; i <= cap; ++i) h.registrar(evento(i));
    std::vector<EventoCombate> eventos = d.eventos;  // copia: registrar consume
    std::size_t siguiente_evento = 0;
    unsigned long long sink = 0;
    std::size_t fallos = 0;
    const auto t0 = Reloj::now();
    for (const OpHist& o : d.ops) {
        if (o.anular) {
            const auto r = h.anular(o.id);
            if (r) sink += r->resultado.turnos; else ++fallos;
        } else {
            h.registrar(std::move(eventos[siguiente_evento++]));
        }
    }
    const auto t1 = Reloj::now();
    if (fallos != 0) {
        std::cerr << "ERROR: " << fallos << " anulaciones no encontraron su evento vivo ("
                  << candidato << ")\n";
        std::exit(2);
    }
    const double ns = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    return {"historial_anular", escenario, candidato, cap, rep, d.ops.size(), ns, sink,
            h.bytes_estructura()};
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "uso: bench_i3 <salida.csv> [--rapido]\n";
        return 1;
    }
    const bool rapido = argc > 2 && std::string(argv[2]) == "--rapido";
    const std::vector<std::size_t> tamanos =
        rapido ? std::vector<std::size_t>{100, 1000} : std::vector<std::size_t>{1000, 10000, 100000, 1000000};
    const int repeticiones = rapido ? 2 : 5;

    g_csv.open(argv[1]);
    if (!g_csv) {
        std::cerr << "no se pudo abrir " << argv[1] << "\n";
        return 1;
    }
    g_csv << "operacion,escenario,candidato,n,repeticion,ops,ns_totales,ns_por_op,checksum,bytes_estructura\n";

    // consulta_final: ademas de las consultas al azar, la ultima operacion es una consulta.
    struct EscRanking { const char* nombre; unsigned pct_consulta; bool consulta_final; };
    const EscRanking esc_topk[] = {
        {"torneo_una_consulta_final", 0, true},
        {"rafaga_1pct_consultas", 1, false},
        {"ranking_en_vivo_50pct_consultas", 50, false}};
    const EscRanking esc_pos[] = {
        {"solo_lectura_100pct_consultas", 100, false},
        {"lectura_95pct_consultas", 95, false},
        {"actualizacion_90pct_cambios", 10, false}};
    struct EscHist { const char* nombre; unsigned pct_anular; };
    const EscHist esc_hist[] = {
        {"sin_anulaciones_0pct", 0},
        {"anulaciones_raras_1pct", 1},
        {"anulaciones_frecuentes_30pct", 30}};

    for (const std::size_t n : tamanos) {
        std::cerr << "n = " << n << "\n";
        for (int rep = 1; rep <= repeticiones; ++rep) {
            const std::uint32_t semilla = 20261127u + static_cast<std::uint32_t>(rep);
            for (const auto& e : esc_topk) {
                const DatosRanking d = generar_ranking(n, e.pct_consulta, e.consulta_final, semilla);
                escribir(medir_ranking<RankingAVL>("top_k", e.nombre, "AVL", d, false, rep));
                escribir(medir_ranking<RankingHeapSelect>("top_k", e.nombre, "HeapSelect", d, false, rep));
            }
            for (const auto& e : esc_pos) {
                const DatosRanking d = generar_ranking(n, e.pct_consulta, e.consulta_final, semilla);
                escribir(medir_ranking<RankingAVL>("posicion", e.nombre, "AVL", d, true, rep));
                escribir(medir_ranking<RankingArreglo>("posicion", e.nombre, "ArregloOrdenado", d, true, rep));
            }
            for (const auto& e : esc_hist) {
                const DatosHist d = generar_hist(n, e.pct_anular, semilla);
                escribir(medir_hist<HistorialCircular>(e.nombre, "BufferCircular", n, d, rep));
                escribir(medir_hist<HistorialLista>(e.nombre, "ListaDoble", n, d, rep));
            }
        }
    }
    std::cerr << "listo: " << argv[1] << "\n";
    return 0;
}
