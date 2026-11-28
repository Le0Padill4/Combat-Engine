#!/usr/bin/env python3
"""Repite las mediciones de I3: compila, ejecuta, guarda CSV + entorno y grafica.

Uso (desde la raiz del repositorio):
    python3 benchmarks/i3/ejecutar.py            # mediciones completas (4 tamanos, 5 rep.)
    python3 benchmarks/i3/ejecutar.py --rapido   # solo comprueba que corre; no reportar

Requiere un compilador C++17 (c++/g++/clang++/cl no soportado aqui: usa CMake en
Windows con el target i3_bench si se prefiere MSVC) y, solo para graficar,
matplotlib. Las mediciones deben compararse en el MISMO computador.
"""
import datetime
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[2]
AQUI = Path(__file__).resolve().parent
RESULTADOS = AQUI / "resultados"
BUILD = RAIZ / "build" / "i3-bench"
FLAGS = ["-std=c++17", "-O2"]


def compilador():
    for c in ("c++", "g++", "clang++"):
        if shutil.which(c):
            return c
    raise SystemExit("No se encontro un compilador C++ (c++, g++ o clang++)")


def salida(cmd):
    try:
        return subprocess.run(cmd, capture_output=True, text=True, check=True).stdout.strip()
    except Exception:
        return "no disponible"


def main():
    rapido = "--rapido" in sys.argv[1:]
    cxx = compilador()
    BUILD.mkdir(parents=True, exist_ok=True)
    RESULTADOS.mkdir(parents=True, exist_ok=True)
    exe = BUILD / ("bench_i3.exe" if os.name == "nt" else "bench_i3")
    cmd = [cxx, *FLAGS, f"-I{RAIZ / 'src' / 'headers'}", str(AQUI / "bench_i3.cpp"), "-o", str(exe)]
    print(" ".join(cmd))
    subprocess.run(cmd, check=True)

    destino = RESULTADOS / ("mediciones_rapido.csv" if rapido else "mediciones.csv")
    args = [str(exe), str(destino)] + (["--rapido"] if rapido else [])
    inicio = datetime.datetime.now().isoformat(timespec="seconds")
    subprocess.run(args, check=True)

    try:
        import matplotlib
        mpl = matplotlib.__version__
    except ImportError:
        mpl = "no instalado"
    cpu = ""
    if Path("/proc/cpuinfo").exists():
        for linea in Path("/proc/cpuinfo").read_text().splitlines():
            if linea.startswith("model name"):
                cpu = linea.split(":", 1)[1].strip()
                break
    cpu = cpu or platform.processor() or "no disponible"
    (RESULTADOS / "entorno.txt").write_text(
        "\n".join([
            f"fecha_inicio: {inicio}",
            f"modo: {'rapido (no reportable)' if rapido else 'completo'}",
            f"sistema: {platform.platform()}",
            f"cpu: {cpu}",
            f"nucleos_logicos: {os.cpu_count()}",
            f"compilador: {salida([cxx, '--version']).splitlines()[0] if salida([cxx, '--version']) != 'no disponible' else 'no disponible'}",
            f"flags: {' '.join(FLAGS)}",
            f"python: {platform.python_version()}",
            f"matplotlib: {mpl}",
            "nota: todas las filas del CSV se midieron en este mismo computador, en una sola ejecucion.",
        ]) + "\n",
        encoding="utf-8",
    )
    if mpl == "no instalado":
        print("matplotlib no esta instalado: se omiten los graficos (el CSV ya esta escrito)")
        return
    subprocess.run([sys.executable, str(AQUI / "graficar.py"), str(destino), str(RESULTADOS)], check=True)


if __name__ == "__main__":
    main()
