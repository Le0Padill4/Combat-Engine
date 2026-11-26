#include "i1/hash_table.h"
#include "i1/perfil_personaje.h"
#include "i1/registro_personajes.h"
#include "soporte/registro_lento.h"
#include "personaje.h"

#include <climits>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace combat::i1;

void exigir(bool condicion, const std::string& mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}

PerfilPersonaje perfil(const std::string& id, int variante = 0) {
    return {id, "Ficticio " + id, {100 + variante, 80, 18, 10},
            {"Golpe", "Especial", 25}};
}

struct HashConstante {
    std::size_t operator()(const std::string&) const { return 0; }
};

struct ValorConFallo {
    inline static int copias_disponibles = -1;
    int dato;
    explicit ValorConFallo(int valor) : dato(valor) {}
    ValorConFallo(const ValorConFallo& otro) : dato(otro.dato) {
        if (copias_disponibles == 0) throw std::runtime_error("copia simulada");
        if (copias_disponibles > 0) --copias_disponibles;
    }
    ValorConFallo(ValorConFallo&&) noexcept = default;
};

void fallo_durante_rehash() {
    HashTable<std::string, ValorConFallo> tabla(2);
    exigir(tabla.insertar("a", ValorConFallo(7)), "preparacion de fallo");
    const auto capacidad = tabla.capacidad();
    ValorConFallo::copias_disponibles = 1;
    bool fallo = false;
    try { tabla.insertar("b", ValorConFallo(9)); }
    catch (const std::runtime_error&) { fallo = true; }
    ValorConFallo::copias_disponibles = -1;
    exigir(fallo && tabla.tamano() == 1 && tabla.capacidad() == capacidad &&
               tabla.buscar("a")->dato == 7 && !tabla.buscar("b") &&
               tabla.verificar_invariantes(), "fallo de rehash dejo cambios parciales");
    exigir(tabla.insertar("b", ValorConFallo(9)), "tabla inutilizable tras excepcion");
}

void validar_perfiles() {
    const auto original = perfil("personaje-1");
    exigir(!perfil_valido(PerfilPersonaje{}), "perfil por defecto valido");
    exigir(perfil_valido(original), "perfil valido rechazado");
    std::vector<PerfilPersonaje> invalidos;
    auto agregar = [&](const std::function<void(PerfilPersonaje&)>& cambiar) {
        auto copia = original;
        cambiar(copia);
        invalidos.push_back(copia);
    };
    agregar([](auto& p) { p.id.clear(); });
    agregar([](auto& p) { p.id = " \t\n"; });
    agregar([](auto& p) { p.alias = " "; });
    agregar([](auto& p) { p.atributos.hp_max = 0; });
    agregar([](auto& p) { p.atributos.ki_max = -1; });
    agregar([](auto& p) { p.atributos.ataque = 0; });
    agregar([](auto& p) { p.atributos.defensa = -1; });
    agregar([](auto& p) { p.habilidades.normal.clear(); });
    agregar([](auto& p) { p.habilidades.especial = "\t"; });
    agregar([](auto& p) { p.habilidades.costo_ki_especial = -1; });
    for (const auto& p : invalidos)
        exigir(!perfil_valido(p), "perfil invalido aceptado");

    auto borde = original;
    borde.atributos = {1, 0, 1, 0};
    borde.habilidades.costo_ki_especial = 0;
    exigir(perfil_valido(borde), "limites inferiores validos rechazados");
    borde.atributos = {INT_MAX, INT_MAX, INT_MAX, INT_MAX};
    borde.habilidades.costo_ki_especial = INT_MAX;
    exigir(perfil_valido(borde), "registro impuso un limite no contratado");
}

void colisiones_y_rehash() {
    HashTable<std::string, int, HashConstante> tabla(1);
    exigir(tabla.tamano() == 0 && tabla.verificar_invariantes(), "tabla vacia");
    exigir(tabla.buscar("ausente") == nullptr, "busqueda vacia");
    const auto capacidad_inicial = tabla.capacidad();
    for (int i = 0; i < 250; ++i) {
        exigir(tabla.insertar("id-" + std::to_string(i), i), "insercion fallida");
        exigir(tabla.verificar_invariantes(), "invariantes tras colision/rehash");
        exigir(tabla.factor_carga() <= 0.75, "factor de carga excedido");
        for (int j = 0; j <= i; ++j) {
            const auto* valor = tabla.buscar("id-" + std::to_string(j));
            exigir(valor && *valor == j, "rehash perdio o altero una entrada");
        }
    }
    exigir(tabla.capacidad() > capacidad_inicial, "no hubo crecimiento");
    const auto capacidad = tabla.capacidad();
    exigir(!tabla.insertar("id-0", 999), "duplicado aceptado");
    exigir(tabla.tamano() == 250 && *tabla.buscar("id-0") == 0,
           "duplicado cambio contenido");
    exigir(tabla.capacidad() == capacidad, "duplicado provoco rehash");
    exigir(tabla.buscar("no-existe") == nullptr, "id ausente encontrado");

    auto copia = tabla;
    exigir(copia.insertar("solo-copia", 8), "copia no independiente");
    exigir(!tabla.buscar("solo-copia") && copia.verificar_invariantes(),
           "copia modifico original");
    HashTable<std::string, int> minima(0);
    exigir(minima.insertar("", 7) && *minima.buscar("") == 7,
           "tabla generica no soporta clave vacia/capacidad cero");
    HashTable<std::string, std::string> alias(2);
    exigir(alias.insertar("a", std::string(200, 'x')), "preparacion de alias");
    const auto* prestado = alias.buscar("a");
    exigir(alias.insertar("b", *prestado), "insercion con referencia propia");
    exigir(*alias.buscar("b") == std::string(200, 'x') && alias.verificar_invariantes(),
           "rehash invalido argumento que apuntaba al valor original");
}

template <typename Registro>
void contrato_registro() {
    Registro registro;
    exigir(registro.tamano() == 0 && !registro.buscar_personaje("ausente"),
           "contrato de registro vacio");
    const auto inicial = perfil("personaje-1");
    exigir(registro.registrar_personaje(inicial) == ResultadoRegistro::registrado,
           "registro valido");
    auto duplicado = inicial;
    duplicado.alias = "Nombre cambiado";
    exigir(registro.registrar_personaje(duplicado) == ResultadoRegistro::id_duplicado,
           "duplicado no rechazado");
    exigir(registro.buscar_personaje(inicial.id) == inicial,
           "duplicado sobrescribio original");
    auto invalido = inicial;
    invalido.atributos.hp_max = 0;
    exigir(registro.registrar_personaje(invalido) == ResultadoRegistro::perfil_invalido,
           "validacion debe preceder a comprobacion de duplicados");
    invalido.id = "nuevo";
    exigir(registro.registrar_personaje(invalido) == ResultadoRegistro::perfil_invalido,
           "registro invalido aceptado");
    exigir(registro.tamano() == 1 && !registro.buscar_personaje("nuevo"),
           "fallo de validacion dejo cambios parciales");

    auto snapshot = registro.buscar_personaje(inicial.id);
    snapshot->alias = "Copia externa";
    exigir(registro.buscar_personaje(inicial.id) == inicial,
           "busqueda expuso estado mutable");
    exigir(registro.registrar_personaje(perfil("Personaje-1")) ==
               ResultadoRegistro::registrado, "ids no respetan mayusculas");
    auto alias_repetido = perfil("otro-id");
    alias_repetido.alias = inicial.alias;
    exigir(registro.registrar_personaje(alias_repetido) ==
               ResultadoRegistro::registrado, "alias repetido debe estar permitido");
    for (int i = 0; i < 1000; ++i)
        exigir(registro.registrar_personaje(perfil(std::to_string(i))) ==
                   ResultadoRegistro::registrado, "insercion masiva");
    exigir(snapshot->alias == "Copia externa", "snapshot invalidado al crecer");
    exigir(registro.verificar_invariantes(), "invariantes del registro");
}

void compatibilidad_personaje() {
    Personaje existente("Ficticio", 120, 80, 18, 20);
    existente.setNataque("Hacer deberes en clases");
    existente.setNataqueEspecial("Al Cesar lo que es del Cesar");
    existente.setKiEspecial(35);
    existente.setHp(12);
    existente.setKi(40);
    existente.setdefiende(true);
    const auto p = perfil_desde_personaje("personaje-1", existente);
    exigir(p == PerfilPersonaje{"personaje-1", "Ficticio", {120, 80, 18, 20},
                               {"Hacer deberes en clases", "Al Cesar lo que es del Cesar", 35}},
           "adaptador altero atributos/habilidades");
    exigir(existente.getHp() == 12 && existente.getKi() == 40 &&
               existente.getDefiende(), "adaptador modifico combate");
    existente.setNombre("Otro alias");
    exigir(p.alias == "Ficticio", "perfil comparte memoria mutable con combate");
}

void diferencial() {
    // Semillas fijas, con consultas ausentes y entradas invalidas/repetidas.
    for (const unsigned semilla : {7U, 42U, 20261125U}) {
        std::mt19937 rng(semilla);
        RegistroPersonajesHash rapido;
        RegistroPersonajesOrdenado ordenado;
        soporte::RegistroPersonajesLento lento;
        std::size_t ausentes = 0, duplicados = 0, invalidos = 0;
        for (int paso = 0; paso < 12000; ++paso) {
            const auto id = "sintetico-" + std::to_string(rng() % 1024);
            if (rng() % 100 < 45) {
                auto p = perfil(id, static_cast<int>(rng() % 30));
                if (rng() % 7 == 0) p.atributos.hp_max = 0;
                const auto esperado = lento.registrar_personaje(p);
                exigir(rapido.registrar_personaje(p) == esperado &&
                           ordenado.registrar_personaje(p) == esperado,
                       "respuesta de registro diferencial, semilla " +
                           std::to_string(semilla) + " paso " + std::to_string(paso));
                duplicados += esperado == ResultadoRegistro::id_duplicado;
                invalidos += esperado == ResultadoRegistro::perfil_invalido;
            } else {
                const auto clave = rng() % 4 == 0 ? "ausente-" + id : id;
                const auto esperado = lento.buscar_personaje(clave);
                exigir(rapido.buscar_personaje(clave) == esperado &&
                           ordenado.buscar_personaje(clave) == esperado,
                       "respuesta de consulta diferencial");
                ausentes += !esperado;
            }
            exigir(rapido.tamano() == lento.tamano() &&
                       ordenado.tamano() == lento.tamano(), "tamano diferencial");
            exigir(rapido.verificar_invariantes() && ordenado.verificar_invariantes(),
                   "invariantes tras operacion aleatoria");
        }
        for (int i = 0; i < 1024; ++i) {
            const auto id = "sintetico-" + std::to_string(i);
            exigir(rapido.buscar_personaje(id) == lento.buscar_personaje(id) &&
                       ordenado.buscar_personaje(id) == lento.buscar_personaje(id),
                   "estado final diferencial");
        }
        exigir(ausentes && duplicados && invalidos, "cobertura aleatoria incompleta");
        std::cout << "  semilla=" << semilla << " operaciones=12000 ausentes="
                  << ausentes << " duplicados=" << duplicados << " invalidos="
                  << invalidos << '\n';
    }
}
}  // namespace

int main() {
    try {
        const std::vector<std::pair<std::string, std::function<void()>>> pruebas = {
            {"validacion de perfiles", validar_perfiles},
            {"colisiones y rehash", colisiones_y_rehash},
            {"excepcion durante rehash", fallo_durante_rehash},
            {"contrato hash", contrato_registro<RegistroPersonajesHash>},
            {"contrato vector ordenado", contrato_registro<RegistroPersonajesOrdenado>},
            {"contrato referencia lenta", contrato_registro<soporte::RegistroPersonajesLento>},
            {"compatibilidad con Personaje", compatibilidad_personaje},
            {"comparacion diferencial del registro (36000 operaciones)", diferencial}
        };
        for (const auto& prueba : pruebas) {
            prueba.second();
            std::cout << "PASS " << prueba.first << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "FAIL " << error.what() << '\n';
        return 1;
    }
}
