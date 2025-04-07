#pragma once

#include <cstdint>
#include "stm32f4xx.h"

// Определение количества строк и столбцов клавиатуры
#define KEYBOARD_ROWS 3
#define KEYBOARD_COLS 3

// Перечисление для обозначения клавиш
typedef enum {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    INVALID
} Keys;

class Keyboard {
public:
    Keyboard();
    ~Keyboard();
    void keyboardInit();                    // Инициализация клавиатуры
    void keyboardPoll();                    // Опрос клавиатуры
    Keys getKey(uint8_t row, uint8_t col);  // Получение клавиши из матрицы
    Keys keyboardGetKey();                  // Получить последнюю нажатую кнопку

private:
    static GPIO_TypeDef* row_Ports[KEYBOARD_ROWS];           // Порты строк
    static const uint8_t row_Pins[KEYBOARD_ROWS];            // Пины строк
    static GPIO_TypeDef* col_Ports[KEYBOARD_COLS];           // Порты столбцов
    static const uint8_t col_Pins[KEYBOARD_COLS];            // Пины столбцов
    static const Keys keymap[KEYBOARD_ROWS][KEYBOARD_COLS]; // Матрица клавиш
    volatile Keys last_Key;                                  // Последняя нажатая кнопка
};
