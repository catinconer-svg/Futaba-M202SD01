#include "M202SD01LA.h"
#include <stdarg.h>
// ВАЖНО: Пользовательские символы используют ОСОБЫЙ формат 5 байтов
// Не путать с обычной матрицей 5×7!
// См. комментарий в M202SD01LA.h о формате данных
// Конструктор для последовательного интерфейса
M202SD01LA::M202SD01LA(uint8_t rxPin, uint8_t txPin, uint32_t baudRate) {
  _interfaceMode = IF_SERIAL;
  _serial = new SoftwareSerial(rxPin, txPin);
  _baudRate = baudRate;
  _cursorPos = POS_ROW1_START;
}

// Конструктор для параллельного интерфейса
M202SD01LA::M202SD01LA(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                       uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                       uint8_t wrPin, uint8_t selPin, uint8_t busyPin) {
  _interfaceMode = IF_PARALLEL;
  
  _dataPins[0] = d0;
  _dataPins[1] = d1;
  _dataPins[2] = d2;
  _dataPins[3] = d3;
  _dataPins[4] = d4;
  _dataPins[5] = d5;
  _dataPins[6] = d6;
  _dataPins[7] = d7;
  
  _wrPin = wrPin;
  _selPin = selPin;
  _busyPin = busyPin;
  
  _cursorPos = POS_ROW1_START;
}

// Инициализация
void M202SD01LA::begin() {
  if (_interfaceMode == IF_SERIAL) {
    _serial->begin(_baudRate);
    delay(100); // Даем время на инициализацию
  } else {
    initPins();
  }
  
  reset();
  delay(50);
  clear();
  setBrightness(BRIGHT_100);
  setCursorMode(CURSOR_OFF);
}

// Инициализация пинов для параллельного режима
void M202SD01LA::initPins() {
  for (int i = 0; i < 8; i++) {
    pinMode(_dataPins[i], OUTPUT);
  }
  
  pinMode(_wrPin, OUTPUT);
  pinMode(_selPin, OUTPUT);
  pinMode(_busyPin, INPUT);
  
  digitalWrite(_wrPin, HIGH);
  digitalWrite(_selPin, HIGH);
}

// НОВЫЙ МЕТОД: отправка одного байта
size_t M202SD01LA::write(uint8_t data) {
  sendByte(data);
  _cursorPos++;
  
  // Автоматический переход на новую строку
  if (_cursorPos > POS_ROW2_END) {
    _cursorPos = POS_ROW1_START;
  }
  
  return 1; // Возвращаем количество отправленных байтов
}

// Отправка строки (перегрузка метода write)
size_t M202SD01LA::write(const char* str) {
  size_t count = 0;
  while (*str) {
    write(*str++);
    count++;
  }
  return count;
}

// Отправка байта данных
void M202SD01LA::sendByte(uint8_t data) {
  if (_interfaceMode == IF_SERIAL) {
    serialWrite(data);
    delayMicroseconds(50); // Уменьшенная задержка для скорости
  } else {
    parallelWrite(data);
  }
}

// Отправка команды
void M202SD01LA::sendCommand(uint8_t cmd) {
  sendByte(cmd);
}

// Запись в параллельном режиме
void M202SD01LA::parallelWrite(uint8_t data) {
  waitNotBusy();
  
  // Устанавливаем данные на шину
  for (int i = 0; i < 8; i++) {
    digitalWrite(_dataPins[i], (data >> i) & 0x01);
  }
  
  // Активируем запись
  digitalWrite(_selPin, LOW);
  delayMicroseconds(1);
  digitalWrite(_wrPin, LOW);
  delayMicroseconds(1);
  digitalWrite(_wrPin, HIGH);
  delayMicroseconds(1);
  digitalWrite(_selPin, HIGH);
  
  // Обновляем позицию курсора
  _cursorPos++;
  if (_cursorPos > POS_ROW2_END) {
    _cursorPos = POS_ROW1_START;
  }
}

// Запись в последовательном режиме
void M202SD01LA::serialWrite(uint8_t data) {
  _serial->write(data);
}

// Ожидание готовности (параллельный режим)
void M202SD01LA::waitNotBusy() {
  if (_interfaceMode == IF_PARALLEL) {
    while (digitalRead(_busyPin) == HIGH) {
      delayMicroseconds(10);
    }
  }
}

// Проверка занятости (параллельный режим)
bool M202SD01LA::isBusy() {
  if (_interfaceMode == IF_PARALLEL) {
    return (digitalRead(_busyPin) == HIGH);
  }
  return false;
}

// Очистка экрана
void M202SD01LA::clear() {
  sendCommand(CMD_CLR);
  _cursorPos = POS_ROW1_START;
  delay(10);
}

// Сброс модуля
void M202SD01LA::reset() {
  sendCommand(CMD_RST);
  _cursorPos = POS_ROW1_START;
  delay(100);
}

// Установка яркости
void M202SD01LA::setBrightness(uint8_t level) {
  sendCommand(CMD_DIM);
  sendByte(level);
  delay(10);
}

// Установка режима курсора
void M202SD01LA::setCursorMode(uint8_t mode) {
  sendCommand(CMD_DC);
  sendByte(mode);
  delay(10);
}

// Установка позиции курсора (абсолютная)
void M202SD01LA::setCursor(uint8_t position) {
  if (position > POS_ROW2_END) position = POS_ROW1_START;
  
  sendCommand(CMD_DP);
  sendByte(position);
  _cursorPos = position;
  delay(10);
}

// Установка позиции курсора (строка и столбец)
void M202SD01LA::setCursor(uint8_t row, uint8_t col) {
  if (row > 1) row = 0;
  if (col > 19) col = 0;
  
  uint8_t position = (row == 0) ? POS_ROW1_START + col : POS_ROW2_START + col;
  setCursor(position);
}

// Курсор в домашнюю позицию (0,0)
void M202SD01LA::home() {
  setCursor(POS_ROW1_START);
}

// Печать строки (через метод write для скорости)
void M202SD01LA::print(const char* str) {
  write(str);
}

// Печать символа
void M202SD01LA::print(char c) {
  write((uint8_t)c);
}

// Печать целого числа
void M202SD01LA::print(int num) {
  char buffer[10];
  itoa(num, buffer, 10);
  print(buffer);
}

// Печать числа с плавающей точкой
void M202SD01LA::print(float num, uint8_t decimals) {
  char buffer[20];
  dtostrf(num, 0, decimals, buffer);
  print(buffer);
}

// Форматированный вывод (упрощенный)
void M202SD01LA::printf(const char* format, ...) {
  char buffer[41]; // Максимум 40 символов + терминатор
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  print(buffer);
}

// Backspace
void M202SD01LA::backspace() {
  sendCommand(CMD_BS);
  if (_cursorPos > POS_ROW1_START) {
    _cursorPos--;
  } else {
    _cursorPos = POS_ROW2_END;
  }
  delay(10);
}

// Horizontal Tab
void M202SD01LA::horizontalTab() {
  sendCommand(CMD_HT);
  _cursorPos++;
  if (_cursorPos > POS_ROW2_END) {
    _cursorPos = POS_ROW1_START;
  }
  delay(10);
}

// All Display ON (тестовый режим)
void M202SD01LA::allDisplayOn() {
  sendCommand(CMD_ALD);
  delay(10);
}

// Определение пользовательского символа
void M202SD01LA::defineCustomChar(uint8_t charNum, uint8_t* fontData) {
  if (charNum > 3) return; // Только 4 пользовательских символа (0-3)
  
  sendCommand(CMD_DEF);
  sendByte(0x1B + charNum); // 0x1B-0x1E для символов UF0-UF3
  
  for (int i = 0; i < 5; i++) {
    sendByte(fontData[i]);
  }
  
  delay(10);
}