Feature: Acción de combate sin depender de la consola
  Scenario: ataque normal
    Given dos personajes vivos y distintos
    When el actor usa ataque normal
    Then se aplica el ataque propio del personaje y gana 15 de ki hasta su máximo
  Scenario: especial sin ki suficiente
    Given un actor sin ki suficiente
    When intenta su especial
    Then no causa daño ni gasta ki y el turno se considera ejecutado
  Scenario: defensa
    When el actor se defiende
    Then obtiene 10 de ki hasta su máximo y permanece defendiendo hasta su siguiente turno
  Scenario: entrada rechazada
    When ambos participantes son el mismo o uno ya no vive o la acción no existe
    Then la operación informa el motivo sin cambiar el estado
