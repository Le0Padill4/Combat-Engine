# Instrucciones para agentes

- Lee `docs/ESTADO_PROYECTO.md`, `docs/PLAN_MAESTRO.md` y `docs/ARQUITECTURA.md` antes de cambiar el motor.
- Conserva la historia y funcionalidad del juego existente. No conviertas el dominio en matchmaking ArenaCore.
- Requisitos CMP3002 mandan sobre las propuestas. No uses datos personales reales ni inventes pruebas, benchmarks, aportes, errores de IA o aprobaciones docentes.
- C++17/CMake es el motor académico. La web en Vercel es secundaria; documenta cualquier divergencia de reglas entre C++ y `api/game.js` antes de tocarla.
- Estructuras propias deben tener invariantes y pruebas que las comprueben. Implementa referencia lenta y comparación diferencial antes de declarar aceptación.
- Usa ramas cortas y PRs; no hagas push directo a `main`. Revisión de otra persona, CI requerido y no autoaprobación.
- Mantén el build multiplataforma Windows/macOS. No elimines el ejecutable rastreado ni activos visuales sin decisión y revisión explícita del equipo.
