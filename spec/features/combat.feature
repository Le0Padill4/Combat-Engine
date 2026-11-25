Feature: Combate por turnos
  Scenario: Selección de combatientes distintos
    Given cuatro personajes de prueba
    When dos jugadores seleccionan el mismo id
    Then la segunda selección se rechaza sin cambiar el combate

  Scenario: Rechazar especial sin energía suficiente
    Given una partida activa y energía menor al costo especial
    When el jugador intenta el ataque especial
    Then no se descuenta energía ni cambia la vida rival

  Scenario: Limitar vida al mínimo
    Given un combatiente con vida positiva
    When recibe daño mayor que su vida actual
    Then su vida queda en cero y la partida termina
