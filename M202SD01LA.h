#ifndef M202SD01LA_h
#define M202SD01LA_h

#include <Arduino.h>
#include <SoftwareSerial.h>

// Режимы интерфейса
#define IF_PARALLEL 0
#define IF_SERIAL   1

// Команды модуля (из даташита)
#define CMD_DEF   0x03  // Определение пользовательского символа
#define CMD_DIM   0x04  // Регулировка яркости
#define CMD_BS    0x08  // Backspace
#define CMD_HT    0x09  // Horizontal Tab
#define CMD_CLR   0x0D  // Clear display
#define CMD_ALD   0x0F  // All Display ON
#define CMD_DP    0x10  // Display Position
#define CMD_DC    0x17  // Display Control (курсор)
#define CMD_RST   0x1F  // Reset

// Яркость
#define BRIGHT_100  0xFF
#define BRIGHT_80   0x80
#define BRIGHT_60   0x60
#define BRIGHT_40   0x40
#define BRIGHT_20   0x20
#define BRIGHT_0    0x00

// Режимы курсора
#define CURSOR_OFF      0x00
#define CURSOR_BLINK    0x88
#define CURSOR_ON       0xFF

// Позиции дисплея
#define POS_ROW1_START  0x00  // Первая строка, первый символ
#define POS_ROW1_END    0x13  // Первая строка, последний символ (19)
#define POS_ROW2_START  0x14  // Вторая строка, первый символ
#define POS_ROW2_END    0x27  // Вторая строка, последний символ (39)

// ============================================================================
// ВАЖНО: ФОРМАТ ПОЛЬЗОВАТЕЛЬСКИХ СИМВОЛОВ
// ============================================================================
/*
  Пользовательские символы (UF0-UF3) определяются с помощью команды DEF (0x03).
  Каждый символ состоит из 5 байтов, которые кодируют матрицу 5×7 точек.
  
  Коды пользовательских символов:
    UF0 = 0x1B (27)
    UF1 = 0x1C (28)
    UF2 = 0x1D (29) 
    UF3 = 0x1E (30)
  
  ФОРМАТ 5 БАЙТОВ (35 бит = 5×7 точек):
  Байты распределяют биты по матрице ОСОБЫМ образом, не построчно!
  
  Обозначение: X-Y, где X=столбец (1-5), Y=строка (1-7)
  
  Байт 1 (биты 7-0):
    7: 1-1, 6: 2-1, 5: 3-1, 4: 4-1, 3: 5-1, 2: 1-2, 1: 2-2, 0: 3-2
  
  Байт 2 (биты 7-0):
    7: 4-2, 6: 5-2, 5: 1-3, 4: 2-3, 3: 3-3, 2: 4-3, 1: 5-3, 0: 1-4
  
  Байт 3 (биты 7-0):
    7: 2-4, 6: 3-4, 5: 4-4, 4: 5-4, 3: 1-5, 2: 2-5, 1: 3-5, 0: 4-5
  
  Байт 4 (биты 7-0):
    7: 5-5, 6: 1-6, 5: 2-6, 4: 3-6, 3: 4-6, 2: 5-6, 1: 1-7, 0: 2-7
  
  Байт 5 (биты 7-0):
    7: 3-7, 6: 4-7, 5: 5-7, 4-0: должны быть 0 (!)
  
  ПРИМЕР: Символ "1" (вертикальная линия в среднем столбце)
  Матрица 5×7 (столбцы 1-5, строки 1-7 сверху вниз):
    Столбец 3 (средний) во всех строках = 1, остальные = 0
  
  Получаем байты: 0x23, 0x08, 0x42, 0x11, 0xC0
  
  ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ для конвертации матрицы 7×5 в формат модуля:
  (Добавьте в свой скетч, если нужно создавать символы программно)
  
  void matrixToBytes(bool matrix[7][5], uint8_t result[5]) {
    for (int i = 0; i < 5; i++) result[i] = 0;
    
    // Байт 1
    result[0] |= (matrix[0][0] ? 1 : 0) << 7; // 1-1
    result[0] |= (matrix[0][1] ? 1 : 0) << 6; // 2-1
    result[0] |= (matrix[0][2] ? 1 : 0) << 5; // 3-1
    result[0] |= (matrix[0][3] ? 1 : 0) << 4; // 4-1
    result[0] |= (matrix[0][4] ? 1 : 0) << 3; // 5-1
    result[0] |= (matrix[1][0] ? 1 : 0) << 2; // 1-2
    result[0] |= (matrix[1][1] ? 1 : 0) << 1; // 2-2
    result[0] |= (matrix[1][2] ? 1 : 0) << 0; // 3-2
    
    // Байт 2
    result[1] |= (matrix[1][3] ? 1 : 0) << 7; // 4-2
    result[1] |= (matrix[1][4] ? 1 : 0) << 6; // 5-2
    result[1] |= (matrix[2][0] ? 1 : 0) << 5; // 1-3
    result[1] |= (matrix[2][1] ? 1 : 0) << 4; // 2-3
    result[1] |= (matrix[2][2] ? 1 : 0) << 3; // 3-3
    result[1] |= (matrix[2][3] ? 1 : 0) << 2; // 4-3
    result[1] |= (matrix[2][4] ? 1 : 0) << 1; // 5-3
    result[1] |= (matrix[3][0] ? 1 : 0) << 0; // 1-4
    
    // Байт 3
    result[2] |= (matrix[3][1] ? 1 : 0) << 7; // 2-4
    result[2] |= (matrix[3][2] ? 1 : 0) << 6; // 3-4
    result[2] |= (matrix[3][3] ? 1 : 0) << 5; // 4-4
    result[2] |= (matrix[3][4] ? 1 : 0) << 4; // 5-4
    result[2] |= (matrix[4][0] ? 1 : 0) << 3; // 1-5
    result[2] |= (matrix[4][1] ? 1 : 0) << 2; // 2-5
    result[2] |= (matrix[4][2] ? 1 : 0) << 1; // 3-5
    result[2] |= (matrix[4][3] ? 1 : 0) << 0; // 4-5
    
    // Байт 4
    result[3] |= (matrix[4][4] ? 1 : 0) << 7; // 5-5
    result[3] |= (matrix[5][0] ? 1 : 0) << 6; // 1-6
    result[3] |= (matrix[5][1] ? 1 : 0) << 5; // 2-6
    result[3] |= (matrix[5][2] ? 1 : 0) << 4; // 3-6
    result[3] |= (matrix[5][3] ? 1 : 0) << 3; // 4-6
    result[3] |= (matrix[5][4] ? 1 : 0) << 2; // 5-6
    result[3] |= (matrix[6][0] ? 1 : 0) << 1; // 1-7
    result[3] |= (matrix[6][1] ? 1 : 0) << 0; // 2-7
    
    // Байт 5
    result[4] |= (matrix[6][2] ? 1 : 0) << 7; // 3-7
    result[4] |= (matrix[6][3] ? 1 : 0) << 6; // 4-7
    result[4] |= (matrix[6][4] ? 1 : 0) << 5; // 5-7
    // Биты 4-0 должны быть 0
  }
*/
// ============================================================================

class M202SD01LA {
  public:
    // Конструкторы
    M202SD01LA(uint8_t rxPin, uint8_t txPin, uint32_t baudRate = 9600); // Последовательный интерфейс
    M202SD01LA(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, 
               uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
               uint8_t wrPin, uint8_t selPin, uint8_t busyPin); // Параллельный интерфейс
    
    // Инициализация
    void begin();
    
    // Основные функции
    void clear();
    void reset();
    void setBrightness(uint8_t level);
    void setCursorMode(uint8_t mode);
    
    // Управление позицией
    void setCursor(uint8_t position);
    void setCursor(uint8_t row, uint8_t col);
    void home(); // Курсор в позицию 0,0
    
    // Вывод данных
    size_t write(uint8_t data);  // Отправка одного байта
    size_t write(const char* str);  // Отправка строки
    
    void print(const char* str);
    void print(char c);
    void print(int num);
    void print(float num, uint8_t decimals = 2);
    
    // Форматированный вывод
    void printf(const char* format, ...);
    
    // Специальные команды
    void backspace();
    void horizontalTab();
    void allDisplayOn();
    
    // Пользовательские символы
    void defineCustomChar(uint8_t charNum, uint8_t* fontData);
    
    // Утилиты
    void waitNotBusy(); // Только для параллельного режима
    bool isBusy();      // Только для параллельного режима
    
  private:
    uint8_t _interfaceMode;
    
    // Для последовательного интерфейса
    SoftwareSerial* _serial;
    uint32_t _baudRate;
    
    // Для параллельного интерфейса
    uint8_t _dataPins[8];
    uint8_t _wrPin;
    uint8_t _selPin;
    uint8_t _busyPin;
    
    // Внутренние методы
    void sendByte(uint8_t data);
    void sendCommand(uint8_t cmd);
    void parallelWrite(uint8_t data);
    void serialWrite(uint8_t data);
    void initPins();
    
    // Текущая позиция курсора
    uint8_t _cursorPos;
};

#endif