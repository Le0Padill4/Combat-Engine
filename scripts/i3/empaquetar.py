#!/usr/bin/env python3
"""Crea el ZIP academico del proyecto sin historial, ejecutables ni carpetas de build.

Uso:  python3 scripts/i3/empaquetar.py APELLIDO1 APELLIDO2 [APELLIDO3 ...]
Salida: proyecto-<apellido1>-<apellido2>-....zip en el directorio padre del repo.
Los apellidos se pasan por argumento; este script no los conoce.
"""
import sys
import zipfile
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[2]
CARPETAS_EXCLUIDAS = {".git", "build", "node_modules", "__pycache__", ".vercel", ".idea", ".vscode"}
PREFIJOS_EXCLUIDOS = ("build", "cmake-build")
EXTENSIONES_EXCLUIDAS = {".exe", ".o", ".obj", ".a", ".lib", ".dll", ".so", ".dylib", ".pdb", ".ilk", ".pyc", ".zip"}
NOMBRES_EXCLUIDOS = {".env", ".DS_Store", "Thumbs.db"}


def excluido(ruta: Path) -> bool:
    rel = ruta.relative_to(RAIZ)
    for parte in rel.parts[:-1]:
        if parte in CARPETAS_EXCLUIDAS or parte.startswith(PREFIJOS_EXCLUIDOS):
            return True
    return (
        ruta.name in NOMBRES_EXCLUIDOS
        or ruta.name.startswith(".env")
        or ruta.suffix.lower() in EXTENSIONES_EXCLUIDAS
    )


def main() -> int:
    apellidos = [a.strip().lower() for a in sys.argv[1:] if a.strip()]
    if len(apellidos) < 2:
        print(__doc__)
        return 2
    destino = RAIZ.parent / ("proyecto-" + "-".join(apellidos) + ".zip")
    n = 0
    with zipfile.ZipFile(destino, "w", zipfile.ZIP_DEFLATED) as z:
        for ruta in sorted(RAIZ.rglob("*")):
            if ruta.is_file() and not ruta.is_symlink() and not excluido(ruta):
                z.write(ruta, ruta.relative_to(RAIZ).as_posix())
                n += 1
    print(f"{destino} ({n} archivos)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
