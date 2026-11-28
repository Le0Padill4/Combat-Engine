#!/usr/bin/env python3
"""Genera medianas.csv y un grafico por operacion a partir de mediciones.csv.

matplotlib es SOLO una dependencia de desarrollo para los graficos; el motor C++
no la usa. Uso: graficar.py <mediciones.csv> <directorio_salida>
"""
import csv
import statistics
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

# Paleta categorica validada (slots 1 y 2) con marcador distinto por candidato:
# el naranja queda bajo 3:1 de contraste, asi que la identidad no depende del color.
ESTILO = [("#257de0", "o"), ("#f47230", "s")]
TEXTO, TEXTO_SEC, REJILLA = "#191919", "#666666", "#e6e6e6"

TITULOS = {
    "top_k": "top_k(10): AVL aumentado vs heap en la consulta",
    "posicion": "posicion(id): AVL aumentado vs arreglo ordenado",
    "historial_anular": "historial: buffer circular vs lista doble (registrar + anular)",
}
ORDEN_OPS = ["top_k", "posicion", "historial_anular"]


def leer(ruta):
    filas = []
    with open(ruta, newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            r["n"] = int(r["n"])
            r["repeticion"] = int(r["repeticion"])
            r["ns_por_op"] = float(r["ns_por_op"])
            r["bytes_estructura"] = int(r["bytes_estructura"])
            filas.append(r)
    return filas


def verificar_checksums(filas):
    """Ambos candidatos reciben el mismo flujo: sus checksums deben coincidir."""
    grupos = defaultdict(set)
    for r in filas:
        grupos[(r["operacion"], r["escenario"], r["n"], r["repeticion"])].add(r["checksum"])
    malos = [k for k, v in grupos.items() if len(v) != 1]
    if malos:
        raise SystemExit(f"Checksums distintos entre candidatos en {malos[:3]}: resultados no equivalentes")


def agrupar(filas):
    g = defaultdict(list)
    for r in filas:
        g[(r["operacion"], r["escenario"], r["candidato"], r["n"])].append(r)
    return g


def escribir_medianas(g, salida):
    with open(salida, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["operacion", "escenario", "candidato", "n", "repeticiones",
                    "mediana_ns_por_op", "min_ns_por_op", "max_ns_por_op", "bytes_estructura"])
        for k in sorted(g):
            v = [r["ns_por_op"] for r in g[k]]
            w.writerow([*k, len(v), f"{statistics.median(v):.1f}", f"{min(v):.1f}",
                        f"{max(v):.1f}", g[k][0]["bytes_estructura"]])


def graficar(g, op, salida_dir, reps):
    escenarios = sorted({k[1] for k in g if k[0] == op})
    candidatos = sorted({k[2] for k in g if k[0] == op})
    fig, ejes = plt.subplots(1, len(escenarios), figsize=(14.5, 4.6), sharey=True, facecolor="white")
    if len(escenarios) == 1:
        ejes = [ejes]
    for ax, esc in zip(ejes, escenarios):
        ax.set_facecolor("white")
        # etiqueta directa: la del candidato mas alto va arriba y la otra abajo (sin pisarse)
        ultimo = {c: statistics.median(r["ns_por_op"] for r in g[(op, esc, c, max(k[3] for k in g if k[:3] == (op, esc, c)))])
                  for c in candidatos}
        mayor = max(ultimo, key=ultimo.get)
        for (cand, (color, marcador)) in zip(candidatos, ESTILO):
            ns = sorted({k[3] for k in g if k[:3] == (op, esc, cand)})
            med = [statistics.median(r["ns_por_op"] for r in g[(op, esc, cand, n)]) for n in ns]
            lo = [min(r["ns_por_op"] for r in g[(op, esc, cand, n)]) for n in ns]
            hi = [max(r["ns_por_op"] for r in g[(op, esc, cand, n)]) for n in ns]
            ax.errorbar(ns, med, yerr=[[m - a for m, a in zip(med, lo)], [b - m for m, b in zip(med, hi)]],
                        color=color, marker=marcador, markersize=7, linewidth=2, capsize=3,
                        markeredgecolor="white", markeredgewidth=1.5, label=cand)
            ax.annotate(cand, (ns[-1], med[-1]), textcoords="offset points", xytext=(-6, 9 if cand == mayor else -17),
                        ha="right", color=TEXTO, fontsize=9)
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_title(esc.replace("_", " "), fontsize=10, color=TEXTO)
        ax.set_xlabel("tamano n", color=TEXTO_SEC)
        ax.grid(True, which="major", color=REJILLA, linewidth=0.8)
        ax.tick_params(colors=TEXTO_SEC, labelsize=9)
        for lado in ("top", "right"):
            ax.spines[lado].set_visible(False)
        for lado in ("left", "bottom"):
            ax.spines[lado].set_color("#bbbbbb")
    ejes[0].set_ylabel(f"ns por operacion (mediana, barras = min-max de {reps} rep.)", color=TEXTO_SEC)
    ejes[-1].legend(frameon=False, fontsize=9, loc="upper left")
    fig.suptitle(TITULOS[op], fontsize=12, color=TEXTO, x=0.01, ha="left")
    fig.tight_layout(rect=(0, 0, 1, 0.94))
    for ext in ("png", "svg"):
        fig.savefig(salida_dir / f"{op}.{ext}", dpi=150)
    plt.close(fig)


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    filas = leer(sys.argv[1])
    out = Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    verificar_checksums(filas)
    g = agrupar(filas)
    reps = max(r["repeticion"] for r in filas)
    escribir_medianas(g, out / "medianas.csv")
    for op in ORDEN_OPS:
        if any(k[0] == op for k in g):
            graficar(g, op, out, reps)
    print("graficos y medianas escritos en", out)


if __name__ == "__main__":
    main()
