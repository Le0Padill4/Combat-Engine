# language: es
Caracteristica: Registro de definiciones de personajes por ID
  El registro devuelve copias de perfiles y no controla el estado de combate.
  Propuesta local del integrante 1; pendiente de acuerdo de integracion del equipo.

  Escenario: Registrar y consultar un personaje ficticio
    Dado un registro vacio
    Cuando registro un perfil valido con ID "personaje-1"
    Entonces el resultado es "registrado"
    Y buscar "personaje-1" devuelve todos sus atributos y habilidades

  Escenario: Rechazar un ID repetido sin reemplazar el perfil
    Dado un perfil registrado con ID "personaje-1"
    Cuando intento registrar otro perfil valido con el mismo ID
    Entonces el resultado es "id_duplicado"
    Y el perfil original y el tamano no cambian

  Escenario: Rechazar atributos invalidos antes de comprobar duplicados
    Dado un perfil registrado con ID "personaje-1"
    Cuando intento registrar ese ID con vida maxima cero
    Entonces el resultado es "perfil_invalido"
    Y el registro no cambia

  Escenario: Consultar un ID inexistente
    Dado un registro vacio o con personajes
    Cuando busco un ID inexistente
    Entonces recibo un opcional vacio
    Y el registro no cambia

  Escenario: Conservar perfiles durante colisiones y crecimiento
    Dado una hash con todas las claves en colision
    Cuando registro suficientes claves distintas para provocar rehash
    Entonces cada clave conserva su valor
    Y se cumplen los invariantes de unicidad, bucket, tamano y factor de carga

  Escenario: Mantener una copia de consulta independiente
    Dado un perfil registrado y una copia devuelta por buscar
    Cuando modifico esa copia y el registro crece
    Entonces el perfil almacenado no cambia
    Y la copia sigue siendo valida

  Escenario: Leer la definicion sin alterar una partida
    Dado un Personaje existente con vida reducida, KI y defensa activa
    Cuando extraigo su perfil con un ID estable
    Entonces el perfil contiene atributos maximos y habilidades
    Y el estado de combate permanece intacto
