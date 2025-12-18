/*
 * Простые часы для VFD M202SD01LA
 * Формат HH-MM-SS по центру верхней строки
 */

#include <M202SD01LA.h>

// Конфигурация пинов
#define RX_PIN 2
#define TX_PIN 3

M202SD01LA vfd(RX_PIN, TX_PIN, 9600);

// Переменные времени
uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;

// Таймеры
uint32_t lastUpdate = 0;
uint32_t lastBlink = 0;
bool blinkState = true;

void setup() {
  Serial.begin(115200);
  Serial.println("Часы VFD");
  
  vfd.begin();
  vfd.setBrightness(BRIGHT_100);
  vfd.clear();
  
  // Заставка
  showSplash();
  
  // Установим начальное время (можно поменять)
  hours = 12;
  minutes = 30;
  seconds = 0;
  
  lastUpdate = millis();
  lastBlink = millis();
}

void loop() {
  uint32_t now = millis();
  
  // Обновление времени каждую секунду
  if (now - lastUpdate >= 1000) {
    updateTime();
    displayTime();
    lastUpdate = now;
  }
  
  // Мигание двоеточий каждые 500мс
  if (now - lastBlink >= 500) {
    blinkState = !blinkState;
    updateColons();
    lastBlink = now;
  }
}

// Обновление времени
void updateTime() {
  seconds++;
  
  if (seconds >= 60) {
    seconds = 0;
    minutes++;
    
    if (minutes >= 60) {
      minutes = 0;
      hours++;
      
      if (hours >= 24) {
        hours = 0;
      }
    }
  }
}

// Отображение времени по центру верхней строки
void displayTime() {
  // Верхняя строка имеет 20 символов
  // Формат "HH-MM-SS" занимает 8 символов
  // Центрируем: (20 - 8) / 2 = 6
  
  vfd.setCursor(0, 6); // Начинаем с 6-й позиции
  
  // Часы
  if (hours < 10) {
    vfd.print("0");
    vfd.print((int)hours);
  } else {
    vfd.print((int)hours);
  }
  
  // Первое двоеточие (будет обновляться отдельно)
  vfd.setCursor(0, 8);
  vfd.print(":");
  
  // Минуты
  vfd.setCursor(0, 9);
  if (minutes < 10) {
    vfd.print("0");
    vfd.print((int)minutes);
  } else {
    vfd.print((int)minutes);
  }
  
  // Второе двоеточие
  vfd.setCursor(0, 11);
  vfd.print(":");
  
  // Секунды
  vfd.setCursor(0, 12);
  if (seconds < 10) {
    vfd.print("0");
    vfd.print((int)seconds);
  } else {
    vfd.print((int)seconds);
  }
  
  // Информация на второй строке
  displayInfo();
}

// Обновление двоеточий (мигание)
void updateColons() {
  if (blinkState) {
    vfd.setCursor(0, 8);
    vfd.print(":");
    vfd.setCursor(0, 11);
    vfd.print(":");
  } else {
    vfd.setCursor(0, 8);
    vfd.print(" ");
    vfd.setCursor(0, 11);
    vfd.print(" ");
  }
}

// Отображение информации на второй строке
void displayInfo() {
  vfd.setCursor(1, 0);
  
  // Формат: "UPTIME: MM:SS"
  uint32_t uptime = millis() / 1000;
  uint8_t uptime_min = (uptime / 60) % 60;
  uint8_t uptime_sec = uptime % 60;
  
  vfd.print("UPTIME:");
  
  if (uptime_min < 10) {
    vfd.print("0");
    vfd.print((int)uptime_min);
  } else {
    vfd.print((int)uptime_min);
  }
  
  vfd.print(":");
  
  if (uptime_sec < 10) {
    vfd.print("0");
    vfd.print((int)uptime_sec);
  } else {
    vfd.print((int)uptime_sec);
  }
  
  // Индикатор мигания
  vfd.setCursor(1, 18);
  if (blinkState) {
    vfd.print("*");
  } else {
    vfd.print(" ");
  }
}

// Заставка
void showSplash() {
  vfd.clear();
  
  vfd.setCursor(0, 0);
  vfd.print("VFD CLOCK v1.0");
  
  vfd.setCursor(1, 0);
  vfd.print("HH-MM-SS FORMAT");
  
  delay(2000);
  
  // Анимация
  vfd.clear();
  vfd.setCursor(0, 0);
  vfd.print("INITIALIZING...");
  
  for (int i = 0; i < 3; i++) {
    vfd.setCursor(1, 0);
    vfd.print(".");
    delay(300);
    vfd.setCursor(1, 1);
    vfd.print(".");
    delay(300);
    vfd.setCursor(1, 2);
    vfd.print(".");
    delay(300);
  }
  
  vfd.clear();
}
