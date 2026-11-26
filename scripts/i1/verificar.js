"use strict";

// Reuses the Node runtime already used by the repository's contract tests.
// No packages, shell interpolation, execution-policy changes or installation.
const fs = require("node:fs");
const path = require("node:path");
const os = require("node:os");
const { spawnSync } = require("node:child_process");

function main() {
    const raiz = path.resolve(__dirname, "../..");
    const args = process.argv.slice(2);
    const accion = args.shift() || "test";
    if (!["test", "benchmark", "all"].includes(accion))
        throw new Error("Uso: node scripts/i1/verificar.js test|benchmark|all [--cxx ruta] [--repeticiones 5]");
    let cxx = process.env.CXX;
    let repeticiones = 5;
    while (args.length) {
        const opcion = args.shift();
        const valor = args.shift();
        if (!valor) throw new Error(`Falta valor para ${opcion}`);
        if (opcion === "--cxx") cxx = valor;
        else if (opcion === "--repeticiones") {
            if (!/^\d+$/.test(valor)) throw new Error("Repeticiones debe ser entero");
            repeticiones = Number(valor);
        } else throw new Error(`Opcion desconocida: ${opcion}`);
    }
    if (repeticiones < 3 || repeticiones > 100)
        throw new Error("Repeticiones debe estar entre 3 y 100");
    const disponible = (programa) => {
        const r = spawnSync(programa, ["--version"], { encoding: "utf8", windowsHide: true });
        return !r.error && r.status === 0;
    };
    if (!cxx) {
        const candidatos = ["g++", "clang++"];
        if (process.platform === "win32")
            candidatos.push(path.join(os.homedir(), "Downloads/Dev-Cpp/TDM-GCC-64/bin/g++.exe"));
        cxx = candidatos.find(disponible);
    }
    if (!cxx || !disponible(cxx)) throw new Error("Indica --cxx con la ruta a g++ o clang++ C++17");
    const ejecutar = (programa, argumentos, capturar = false) => {
        const entorno = { ...process.env };
        if (process.platform === "win32" && /^rscript(?:\.exe)?$/i.test(path.basename(programa))) {
            entorno.LC_ALL = "C";
            entorno.LANG = "C";
        }
        const r = spawnSync(programa, argumentos, {
            cwd: raiz,
            encoding: "utf8",
            windowsHide: true,
            stdio: capturar ? "pipe" : "inherit",
            env: entorno,
        });
        if (r.error) throw r.error;
        if (r.status !== 0) throw new Error(`${programa} termino con codigo ${r.status}`);
        return r.stdout;
    };
    const extension = process.platform === "win32" ? ".exe" : "";
    const build = path.join(raiz, "build/i1");
    fs.mkdirSync(build, { recursive: true });
    const flags = ["-std=c++17", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Isrc/headers"];
    if (["test", "all"].includes(accion)) {
        const ejecutable = path.join(build, `registro_tests${extension}`);
        ejecutar(cxx, [...flags, "tests/i1/registro_tests.cpp", "src/personajes/personaje.cpp", "-o", ejecutable]);
        ejecutar(ejecutable, []);
    }
    if (["benchmark", "all"].includes(accion)) {
        const ejecutable = path.join(build, `registro_benchmark${extension}`);
        ejecutar(cxx, [...flags, "benchmarks/i1/registro_benchmark.cpp", "-o", ejecutable]);
        const resultados = path.join(raiz, "benchmarks/i1/resultados");
        fs.mkdirSync(resultados, { recursive: true });
        const metadatos = [
            "Alcance: registro del integrante 1; no motor completo",
            `Compilador: ${ejecutar(cxx, ["--version"], true).split(/\r?\n/)[0]}`,
            `Flags: ${flags.join(" ")}`,
            `OS: ${os.type()} ${os.release()} ${os.arch()}`,
            `Procesador: ${os.cpus()[0]?.model || "no disponible"}`,
            `Node: ${process.version}`,
            `Repeticiones: ${repeticiones}`,
            "Semilla base: 20261125",
            "Generador: std::mt19937; semilla efectiva = base + n + indice de escenario",
            "Generacion de fixtures, validacion y graficacion fuera del intervalo de consultas",
        ];
        ejecutar(ejecutable, [path.join(resultados, "busqueda_por_id.csv"), String(repeticiones)]);
        let rscript = process.env.RSCRIPT;
        if (!rscript && disponible("Rscript")) rscript = "Rscript";
        if (!rscript && process.platform === "win32") {
            const directorioR = path.join(process.env.ProgramFiles || "C:/Program Files", "R");
            if (fs.existsSync(directorioR))
                rscript = fs.readdirSync(directorioR).sort().reverse()
                    .map((nombre) => path.join(directorioR, nombre, "bin/Rscript.exe"))
                    .find((candidato) => fs.existsSync(candidato));
        }
        if (rscript) {
            metadatos.push(`R: ${ejecutar(rscript, ["--version"], true).trim()}`);
            ejecutar(rscript, ["--vanilla", "benchmarks/i1/graficar.R", resultados]);
        } else {
            console.warn("CSV generado. Ejecuta graficar.R con Rscript para generar graficos.");
        }
        fs.writeFileSync(path.join(resultados, "entorno.txt"), metadatos.join("\n") + "\n");
    }
}

try { main(); }
catch (error) { console.error(error.message); process.exitCode = 1; }
