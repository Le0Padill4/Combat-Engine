# Preparación de la defensa — I3

Solo contiene lo que está respaldado por archivos del repositorio. De I1 e I2 no se resume nada: que lo
presenten sus autores.

## Una decisión de diseño
Orden del ranking `(puntos desc, id asc)` con AVL con tamaños de subárbol: permite `posicion` y `top_k` en
O(log n) y O(log n + k) sin reordenar. Alternativa descartada: arreglo ordenado (actualizar es O(n)).

## Corrección y costo
- Corrección: 400.000 operaciones comparadas contra una referencia lenta, 9 mutantes detectados, invariantes del AVL.
- Costo: tabla de complejidad en `CONTRATO_I3.md` §4; mediciones en §5.

## Una medición para explicar
`top_k` "torneo" (1 consulta tras 1999 cambios): el heap gana 54× a n=10³ pero el AVL gana 2,25× a n=10⁶. La
conclusión no es "el heap es mejor": depende de cuántas actualizaciones hay entre consultas.

## Qué no se demuestra (decirlo sin que pregunten)
Una sola VM; reglas de puntaje propuestas; sin concurrencia; CI/MSVC sin ejecutar; integración real solo probada
en copia desechable; hueco de operaciones del grupo (6–8) sin cerrar.

## Preguntas probables
1. ¿Por qué el buffer circular pierde con 1 % de anulaciones? Anular desplaza O(N) elementos; la lista con índice es O(1).
2. ¿Por qué `ganador()==nullptr` no basta? También ocurre con combate sin terminar; se consulta `terminado()` antes.
3. ¿Cómo se evita contar dos veces un combate? `id_externo` idempotente (`duplicado`/`conflicto_clave`).
4. ¿Qué cambiaría si cambia el contrato de puntaje? Constantes en `tipos.h`; las pruebas lo detectan.
