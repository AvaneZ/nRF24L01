//--------------------- НАСТРОЙКИ ----------------------
#define CH_AMOUNT 8   // число каналов (должно совпадать с приёмником)
#define CH_NUM 0x60   // номер канала (должен совпадать с приёмником)
//--------------------- НАСТРОЙКИ ----------------------

//--------------------- ДЛЯ РАЗРАБОТЧИКОВ -----------------------
// УРОВЕНЬ МОЩНОСТИ ПЕРЕДАТЧИКА
// На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
#define SIG_POWER RF24_PA_LOW

// СКОРОСТЬ ОБМЕНА
// На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
// должна быть одинакова на приёмнике и передатчике!
// при самой низкой скорости имеем самую высокую чувствительность и дальность!!
// ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!
#define SIG_SPEED RF24_1MBPS
//--------------------- ДЛЯ РАЗРАБОТЧИКОВ -----------------------

//--------------------- БИБЛИОТЕКИ ----------------------
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9, 53); // для Меги
//--------------------- БИБЛИОТЕКИ ----------------------

//--------------------- ПЕРЕМЕННЫЕ ----------------------
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // возможные номера труб

int transmit_data[CH_AMOUNT];  // массив пересылаемых данных
int telemetry[2];              // массив принятых от приёмника данных телеметрии
byte rssi;
int trnsmtd_pack = 1, failed_pack;
unsigned long RSSI_timer;
//--------------------- ПЕРЕМЕННЫЕ ----------------------


void setup() {
  Serial.begin(9600); // открываем порт для связи с ПК
  radioSetup();
}

void loop() {
  if (radio.write(&transmit_data, sizeof(transmit_data))) {    // отправка пакета
    trnsmtd_pack++;
    if (!radio.available()) {                                  // если получаем пустой ответ
    } else {
      while (radio.available() ) {                    // если в ответе что-то есть
        radio.read(&telemetry, sizeof(telemetry));    // читаем
        // получили забитый данными массив telemetry ответа от приёмника
      }
    }
  } else {
    failed_pack++;
  }

  if (millis() - RSSI_timer > 1000) {                        // таймер RSSI
    // расчёт качества связи (0 - 100%) на основе числа ошибок и числа успешных передач
    rssi = (1 - ((float)failed_pack / trnsmtd_pack)) * 100;  

    // сбросить значения
    failed_pack = 0;
    trnsmtd_pack = 0;
    RSSI_timer = millis();
  }

}

void radioSetup() {
  radio.begin();              // активировать модуль
  radio.setAutoAck(1);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);   // размер пакета, в байтах
  radio.openWritingPipe(address[0]);   // мы - труба 0, открываем канал для передачи данных
  radio.setChannel(CH_NUM);            // выбираем канал (в котором нет шумов!)
  radio.setPALevel(SIG_POWER);         // уровень мощности передатчика
  radio.setDataRate(SIG_SPEED);        // скорость обмена
  // должна быть одинакова на приёмнике и передатчике!
  // при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp();         // начать работу
  radio.stopListening();   // не слушаем радиоэфир, мы передатчик
}
