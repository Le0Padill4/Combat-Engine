# Comandos de I3 (borrador para el README del grupo)

Requisitos: CMake ≥ 3.16 y un compilador C++17. Python 3 con matplotlib solo para regenerar gráficos.

```
# Pruebas (proyecto aislado)
cmake -S tests/i3 -B build-i3 -DCMAKE_BUILD_TYPE=Release
cmake --build build-i3
ctest --test-dir build-i3 --output-on-failure

# Benchmarks: comprobacion rapida (no reportable) y medicion completa
# (la completa SOBREESCRIBE benchmarks/i3/resultados/ y tarda varios minutos)
python3 benchmarks/i3/ejecutar.py --rapido
python3 benchmarks/i3/ejecutar.py
```
Resultados publicados: `benchmarks/i3/resultados/`. Contrato: `docs/i3/CONTRATO_I3.md`.
