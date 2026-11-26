# Prioridades y heap — análisis I2

Estado: propuesta para revisión de I1 e I3; no hay contrato aprobado de
prioridad, eventos, efectos ni selección de rival. El controlador actual solo
alterna dos personajes, por lo que un heap no mejora ni justifica esa operación.

Si el equipo aprueba una cola dinámica de eventos con prioridades cambiantes,
comparar estas alternativas sobre la misma operación y empates deterministas:

| Candidato | Insertar | Extraer siguiente | Cambiar prioridad | Espacio | Situación posible |
|---|---|---|---|---|---|
| Vector no ordenado | O(1) amortizado | O(n) | O(n) por búsqueda | O(n) | Pocas extracciones o cola pequeña; menos mantenimiento. |
| Heap binario propio | O(log n) peor caso, salvo realocación amortizada del vector | O(log n) | O(n) para localizar, más O(log n) para ajustar | O(n) | Muchas inserciones y extracciones con cola grande. |
| Heap con índice id→posición | O(log n) esperado con hash | O(log n) esperado con hash | O(log n) esperado con hash | O(n) adicional | Cambios frecuentes por id; cada intercambio mantiene el índice. |
| Vector ordenado | O(n) | O(1) si se extrae del extremo | O(n) | O(n) | Muchas consultas al siguiente y pocas altas/cambios. |

Los costos dependen de la API: ver el siguiente sin retirar es O(1) tanto en
heap como en vector ordenado; un índice auxiliar consume memoria y se actualiza
en cada intercambio, inserción y eliminación. Los empates necesitarían una
clave acordada, por ejemplo `(prioridad, secuencia, id)` con dirección definida.
No se debe introducir prioridad ni alterar el turno alternado solo para usar
una estructura. Si se necesitan cancelación, cambios o expiración de efectos,
definir identidad, orden, errores y estrategia antes de implementarlos.

Estas ventajas son hipótesis teóricas, no mediciones ni pruebas de que la
operación cuente en la rúbrica. El equipo debe demostrar un escenario razonable
en que cada candidato gane, decidir si dicha operación pertenece al motor y
probar invariantes (padre frente a hijos, tamaño, identidad y coherencia del
índice si existe). Ranking/top-k pertenece al contrato y decisiones de I3 y no
se presupone que comparta esta cola.
