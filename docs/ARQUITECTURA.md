# Arquitectura — Combat-Engine

## Estado actual observado

C++17/CMake: `Personaje` y subclases guardan atributos y acciones; `main.cpp` compone partida por turnos; `seleccion_personajes.cpp` ofrece servidor visual local POSIX/Windows. Frontend estático en `public/`; Vercel ejecuta `api/game.js`, que contiene otra implementación JS del estado/reglas y serializa estado en cookie. No hay dependencia Python en el motor actual.

## Dirección académica

Mantener C++17/CMake y separar dominio de combate, estructuras/indexes, aplicación y CLI. Implementar dos estructuras propias con invariantes. Las operaciones y estructuras definitivas dependen del contrato CMP3002 aprobado por el equipo. La UI y Vercel se preservan mientras sigan funcionando; no ampliar UI antes del motor.

## Límite de consistencia

El juego local C++ y la API Vercel actualmente contienen reglas paralelas. La entrega académica debe declarar cuál es la fuente del motor y qué comportamiento web no se sincroniza todavía. No afirmar que la API llama al binario C++.
