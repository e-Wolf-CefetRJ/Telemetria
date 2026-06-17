#pragma once

#include <Arduino.h>

namespace Pins {
    // Sensores
    constexpr uint8_t THROTTLE     = A0;   // aceleração, Mega novo, podemos usar A0 novamente
    constexpr uint8_t RPM          = A8;   // rotação por minuto, PCINT2 / PK0
    constexpr uint8_t IBAT         = A2;   // corrente da bateria
    constexpr uint8_t IMOT         = A3;   // corrente do motor
    constexpr uint8_t DHT22        = 4;    // temperatura/umidade
    
    // Controlador do Motor
    constexpr uint8_t PWM_OUT      = 11;   // Controle do motor, Timer1 
    
    // Relacionados ao SD
    constexpr uint8_t SD_CS        = 10;   // Logger
    constexpr uint8_t SPI_MISO     = 50;   // Lê dados
    constexpr uint8_t SPI_MOSI     = 51;   // Escreve dados
    constexpr uint8_t SPI_SCK      = 52;   // Clock do SD
    constexpr uint8_t SPI_SS       = 53;   // Configuração do barramento, Deve ser OUTPUT para modo mestre SPI

    // RTC DS1302 (Real Time Clock)
    constexpr uint8_t RTC_DATA     = 22;
    constexpr uint8_t RTC_CLOCK    = 23;
    constexpr uint8_t RTC_RESET    = 24;

    // PCINT (Relacionados ao RPM, não são pinos físicos)
    constexpr uint8_t PCINT_PIN    = _BV(PK0);           // Bit 0 do registrador PINK
    constexpr uint8_t PCINT_MASK   = _BV(PCINT16);  // Constante que identifica interrupções
    constexpr uint8_t PCIE_NUM     = _BV(PCIE2);         // Bit 2 do registrador PCICR
}