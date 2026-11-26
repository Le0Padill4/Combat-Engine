# Borrador de PR: registro de personajes y hash propia del integrante 1

Descripcion preparada para el PR en borrador. El usuario autorizo el 26 de
noviembre de 2026 el commit, push y publicacion para revision, sin merge.

Titulo propuesto: `[I1] Add isolated character registry, custom hash and reproducible evidence`

Rama: `feat/i1-registro-personajes`. Base prevista: `main`, a revisar contra el
estado vigente antes de publicar. Refs #3, #4 y #5; no cierra esos Issues globales.

## Problema y comportamiento

El juego actual no ofrece un registro extensible por ID separado del combate.
Se agrega un modulo C++17 aislado con perfiles, registro y consulta que devuelve
copias. Un ID repetido se rechaza sin reemplazar el perfil y un perfil invalido
no altera el registro.

El modulo incluye hash propia con encadenamiento/rehash, candidato con vector
ordenado y referencia lineal. El adaptador lee atributos/habilidades de
`Personaje` sin modificar su estado vivo. La API es una propuesta del integrante 1
para revision de contratos; no se conecta aun con combate, ranking ni frontend.

## Aceptacion y validacion local

- [x] Perfiles, duplicados, IDs ausentes y rangos con contrato documentado.
- [x] Colisiones forzadas, rehash, snapshots e invariantes verificados.
- [x] Fallo simulado de copia durante rehash conserva las entradas previas.
- [x] 36.000 operaciones del registro comparadas entre tres implementaciones.
- [x] Suite adicional con comprobaciones de contenedores `-D_GLIBCXX_DEBUG`.
- [x] Siete pruebas Node existentes satisfactorias.
- [x] Benchmark de una operacion: dos candidatos, dos escenarios, cuatro tamanos,
  cinco repeticiones; CSV y graficos reproducibles.
- [x] Workflow individual de CMake/CTest y regresiones Node en tres sistemas.
- [ ] Contrato revisado por integrantes 2/3.
- [ ] Build CMake/CTest y CI de esta rama.
- [ ] Integracion del motor completo y validacion academica colectiva.

Comandos realmente ejecutados:

```sh
node scripts/i1/verificar.js test
node --test tests/battle-contract.test.js tests/vercel-api.test.js
```

El benchmark y los graficos se generaron previamente con
`node scripts/i1/verificar.js all`; se conservan sus CSV y entorno. El 26 de
noviembre se repitieron las pruebas antes de publicar. No se declara CMake local;
el workflow propio valida esa ruta remotamente.

Los resultados, metodo, complejidades y limitaciones estan en
`docs/integrante1/README.md` y `docs/integrante1/CONTINUIDAD.md`.

## Impacto y coordinacion

Agrega los archivos del modulo y un workflow exclusivo del integrante 1. No cambia
el CMake raiz, reglas de combate, API, UI, activos o contratos compartidos.
El CMake standalone no se verifico localmente; el workflow lo ejecuta en Ubuntu,
macOS y Windows. No se declara que el CI de otro PR valide esta rama.

El costo de sincronizar registro/ranking y el protocolo ante fallos entre indices
requieren acuerdo previo a integracion. La comparacion aleatoria local no cubre
el motor completo. El benchmark de busqueda no satisface por si solo las dos
operaciones medidas requeridas en la entrega global.

Revisores propuestos: otro integrante para contrato del combate y otro para
registro/ranking/benchmarks. Sus identidades se confirmaran antes de solicitar
revision; no se inventan aprobaciones ni contribuciones.
