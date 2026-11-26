#include "i1/registro_personajes.h"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace combat::i1;
using Reloj = std::chrono::steady_clock;
constexpr unsigned SEMILLA = 20261125;
// Long batches reduce the impact of coarse clocks in older Windows toolchains.
constexpr std::size_t CONSULTAS = 1000000;

std::string id_de(std::size_t i) {
    std::ostringstream s;
    s << "id-" << std::setw(10) << std::setfill('0') << i;
    return s.str();
}

PerfilPersonaje perfil_de(std::size_t i) {
    const auto id = id_de(i);
    return {id, "Ficticio " + id, {100 + static_cast<int>(i % 30), 80, 18, 10},
            {"Golpe", "Especial", 25}};
}

struct Consultas {
    std::vector<std::string> ids;
    std::size_t aciertos = 0;
    std::uint64_t checksum = 0;
};

Consultas generar(std::size_t n, int escenario) {
    std::mt19937 rng(SEMILLA + static_cast<unsigned>(n) + static_cast<unsigned>(escenario));
    Consultas salida;
    salida.ids.reserve(CONSULTAS);
    for (std::size_t i = 0; i < CONSULTAS; ++i) {
        std::size_t indice;
        if (escenario == 0) {
            const bool existe = rng() % 100 < 80;
            indice = rng() % n + (existe ? 0 : n);
        } else {
            indice = rng() % 100 < 90 ? rng() % 32 : rng() % n;
        }
        salida.ids.push_back(id_de(indice));
        if (indice < n) {
            ++salida.aciertos;
            salida.checksum += 100 + indice % 30;
        }
    }
    return salida;
}

template <typename Registro>
long long construir(Registro& registro, const std::vector<PerfilPersonaje>& perfiles) {
    const auto inicio = Reloj::now();
    // Same ascending-ID input for both candidates; excludes fixture generation.
    for (const auto& p : perfiles)
        if (registro.registrar_personaje(p) != ResultadoRegistro::registrado)
            throw std::runtime_error("construccion incompleta");
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Reloj::now() - inicio).count();
}

template <typename Registro>
void medir(std::ofstream& csv, const Registro& registro, const Consultas& consultas,
           const char* candidato, const char* escenario, std::size_t n,
           int repeticion, long long construccion_ns) {
    for (std::size_t i = 0; i < 1000; ++i) {
        const auto p = registro.buscar_personaje(consultas.ids[i]);
        if (p && p->atributos.hp_max < 1) throw std::runtime_error("warmup invalido");
    }
    std::uint64_t checksum = 0;
    std::size_t aciertos = 0;
    const auto inicio = Reloj::now();
    for (const auto& id : consultas.ids) {
        const auto p = registro.buscar_personaje(id);
        if (p) {
            checksum += static_cast<std::uint64_t>(p->atributos.hp_max);
            ++aciertos;
        }
    }
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        Reloj::now() - inicio).count();
    if (aciertos != consultas.aciertos || checksum != consultas.checksum)
        throw std::runtime_error("resultado medido no coincide con fixture");
    csv << "buscar_personaje," << escenario << ',' << n << ',' << candidato << ','
        << repeticion << ',' << consultas.ids.size() << ',' << aciertos << ','
        << SEMILLA << ',' << construccion_ns << ',' << ns << ','
        << static_cast<double>(ns) / static_cast<double>(consultas.ids.size()) << ','
        << registro.bytes_estructura() << ',' << checksum << '\n';
}
}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::runtime_error("uso: benchmark salida.csv repeticiones");
        std::size_t leidos = 0;
        const std::string argumento = argv[2];
        const int repeticiones = std::stoi(argumento, &leidos);
        if (leidos != argumento.size() || repeticiones < 3 || repeticiones > 100)
            throw std::runtime_error("repeticiones debe estar entre 3 y 100");
        std::ofstream csv(argv[1]);
        if (!csv) throw std::runtime_error("no se pudo abrir CSV");
        csv.imbue(std::locale::classic());
        csv << std::setprecision(12);
        csv << "operacion,escenario,n,candidato,repeticion,consultas,aciertos,semilla,"
               "construccion_ns,consulta_total_ns,ns_por_consulta,"
               "bytes_estructura_estimados,checksum\n";
        for (const std::size_t n : {1000U, 4000U, 16000U, 64000U}) {
            std::vector<PerfilPersonaje> perfiles;
            perfiles.reserve(n);
            for (std::size_t i = 0; i < n; ++i) perfiles.push_back(perfil_de(i));
            const Consultas escenarios[] = {generar(n, 0), generar(n, 1)};
            const char* nombres[] = {"uniforme_80_aciertos", "popular_90_por_ciento"};
            for (int r = 1; r <= repeticiones; ++r) {
                RegistroPersonajesHash hash;
                RegistroPersonajesOrdenado ordenado;
                long long tiempo_hash, tiempo_ordenado;
                if (r % 2) {
                    tiempo_hash = construir(hash, perfiles);
                    tiempo_ordenado = construir(ordenado, perfiles);
                } else {
                    tiempo_ordenado = construir(ordenado, perfiles);
                    tiempo_hash = construir(hash, perfiles);
                }
                if (!hash.verificar_invariantes() || !ordenado.verificar_invariantes())
                    throw std::runtime_error("invariantes antes del benchmark");
                for (int e = 0; e < 2; ++e) {
                    if (r == 1)
                        for (const auto& id : escenarios[e].ids)
                            if (hash.buscar_personaje(id) != ordenado.buscar_personaje(id))
                                throw std::runtime_error("candidatos no equivalentes");
                    if (r % 2) {
                        medir(csv, hash, escenarios[e], "hash", nombres[e], n, r, tiempo_hash);
                        medir(csv, ordenado, escenarios[e], "vector_ordenado", nombres[e], n, r,
                              tiempo_ordenado);
                    } else {
                        medir(csv, ordenado, escenarios[e], "vector_ordenado", nombres[e], n, r,
                              tiempo_ordenado);
                        medir(csv, hash, escenarios[e], "hash", nombres[e], n, r, tiempo_hash);
                    }
                }
            }
            std::cout << "Medido n=" << n << " escenarios=2 candidatos=2 repeticiones="
                      << repeticiones << '\n';
        }
        csv.flush();
        if (!csv) throw std::runtime_error("fallo al escribir CSV");
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
