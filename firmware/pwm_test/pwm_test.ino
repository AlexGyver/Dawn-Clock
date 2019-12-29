/*
  Скетч с тестом AC диммера и используемого источника света.
  Крутим ручку энкодера - меняем яркость лампы.
  На дисплей выводится текущее значение 0-255
  Пины такие же как и в основном скетче
*/

// *************************** НАСТРОЙКИ ***************************
#define DAWN_TYPE 0       // 1 - мосфет (DC диммер), 0 - симистор (AC диммер) СМОТРИ СХЕМЫ
#define ENCODER_TYPE 1    // тип энкодера (0 или 1). Типы энкодеров расписаны на странице проекта

// ************ ПИНЫ ************
#define CLKe 8        // энкодер
#define DTe 9         // энкодер
#define SWe 10        // энкодер

#define CLK 12        // дисплей
#define DIO 11        // дисплей

#define ZERO_PIN 2    // пин детектора нуля (Z-C) для диммера (если он используется)
#define DIM_PIN 3     // мосфет / DIM(PWM) пин диммера

#define BUZZ_PIN 7    // пищалка (по желанию)
#define LED_PIN 6     // светодиод индикатор

// ***************** ОБЪЕКТЫ И ПЕРЕМЕННЫЕ *****************
# include "GyverTimer.h"
# include "GyverEncoder.h"
Encoder enc(CLKe, DTe, SWe);

# include "GyverTM1637.h"
GyverTM1637 disp(CLK, DIO);

# include "EEPROM.h"
# include <CyberLib.h> // шустрая библиотека для таймера
# include <Wire.h>
# include "RTClib.h"

int tic, duty = 0, tmp = 0;
long lastUpdate = 0, i = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(DIM_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

#if (DAWN_TYPE == 0)
  pinMode(ZERO_PIN, INPUT);
  attachInterrupt(0, detect_up, FALLING);
  StartTimer1(timer_interrupt, 40);        // время для одного разряда ШИМ
  StopTimer1();                            // остановить таймер
#endif

  enc.setType(ENCODER_TYPE);     // тип энкодера TYPE1 одношаговый, TYPE2 двухшаговый. Если ваш энкодер работает странно, смените тип
  disp.clear();  
  disp.brightness(4);
}

void loop()
{
  enc.tick();

  if (enc.isRight())
  {
    tmp += 5;
  }

  if (enc.isLeft())
  {
    tmp -= 5;
  }

  duty = constrain(tmp, 0, 255);
  tmp = duty;

  if (millis() - lastUpdate > 600)
  {
    lastUpdate = millis();
    disp.displayInt(duty);    
  }

#if (DAWN_TYPE == 0)
  analogWrite(DIM_PIN, duty);
#endif
}


#if (DAWN_TYPE == 0)  // если диммер
//----------------------ОБРАБОТЧИКИ ПРЕРЫВАНИЙ--------------------------
void timer_interrupt() {          // прерывания таймера срабатывают каждые 40 мкс
  if (duty > 0) {
    tic++;                        // счетчик
    if (tic > (255 - duty))       // если настало время включать ток
      digitalWrite(DIM_PIN, 1);   // врубить ток
  }
}

void detect_up() {    // обработка внешнего прерывания на пересекание нуля снизу
  if (duty > 0) {
    tic = 0;                                  // обнулить счетчик
    ResumeTimer1();                           // перезапустить таймер
    attachInterrupt(0, detect_down, RISING);  // перенастроить прерывание
  }
}

void detect_down() {      // обработка внешнего прерывания на пересекание нуля сверху
  if (duty > 0) {
    tic = 0;                                  // обнулить счетчик
    StopTimer1();                             // остановить таймер
    digitalWrite(DIM_PIN, 0);                 // вырубить ток
    attachInterrupt(0, detect_up, FALLING);   // перенастроить прерывание
  }
}
#endif
