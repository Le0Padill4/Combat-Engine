Feature: Secuencia de combate por turnos
  Scenario: alternancia tras acción ejecutada
    Given dos combatientes diferentes y vivos
    When el actor realiza una acción válida y no termina el combate
    Then el siguiente turno corresponde al adversario
  Scenario: intento especial sin ki
    When el actor intenta un especial sin ki suficiente
    Then no causa daño y el turno pasa al adversario
  Scenario: acción inválida
    When se recibe una acción que no existe
    Then no cambia el actor ni aumenta el contador de turnos
  Scenario: resultado
    When un combatiente pierde toda su vida
    Then el combate termina y el combatiente vivo es el ganador
