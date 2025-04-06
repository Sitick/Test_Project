// При написании кода предполагал следующие допущения:
// 1. Подтяжка кнопок реализована аппаратно (Pull-Up)
// 2. Защита от дребезка не реализована программно поскольку имеется RC фильтр, а также довольно высокая частота опроса
// 3. Программное обеспечение будет использоваться для панелей управления различных устройств
// Исходя из пунта 3 я предположил что обработка кнопок не должна включать в себя:
// - обработку комбинаций кнопок
// - обработку длительного нажатия кнопки
// - обработку двойного/тройного нажатия кнопки
#include "Keyboard.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

// Определение портов и пинов строк и столбцов
GPIO_TypeDef* Keyboard::rowPorts[KEYBOARD_ROWS] = {GPIOA, GPIOA, GPIOB}; // Порты строк (В качестве примера взяты: PA0, PA1, PB2)
const uint8_t Keyboard::rowPins[KEYBOARD_ROWS] = {0, 1, 2};              // Пины строк (В качестве примера взяты: PA0, PA1, PB2)

GPIO_TypeDef* Keyboard::colPorts[KEYBOARD_COLS] = {GPIOA, GPIOB, GPIOB}; // Порты столбцов (В качестве примера взяты: PA3, PB4, PB5)
const uint8_t Keyboard::colPins[KEYBOARD_COLS] = {3, 4, 5};              // Пины столбцов (В качестве примера взяты: PA3, PB4, PB5)

// Матрица кнопок в виде перечисления Keys
const Keys Keyboard::keymap[KEYBOARD_ROWS][KEYBOARD_COLS] = {
    {UP, UP, UP},
    {LEFT, UP, RIGHT},
    {DOWN, INVALID, DOWN}
};


Keyboard::Keyboard() : lastKey(INVALID) {
    keyboardInit();
}

Keyboard::~Keyboard() {

}

// Инициализация GPIO для строк и столбцов
void Keyboard::keyboardInit() {
    // Включение тактирования GPIOA и GPIOB
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    // Настройка строк как входы без подтяжки (реализована аппаратно) с низкой скоростью переключения
    for (int i = 0; i < KEYBOARD_ROWS; ++i) {
        rowPorts[i]->MODER &= ~(3 << (rowPins[i] * 2));     // Установка режима Input
        rowPorts[i]->PUPDR &= ~(3 << (rowPins[i] * 2));     // без подтяжки Pull-Up/Down (реализуется аппаратно)
        rowPorts[i]->OSPEEDR &= ~(3 << (rowPins[i] * 2));   // Установка низкой скорости переключения
    }

    // Настройка столбцов как входы без подтяжки (реализована аппаратно) с низкой скоростью переключения
    for (int i = 0; i < KEYBOARD_COLS; ++i) {
        colPorts[i]->MODER &= ~(3 << (colPins[i] * 2));     // Установка режима Input
        colPorts[i]->PUPDR &= ~(3 << (colPins[i] * 2));     // без подтяжки Pull-Up/Down (реализуется аппаратно)
        colPorts[i]->OSPEEDR &= ~(3 << (colPins[i] * 2));   // Установка низкой скорости переключения
    }
}

// Получение кнопки из матрицы по строке и столбцу
Keys Keyboard::getKey(uint8_t row, uint8_t col) {
    return keymap[row][col];
}

// Получение последней нажатой кнопки
Keys Keyboard::keyboardGetKey() {
    Keys key = lastKey;
    lastKey = INVALID; // Сбрасываем значение после получения
    return key;
}

// Опрос кнопок
void Keyboard::keyboardPoll() {
    TickType_t xLastWakeTime = xTaskGetTickCount(); // Получаем текущее время
    Keys previousKey = INVALID; // Храним предыдущую обработанную кнопку (для корректной работы при долгом нажатии/ залипании кнопки)

    while (1) {
        bool keyFound = false;

        // Перебор строк клавиатуры
        for (int row = 0; row < KEYBOARD_ROWS; ++row) {
            // Опрос столбцов
            for (int col = 0; col < KEYBOARD_COLS; ++col) {
                // Проверяем, равны ли строка и столбец логическому нулю
                if (!(rowPorts[row]->IDR & (1 << rowPins[row])) &&
                    !(colPorts[col]->IDR & (1 << colPins[col]))) {
                    Keys key = getKey(row, col);
                    //  Защита от непредвиденных ситуаций
                    if (key == INVALID) {
                        continue;
                    }

                    // Записываем кнопку в lastKey, только если она не совпадает с кнопкой, нажатой в предыдущем цикле (для корректной работы при долгом нажатии/ залипании кнопки)
                    // и только первую нажатую кнопку (на случай если пользователь нажмет несколько кнопок сразу)
                    if (!keyFound && key != previousKey) {
                        lastKey = key;
                        previousKey = key;
                        keyFound = true;
                    }
                }
            }
        }

        // Сброс previousKey
        if (!keyFound) {
            previousKey = INVALID;
        }

        // Задержка для опроса клавиш в 5 мс 
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
    }
}
