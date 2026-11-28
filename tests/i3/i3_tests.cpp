// Pruebas de I3: AVL, candidatos de ranking e historial, motor de resultados y
// comparacion diferencial (>= 10.000 operaciones aleatorias por combinacion)
// contra la referencia lenta de motor_lento.h.

#include "generador.h"
#include "i3/adaptador_combate.h"
#include "i3/historial.h"
#include "i3/motor_resultados.h"
#include "i3/ranking.h"
#include "motor_lento.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace combat::i3;
using namespace combat::i3::tests;

namespace {

long long g_comprobaciones = 0;
long long g_fallos = 0;
long long g_ops_diferenciales = 0;
std::map<std::string, long long> g_cobertura;

void comprobar(bool ok, const char* texto, int linea) {
    ++g_comprobaciones;
    if (!ok) {
        ++g_fallos;
        std::cerr << "FALLO linea " << linea << ": " << texto << "\n";
    }
}
#define CHECK(cond) comprobar((cond), #cond, __LINE__)

template <typename F>
bool lanza_invalid_argument(F f) {
    try {
        f();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

// ------------------------------------------------------------------ AVL
void pruebas_avl_basicas() {
    ArbolAVL t;
    CHECK(t.tamano() == 0);
    CHECK(t.altura() == 0);
    CHECK(t.verificar_invariantes());
    CHECK(!t.posicion(0, "a").has_value());
    CHECK(!t.eliminar(0, "a"));
    int llamadas = 0;
    t.primeros(5, [&llamadas](Puntos, const std::string&) { ++llamadas; });
    CHECK(llamadas == 0);

    CHECK(t.insertar(5, "b"));
    CHECK(!t.insertar(5, "b"));  // clave repetida
    CHECK(t.insertar(5, "a"));   // mismo puntaje, id menor va antes
    CHECK(t.posicion(5, "a") == std::optional<std::size_t>(0));
    CHECK(t.posicion(5, "b") == std::optional<std::size_t>(1));
    CHECK(!t.posicion(5, "c").has_value());
    CHECK(t.insertar(9, "z"));  // mas puntos va primero
    CHECK(t.posicion(9, "z") == std::optional<std::size_t>(0));
    CHECK(t.eliminar(9, "z"));
    CHECK(!t.eliminar(9, "z"));
    CHECK(t.tamano() == 2);
    CHECK(t.verificar_invariantes());
}

void pruebas_avl_degeneradas() {
    // Secuencias que degeneran un BST sin balance: la altura debe ser logaritmica.
    for (int modo = 0; modo < 3; ++modo) {
        ArbolAVL t;
        const int n = 4000;
        for (int i = 0; i < n; ++i) {
            const int k = modo == 0 ? i : (modo == 1 ? n - i : (i % 2 == 0 ? i : n - i));
            CHECK(t.insertar(k, "id"));
            if (i % 500 == 0) CHECK(t.verificar_invariantes());
        }
        CHECK(t.verificar_invariantes());
        CHECK(t.tamano() == static_cast<std::size_t>(n));
        const double cota = 1.4405 * std::log2(static_cast<double>(n) + 2.0);
        CHECK(static_cast<double>(t.altura()) <= cota + 1.0);
        for (int i = 0; i < n; i += 2) CHECK(t.eliminar(modo == 0 ? i : (modo == 1 ? n - i : i), "id") || modo == 2);
        CHECK(t.verificar_invariantes());
    }
}

void pruebas_avl_vs_oraculo() {
    Generador g(11);
    ArbolAVL t;
    std::vector<std::pair<Puntos, std::string>> oraculo;  // ordenado por antes_en_ranking
    const auto cmp = [](const std::pair<Puntos, std::string>& x, const std::pair<Puntos, std::string>& y) {
        return antes_en_ranking(x.first, x.second, y.first, y.second);
    };
    for (int paso = 0; paso < 6000; ++paso) {
        const Puntos p = static_cast<Puntos>(g.entero(0, 15));
        const std::string id = "k" + std::to_string(g.entero(0, 15));
        const auto clave = std::make_pair(p, id);
        const auto it = std::lower_bound(oraculo.begin(), oraculo.end(), clave, cmp);
        const bool existe = it != oraculo.end() && it->first == p && it->second == id;
        if (g.prob(55)) {
            CHECK(t.insertar(p, id) == !existe);
            if (!existe) oraculo.insert(it, clave);
        } else {
            CHECK(t.eliminar(p, id) == existe);
            if (existe) oraculo.erase(it);
        }
        CHECK(t.tamano() == oraculo.size());
        CHECK(t.verificar_invariantes());
        if (paso % 30 == 0) {
            for (std::size_t i = 0; i < oraculo.size(); ++i)
                CHECK(t.posicion(oraculo[i].first, oraculo[i].second) == std::optional<std::size_t>(i));
            for (std::size_t k : {std::size_t{0}, std::size_t{1}, std::size_t{3}, oraculo.size(),
                                  oraculo.size() + 4}) {
                std::vector<std::pair<Puntos, std::string>> obtenido;
                t.primeros(k, [&obtenido](Puntos pp, const std::string& ii) {
                    obtenido.emplace_back(pp, ii);
                });
                const std::size_t esperado = std::min(k, oraculo.size());
                CHECK(obtenido.size() == esperado);
                CHECK(std::equal(obtenido.begin(), obtenido.end(), oraculo.begin()));
            }
        }
    }
}

// ----------------------------------------------------- candidatos de ranking
template <typename R>
void pruebas_ranking(const char* nombre) {
    std::cout << "  ranking " << nombre << "\n";
    {
        R r;
        CHECK(r.tamano() == 0);
        CHECK(r.top_k(0).empty());
        CHECK(r.top_k(5).empty());
        CHECK(!r.posicion(0).has_value());
        CHECK(r.verificar_invariantes());
        CHECK(lanza_invalid_argument([&r] { r.agregar(3, "x"); }));  // slot no consecutivo
    }
    {
        R r;
        r.agregar(0, "c");
        r.agregar(1, "a");
        r.agregar(2, "b");
        // Todos con 0 puntos: desempate por id.
        auto t = r.top_k(10);
        CHECK(t.size() == 3);
        CHECK(t[0].id == "a" && t[1].id == "b" && t[2].id == "c");
        CHECK(t[0].posicion == 1 && t[2].posicion == 3);
        r.fijar_puntos(0, 7);  // "c" sube al primero
        CHECK(r.top_k(1)[0].id == "c");
        CHECK(r.posicion(0) == std::optional<std::size_t>(1));
        r.fijar_puntos(0, 0);  // vuelve a empatar
        CHECK(r.top_k(3)[2].id == "c");
        r.fijar_puntos(1, 5);
        r.fijar_puntos(2, 5);  // empate a 5: a antes que b
        CHECK(r.top_k(2)[0].id == "a" && r.top_k(2)[1].id == "b");
        r.fijar_puntos(2, 5);  // sin cambio
        CHECK(r.verificar_invariantes());
        CHECK(lanza_invalid_argument([&r] { r.cargar({"q"}, {1}); }));  // no vacio
    }
    {
        // cargar == agregar + fijar_puntos
        Generador g(5);
        std::vector<std::string> ids;
        std::vector<Puntos> puntos;
        for (int i = 0; i < 200; ++i) {
            ids.push_back(id_luchador(static_cast<std::uint64_t>(i)));
            puntos.push_back(static_cast<Puntos>(g.entero(0, 9)));
        }
        R a, b;
        a.cargar(ids, puntos);
        for (std::size_t i = 0; i < ids.size(); ++i) {
            b.agregar(i, ids[i]);
            b.fijar_puntos(i, puntos[i]);
        }
        CHECK(a.top_k(300) == b.top_k(300));
        CHECK(a.verificar_invariantes() && b.verificar_invariantes());
        for (std::size_t i = 0; i < ids.size(); ++i) CHECK(a.posicion(i) == b.posicion(i));
        CHECK(lanza_invalid_argument([] { R x; x.cargar({"a", "b"}, {1}); }));
    }
    {
        // Contra un oraculo con ordenamiento completo tras cada tanda de cambios.
        Generador g(23);
        const std::size_t n = 120;
        R r;
        std::vector<Puntos> p(n, 0);
        for (std::size_t i = 0; i < n; ++i) r.agregar(i, id_luchador(i));
        for (int paso = 0; paso < 8000; ++paso) {
            const std::size_t s = g.entero(0, n - 1);
            const Puntos nuevo = static_cast<Puntos>(g.entero(0, 9));  // muchos empates
            r.fijar_puntos(s, nuevo);
            p[s] = nuevo;
            if (paso % 100 == 0) {
                std::vector<std::size_t> orden(n);
                for (std::size_t i = 0; i < n; ++i) orden[i] = i;
                std::sort(orden.begin(), orden.end(), [&p](std::size_t x, std::size_t y) {
                    return antes_en_ranking(p[x], id_luchador(x), p[y], id_luchador(y));
                });
                const auto top = r.top_k(n + 2);
                CHECK(top.size() == n);
                for (std::size_t i = 0; i < n; ++i) {
                    CHECK(top[i].id == id_luchador(orden[i]));
                    CHECK(top[i].puntos == p[orden[i]]);
                    CHECK(r.posicion(orden[i]) == std::optional<std::size_t>(i + 1));
                }
                CHECK(r.verificar_invariantes());
            }
        }
    }
}

// --------------------------------------------------------------- historial
EventoCombate evento(std::uint64_t id) {
    return EventoCombate{id, ResultadoCombate{"a" + std::to_string(id), "b", "b", id}};
}

template <typename H>
void pruebas_historial(const char* nombre) {
    std::cout << "  historial " << nombre << "\n";
    CHECK(lanza_invalid_argument([] { H h(0); }));
    {
        H h(3);
        CHECK(h.tamano() == 0 && h.capacidad() == 3);
        CHECK(h.ultimos(5).empty());
        CHECK(!h.anular(1).has_value());
        CHECK(lanza_invalid_argument([&h] { h.registrar(evento(0)); }));  // id 0 no valido
        for (std::uint64_t i = 1; i <= 5; ++i) {
            h.registrar(evento(i));
            CHECK(h.verificar_invariantes());
        }
        CHECK(h.tamano() == 3);
        CHECK(lanza_invalid_argument([&h] { h.registrar(evento(5)); }));  // repetido
        CHECK(lanza_invalid_argument([&h] { h.registrar(evento(2)); }));  // no creciente
        const auto u = h.ultimos(10);
        CHECK(u.size() == 3 && u[0].id_combate == 5 && u[1].id_combate == 4 && u[2].id_combate == 3);
        CHECK(h.ultimos(0).empty());
        CHECK(h.ultimos(2).size() == 2);
        CHECK(!h.anular(1).has_value());  // expulsado
        CHECK(!h.anular(2).has_value());  // expulsado
        const auto q = h.anular(4);
        CHECK(q.has_value() && q->id_combate == 4 && q->resultado.turnos == 4);
        CHECK(!h.anular(4).has_value());  // ya anulado
        CHECK(h.tamano() == 2 && h.verificar_invariantes());
        h.registrar(evento(6));           // hay hueco: no expulsa
        CHECK(h.tamano() == 3 && h.ultimos(3)[2].id_combate == 3);
        h.registrar(evento(7));           // ahora si expulsa el 3
        const auto v = h.ultimos(5);
        CHECK(v.size() == 3 && v[0].id_combate == 7 && v[1].id_combate == 6 && v[2].id_combate == 5);
        CHECK(h.anular(5).has_value());   // el mas antiguo
        CHECK(h.anular(7).has_value());   // el mas reciente
        CHECK(h.ultimos(5).size() == 1 && h.ultimos(5)[0].id_combate == 6);
        CHECK(h.verificar_invariantes());
    }
    {
        H h(1);  // ventana minima
        h.registrar(evento(1));
        h.registrar(evento(2));
        CHECK(h.tamano() == 1 && h.ultimos(3)[0].id_combate == 2);
        CHECK(h.anular(2).has_value());
        CHECK(h.tamano() == 0 && h.verificar_invariantes());
        h.registrar(evento(3));
        CHECK(h.ultimos(1)[0].id_combate == 3);
    }
    for (std::size_t cap : {std::size_t{7}, std::size_t{50}}) {
        // Contra un modelo con std::vector, 10.000 operaciones.
        Generador g(static_cast<std::uint32_t>(100 + cap));
        H h(cap);
        std::vector<EventoCombate> modelo;
        std::uint64_t ultimo = 0;
        for (int paso = 0; paso < 10000; ++paso) {
            const auto t = g.entero(0, 99);
            if (t < 45) {
                const EventoCombate e = evento(++ultimo);
                h.registrar(e);
                modelo.push_back(e);
                if (modelo.size() > cap) modelo.erase(modelo.begin());
            } else if (t < 75) {
                const std::uint64_t id = g.entero(0, ultimo + 2);
                const auto r = h.anular(id);
                const auto m = std::find_if(modelo.begin(), modelo.end(),
                                            [id](const EventoCombate& e) { return e.id_combate == id; });
                CHECK(r.has_value() == (m != modelo.end()));
                if (m != modelo.end()) {
                    CHECK(*r == *m);
                    modelo.erase(m);
                }
            } else {
                const std::size_t k = g.entero(0, cap + 3);
                std::vector<EventoCombate> esperado;
                for (std::size_t i = modelo.size(); i > 0 && esperado.size() < k; --i)
                    esperado.push_back(modelo[i - 1]);
                CHECK(h.ultimos(k) == esperado);
            }
            CHECK(h.tamano() == modelo.size());
            if (paso % 50 == 0) CHECK(h.verificar_invariantes());
            ++g_ops_diferenciales;
        }
        CHECK(h.verificar_invariantes());
    }
}

// ------------------------------------------------------ motor: casos borde
template <typename M>
std::string firma_estado(const M& m, std::size_t pool, std::size_t cap) {
    std::string s;
    for (const auto& e : m.top_k(pool + 5))
        s += std::to_string(e.posicion) + "," + e.id + "," + std::to_string(e.puntos) + ";";
    s += "|";
    for (std::size_t i = 0; i < pool + 2; ++i) {
        const std::string id = i < pool ? id_luchador(i) : "X" + std::to_string(i);
        const auto st = m.estadisticas(id);
        const auto po = m.posicion(id);
        s += po ? std::to_string(*po) : std::string("-");
        if (st)
            s += ":" + std::to_string(st->combates) + "," + std::to_string(st->victorias) + "," +
                 std::to_string(st->derrotas) + "," + std::to_string(st->empates) + "," +
                 std::to_string(st->turnos_totales) + "," + std::to_string(st->puntos);
        s += ";";
    }
    s += "|";
    for (const auto& e : m.ultimos(cap + 1))
        s += std::to_string(e.id_combate) + "," + e.resultado.id_a + "," + e.resultado.id_b + "," +
             e.resultado.id_ganador + "," + std::to_string(e.resultado.turnos) + "," +
             std::to_string(e.resultado.id_externo) + ";";
    return s;
}

template <typename M>
void casos_borde_motor(const char* nombre) {
    std::cout << "  motor " << nombre << "\n";
    const auto res = [](const char* a, const char* b, const char* g, std::uint64_t t) {
        return ResultadoCombate{a, b, g, t};
    };
    {   // Motor vacio
        M m(4);
        CHECK(m.luchadores() == 0);
        CHECK(m.top_k(0).empty() && m.top_k(5).empty());
        CHECK(!m.posicion("x").has_value());
        CHECK(!m.estadisticas("x").has_value());
        CHECK(m.ultimos(3).empty() && m.ultimos(0).empty());
        CHECK(!m.anular_resultado(1) && !m.anular_resultado(0));
        const auto r = m.registrar_resultado(res("a", "b", "a", 3));
        CHECK(r.estado == EstadoRegistro::id_desconocido && r.id_combate == 0);
        CHECK(m.verificar_invariantes());
        CHECK(lanza_invalid_argument([] { M x(0); }));  // capacidad 0
    }
    {   // Registro de luchadores
        M m(4);
        CHECK(!m.registrar_luchador(""));
        CHECK(m.registrar_luchador("A"));
        CHECK(!m.registrar_luchador("A"));
        CHECK(m.luchadores() == 1);
        CHECK(m.posicion("A") == std::optional<std::size_t>(1));
        CHECK(m.estadisticas("A") == std::optional<Estadisticas>(Estadisticas{}));
    }
    {   // Validaciones: ninguna rechazada cambia el estado
        M m(4);
        for (const char* id : {"A", "B", "C"}) CHECK(m.registrar_luchador(id));
        const std::string antes = firma_estado(m, 0, 4);
        CHECK(m.registrar_resultado(res("A", "A", "A", 1)).estado == EstadoRegistro::ids_iguales);
        CHECK(m.registrar_resultado(res("A", "Q", "A", 1)).estado == EstadoRegistro::id_desconocido);
        CHECK(m.registrar_resultado(res("Q", "A", "A", 1)).estado == EstadoRegistro::id_desconocido);
        CHECK(m.registrar_resultado(res("", "", "", 1)).estado == EstadoRegistro::ids_iguales);
        CHECK(m.registrar_resultado(res("A", "B", "C", 1)).estado == EstadoRegistro::ganador_invalido);
        CHECK(m.registrar_resultado(res("A", "B", "A", 0)).estado == EstadoRegistro::turnos_invalidos);
        CHECK(m.registrar_resultado(res("A", "B", "A", kMaxTurnosPorCombate + 1)).estado ==
              EstadoRegistro::turnos_invalidos);
        CHECK(firma_estado(m, 0, 4) == antes);
        CHECK(m.ultimos(10).empty());
        CHECK(m.verificar_invariantes());
        const auto ok = m.registrar_resultado(res("A", "B", "A", kMaxTurnosPorCombate));  // limite valido
        CHECK(ok.estado == EstadoRegistro::registrado && ok.id_combate == 1);
    }
    {   // Puntajes, empates y desempate por id
        M m(10);
        for (const char* id : {"E", "D", "C", "B", "A"}) CHECK(m.registrar_luchador(id));
        CHECK(m.registrar_resultado(res("A", "B", "A", 5)).id_combate == 1);  // A=3
        CHECK(m.registrar_resultado(res("C", "D", "", 7)).id_combate == 2);   // C=1 D=1 empate
        const auto t = m.top_k(10);
        CHECK(t.size() == 5);
        CHECK(t[0].id == "A" && t[0].puntos == 3);
        CHECK(t[1].id == "C" && t[2].id == "D");  // empate de puntos: C antes que D
        CHECK(t[3].id == "B" && t[4].id == "E");  // 0 puntos: B antes que E
        CHECK(m.posicion("E") == std::optional<std::size_t>(5));
        const auto sa = *m.estadisticas("A");
        CHECK(sa.combates == 1 && sa.victorias == 1 && sa.puntos == 3 && sa.turnos_totales == 5);
        CHECK(m.top_k(0).empty() && m.top_k(2).size() == 2);
        CHECK(m.verificar_invariantes());
        // La actualizacion de puntajes reordena: B gana dos veces.
        CHECK(m.registrar_resultado(res("B", "E", "B", 2)).id_combate == 3);
        CHECK(m.registrar_resultado(res("B", "D", "B", 2)).id_combate == 4);
        CHECK(m.top_k(1)[0].id == "B" && m.top_k(1)[0].puntos == 6);
        CHECK(m.verificar_invariantes());
    }
    {   // Anulacion revierte; doble anulacion y ids fuera de ventana
        M m(3);
        for (const char* id : {"A", "B", "C"}) CHECK(m.registrar_luchador(id));
        const std::string vacio = firma_estado(m, 0, 3);
        CHECK(m.registrar_resultado(res("A", "B", "B", 4)).id_combate == 1);
        CHECK(m.anular_resultado(1));
        CHECK(!m.anular_resultado(1));
        CHECK(firma_estado(m, 0, 3) == vacio);
        // Tras anular y volver a registrar, el id sigue creciendo.
        CHECK(m.registrar_resultado(res("A", "B", "A", 4)).id_combate == 2);
        for (int i = 0; i < 3; ++i) CHECK(m.registrar_resultado(res("B", "C", "B", 1)).estado == EstadoRegistro::registrado);
        // ventana de 3: el combate 2 ya fue expulsado y no se puede anular.
        CHECK(!m.anular_resultado(2));
        CHECK(m.estadisticas("A")->victorias == 1);
        CHECK(m.ultimos(10).size() == 3 && m.ultimos(10)[0].id_combate == 5);
        CHECK(m.anular_resultado(4) && m.verificar_invariantes());
        CHECK(!m.anular_resultado(0));
        CHECK(!m.anular_resultado(999));
    }
    {   // Acumulacion de turnos mas alla de 32 bits y reversion exacta
        M m(10);
        CHECK(m.registrar_luchador("A") && m.registrar_luchador("B"));
        for (int i = 0; i < 5; ++i)
            CHECK(m.registrar_resultado(res("A", "B", "A", kMaxTurnosPorCombate)).estado ==
                  EstadoRegistro::registrado);
        CHECK(m.estadisticas("A")->turnos_totales == 5ULL * kMaxTurnosPorCombate);
        CHECK(m.anular_resultado(3));
        CHECK(m.estadisticas("B")->turnos_totales == 4ULL * kMaxTurnosPorCombate);
        CHECK(m.verificar_invariantes());
    }
    {   // Idempotencia: la clave externa evita contar dos veces el mismo combate
        M m(5);
        for (const char* id : {"A", "B", "C"}) CHECK(m.registrar_luchador(id));
        ResultadoCombate r = res("A", "B", "A", 7);
        r.id_externo = 42;
        const auto primero = m.registrar_resultado(r);
        CHECK(primero.estado == EstadoRegistro::registrado && primero.id_combate == 1);
        const std::string tras_primero = firma_estado(m, 0, 5);
        const auto repetido = m.registrar_resultado(r);  // mismo contenido
        CHECK(repetido.estado == EstadoRegistro::duplicado && repetido.id_combate == 1);
        CHECK(firma_estado(m, 0, 5) == tras_primero);    // no se conto dos veces
        ResultadoCombate otro = r;
        otro.turnos = 8;                                  // misma clave, otro contenido
        CHECK(m.registrar_resultado(otro).estado == EstadoRegistro::conflicto_clave);
        otro = r;
        otro.id_ganador = "B";
        CHECK(m.registrar_resultado(otro).estado == EstadoRegistro::conflicto_clave);
        CHECK(firma_estado(m, 0, 5) == tras_primero);
        // Los datos invalidos se rechazan como invalidos aunque la clave exista.
        ResultadoCombate malo = r;
        malo.turnos = 0;
        CHECK(m.registrar_resultado(malo).estado == EstadoRegistro::turnos_invalidos);
        // Clave 0 = sin clave: no hay idempotencia y cada registro cuenta.
        ResultadoCombate sin = res("A", "C", "C", 3);
        CHECK(m.registrar_resultado(sin).estado == EstadoRegistro::registrado);
        CHECK(m.registrar_resultado(sin).estado == EstadoRegistro::registrado);
        CHECK(m.estadisticas("C")->victorias == 2);
        // Anular libera la clave: se puede registrar una correccion con la misma clave.
        CHECK(m.anular_resultado(1));
        ResultadoCombate corregido = res("A", "B", "B", 7);
        corregido.id_externo = 42;
        const auto nuevo = m.registrar_resultado(corregido);
        CHECK(nuevo.estado == EstadoRegistro::registrado && nuevo.id_combate == 4);
        CHECK(m.estadisticas("B")->victorias == 1 && m.estadisticas("A")->victorias == 0);
        CHECK(m.registrar_resultado(corregido).estado == EstadoRegistro::duplicado);
        CHECK(m.verificar_invariantes());
    }
    {   // Un resultado fuera de la ventana sigue siendo duplicado (la clave no se expulsa)
        M m(1);
        CHECK(m.registrar_luchador("A") && m.registrar_luchador("B"));
        ResultadoCombate r1 = res("A", "B", "A", 2);
        r1.id_externo = 7;
        CHECK(m.registrar_resultado(r1).estado == EstadoRegistro::registrado);
        CHECK(m.registrar_resultado(res("A", "B", "B", 2)).estado == EstadoRegistro::registrado);  // expulsa r1
        CHECK(m.ultimos(5).size() == 1);
        CHECK(m.registrar_resultado(r1).estado == EstadoRegistro::duplicado);
        CHECK(!m.anular_resultado(1));  // fuera de ventana: no se puede anular
        CHECK(m.estadisticas("A")->combates == 2);
        CHECK(m.verificar_invariantes());
    }
    {   // Ids reales del juego (con caracteres no ASCII), todos contra todos
        M m(20);
        const char* ids[] = {"Sexitar", "Fran\xC3\xA7ois", "Leo1", "Leo2"};
        for (const char* id : ids) CHECK(m.registrar_luchador(id));
        for (int i = 0; i < 4; ++i)
            for (int j = i + 1; j < 4; ++j)
                CHECK(m.registrar_resultado(res(ids[i], ids[j], ids[(i + j) % 2 ? i : j], 9)).estado ==
                      EstadoRegistro::registrado);
        CHECK(m.top_k(4).size() == 4 && m.verificar_invariantes());
        CHECK(m.estadisticas("Fran\xC3\xA7ois").has_value());
    }
}

// ----------------------------------------------------- comparacion diferencial
struct Salida {
    std::string firma;
    std::string categoria;
    std::uint64_t id_aceptado = 0;
    bool luchador_ok = false;
};

const char* nombre_estado(EstadoRegistro e) {
    switch (e) {
        case EstadoRegistro::registrado: return "registrado";
        case EstadoRegistro::ids_iguales: return "ids_iguales";
        case EstadoRegistro::id_desconocido: return "id_desconocido";
        case EstadoRegistro::ganador_invalido: return "ganador_invalido";
        case EstadoRegistro::turnos_invalidos: return "turnos_invalidos";
        case EstadoRegistro::duplicado: return "duplicado";
        case EstadoRegistro::conflicto_clave: return "conflicto_clave";
    }
    return "?";
}

template <typename M>
Salida ejecutar(M& m, const Op& op) {
    Salida s;
    switch (op.tipo) {
        case TipoOp::luchador: {
            const bool ok = m.registrar_luchador(op.id);
            s.firma = std::string("L:") + (ok ? "1" : "0");
            s.categoria = ok ? "luchador/ok" : "luchador/rechazado";
            s.luchador_ok = ok;
            break;
        }
        case TipoOp::resultado: {
            const auto r = m.registrar_resultado(op.res);
            s.firma = "R:" + std::to_string(static_cast<int>(r.estado)) + ":" + std::to_string(r.id_combate);
            s.categoria = std::string("resultado/") + nombre_estado(r.estado);
            if (r.estado == EstadoRegistro::registrado) s.id_aceptado = r.id_combate;
            break;
        }
        case TipoOp::anular: {
            const bool ok = m.anular_resultado(op.numero);
            s.firma = std::string("A:") + (ok ? "1" : "0");
            s.categoria = ok ? "anular/ok" : "anular/falso";
            break;
        }
        case TipoOp::top_k: {
            const auto v = m.top_k(static_cast<std::size_t>(op.numero));
            s.firma = "T:";
            for (const auto& e : v)
                s.firma += std::to_string(e.posicion) + "," + e.id + "," + std::to_string(e.puntos) + ";";
            s.categoria = v.empty() ? "top_k/vacio" : "top_k/datos";
            break;
        }
        case TipoOp::posicion: {
            const auto p = m.posicion(op.id);
            s.firma = "P:" + (p ? std::to_string(*p) : std::string("-"));
            s.categoria = p ? "posicion/presente" : "posicion/ausente";
            break;
        }
        case TipoOp::estadisticas: {
            const auto st = m.estadisticas(op.id);
            s.firma = "S:";
            if (st)
                s.firma += std::to_string(st->combates) + "," + std::to_string(st->victorias) + "," +
                           std::to_string(st->derrotas) + "," + std::to_string(st->empates) + "," +
                           std::to_string(st->turnos_totales) + "," + std::to_string(st->puntos);
            else
                s.firma += "-";
            s.categoria = st ? "estadisticas/presente" : "estadisticas/ausente";
            break;
        }
        case TipoOp::ultimos: {
            const auto u = m.ultimos(static_cast<std::size_t>(op.numero));
            s.firma = "U:";
            for (const auto& e : u)
                s.firma += std::to_string(e.id_combate) + "," + e.resultado.id_a + "," +
                           e.resultado.id_b + "," + e.resultado.id_ganador + "," +
                           std::to_string(e.resultado.turnos) + "," +
                           std::to_string(e.resultado.id_externo) + ";";
            s.categoria = u.empty() ? "ultimos/vacio" : "ultimos/datos";
            break;
        }
    }
    return s;
}

struct Escenario {
    const char* nombre;
    PerfilEscenario perfil;
    std::size_t capacidad;
    std::uint32_t semilla;
    std::size_t n_ops;
};

template <typename Rapido>
void diferencial(const char* combo, const Escenario& e, bool invariantes_siempre) {
    Rapido rapido(e.capacidad);
    MotorLento lento(e.capacidad);
    Generador g(e.semilla);
    ContextoGen ctx;
    std::map<std::string, long long> cat;
    bool divergio = false;
    for (std::size_t i = 0; i < e.n_ops && !divergio; ++i) {
        const Op op = siguiente_op(g, e.perfil, ctx);
        const Salida a = ejecutar(rapido, op);
        const Salida b = ejecutar(lento, op);
        ++g_ops_diferenciales;
        ++cat[b.categoria];
        if (a.firma != b.firma) {
            divergio = true;
            std::cerr << "DIVERGENCIA [" << combo << " / " << e.nombre << " semilla " << e.semilla
                      << "] en la operacion " << i << "\n  rapido: " << a.firma << "\n  lento : "
                      << b.firma << "\n";
            ++g_fallos;
            break;
        }
        if (op.tipo == TipoOp::resultado && op.res.id_externo > ctx.ultima_clave)
            ctx.ultima_clave = op.res.id_externo;
        if (b.id_aceptado != 0) {
            ctx.combates_aceptados.push_back(b.id_aceptado);
            ctx.resultados_aceptados.push_back(op.res);
        }
        if (b.luchador_ok) ++ctx.luchadores_registrados;
        if (invariantes_siempre || i % 97 == 0) {
            CHECK(rapido.verificar_invariantes());
            const std::size_t pool = e.perfil.pool_luchadores;
            CHECK(firma_estado(rapido, pool, e.capacidad) == firma_estado(lento, pool, e.capacidad));
        }
    }
    CHECK(rapido.verificar_invariantes());
    CHECK(firma_estado(rapido, e.perfil.pool_luchadores, e.capacidad) ==
          firma_estado(lento, e.perfil.pool_luchadores, e.capacidad));
    for (const auto& kv : cat) g_cobertura[kv.first] += kv.second;
}

const Escenario kEscenarios[] = {
    // mezcla: luchador, resultado, anular, top_k, posicion, estadisticas, ultimos
    {"mixto", {40, {10, 45, 8, 12, 12, 8, 5}}, 25, 1, 10000},
    {"mixto", {40, {10, 45, 8, 12, 12, 8, 5}}, 25, 2, 10000},
    {"muchos-empates", {6, {8, 60, 10, 10, 6, 3, 3}}, 8, 3, 10000},
    {"ventana-minima", {12, {5, 45, 25, 8, 7, 5, 5}}, 1, 4, 10000},
    {"ventana-3-anulaciones", {12, {5, 45, 25, 8, 7, 5, 5}}, 3, 5, 10000},
    {"consulta-pesada", {80, {10, 30, 5, 25, 15, 10, 5}}, 100, 6, 10000},
};


// Combate simulado con la misma interfaz que Combate (I2): no depende de I2.
struct PersonajeFalso { bool vivo = true; };
struct CombateFalso {
    PersonajeFalso* a;
    PersonajeFalso* b;
    unsigned long turnos = 0;
    bool terminado() const { return !a->vivo || !b->vivo; }
    const PersonajeFalso* ganador() const {
        if (a->vivo == b->vivo) return nullptr;
        return a->vivo ? a : b;
    }
    unsigned long turnosEjecutados() const { return turnos; }
};

void pruebas_adaptador() {
    PersonajeFalso a, b, otro;
    CombateFalso c{&a, &b, 7};
    // No terminado: nada que registrar (ganador() tambien es nullptr aqui).
    CHECK(!resultado_desde_combate(c, a, b, "goku", "vegeta").has_value());
    b.vivo = false;
    auto r = resultado_desde_combate(c, a, b, "goku", "vegeta", 42);
    CHECK(r.has_value());
    CHECK(r->id_ganador == "goku" && r->id_a == "goku" && r->id_b == "vegeta");
    CHECK(r->turnos == 7 && r->id_externo == 42);
    a.vivo = true; b.vivo = true; a.vivo = false;
    r = resultado_desde_combate(c, a, b, "goku", "vegeta");
    CHECK(r.has_value() && r->id_ganador == "vegeta");
    b.vivo = false;  // ambos caidos: empate
    r = resultado_desde_combate(c, a, b, "goku", "vegeta");
    CHECK(r.has_value() && r->id_ganador.empty());
    // Ganador que no es ninguno de los dos: el motor lo rechaza.
    struct Raro : CombateFalso {
        PersonajeFalso* x;
        const PersonajeFalso* ganador() const { return x; }
    } raro{{&a, &b, 1}, &otro};
    r = resultado_desde_combate(raro, a, b, "goku", "vegeta");
    using M = MotorResultados<RankingAVL, HistorialCircular>;
    M m(8);
    m.registrar_luchador("goku");
    m.registrar_luchador("vegeta");
    CHECK(r.has_value() && m.registrar_resultado(*r).estado == EstadoRegistro::ganador_invalido);
    // Flujo completo con idempotencia.
    a.vivo = true; b.vivo = false;
    r = resultado_desde_combate(c, a, b, "goku", "vegeta", 9);
    const auto p1 = m.registrar_resultado(*r);
    const auto p2 = m.registrar_resultado(*r);
    CHECK(p1.estado == EstadoRegistro::registrado);
    CHECK(p2.estado == EstadoRegistro::duplicado && p2.id_combate == p1.id_combate);
    CHECK(m.estadisticas("goku")->victorias == 1);
    CHECK(m.verificar_invariantes());
}

template <typename R, typename H, typename I = IndiceStd>
void correr_combinacion(const char* nombre) {
    std::cout << "  diferencial " << nombre << "\n";
    using M = MotorResultados<R, H, I>;
    casos_borde_motor<M>(nombre);
    for (const auto& e : kEscenarios) diferencial<M>(nombre, e, e.n_ops == 10000 && e.semilla == 1);
}

}  // namespace

int main() {
    std::cout << "== AVL ==\n";
    pruebas_avl_basicas();
    pruebas_avl_degeneradas();
    pruebas_avl_vs_oraculo();

    std::cout << "== Candidatos de ranking ==\n";
    pruebas_ranking<RankingAVL>("AVL");
    pruebas_ranking<RankingHeapSelect>("HeapSelect");
    pruebas_ranking<RankingArreglo>("Arreglo");

    std::cout << "== Candidatos de historial ==\n";
    pruebas_historial<HistorialCircular>("Circular");
    pruebas_historial<HistorialLista>("Lista");

    std::cout << "== Adaptador de combate ==\n";
    pruebas_adaptador();

    std::cout << "== Motor completo y comparacion con la referencia lenta ==\n";
    correr_combinacion<RankingAVL, HistorialCircular>("AVL+Circular");
    correr_combinacion<RankingAVL, HistorialLista>("AVL+Lista");
    correr_combinacion<RankingHeapSelect, HistorialCircular>("HeapSelect+Circular");
    correr_combinacion<RankingHeapSelect, HistorialLista>("HeapSelect+Lista");
    correr_combinacion<RankingArreglo, HistorialCircular>("Arreglo+Circular");
    correr_combinacion<RankingArreglo, HistorialLista>("Arreglo+Lista");
#ifdef COMBAT_I3_HAY_HASH_I1
    std::cout << "== Adaptador de la hash table de I1 ==\n";
    correr_combinacion<RankingAVL, HistorialCircular, IndiceHashI1>("AVL+Circular+HashI1");
#else
    std::cout << "(hash table de I1 no disponible en esta rama: adaptador IndiceHashI1 omitido)\n";
#endif

    // La comparacion no puede ser vacua: cada categoria relevante debe aparecer.
    for (const char* c : {"resultado/registrado", "resultado/id_desconocido", "resultado/ids_iguales",
                          "resultado/ganador_invalido", "resultado/turnos_invalidos", "resultado/duplicado",
                          "resultado/conflicto_clave", "anular/ok",
                          "anular/falso", "luchador/rechazado", "top_k/datos", "top_k/vacio",
                          "posicion/ausente", "posicion/presente", "estadisticas/ausente",
                          "ultimos/datos"})
        CHECK(g_cobertura[c] > 50);

    std::cout << "\nCobertura de la comparacion diferencial (suma de todas las combinaciones):\n";
    for (const auto& kv : g_cobertura) std::cout << "  " << kv.first << ": " << kv.second << "\n";
    std::cout << "\nOperaciones comparadas contra la referencia: " << g_ops_diferenciales << "\n";
    std::cout << "Comprobaciones: " << g_comprobaciones << ", fallos: " << g_fallos << "\n";
    return g_fallos == 0 ? 0 : 1;
}
