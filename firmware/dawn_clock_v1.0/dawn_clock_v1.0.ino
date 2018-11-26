/*
  Скетч к проекту "Будильник - рассвет"
  Страница проекта (схемы, описания): https://alexgyver.ru/dawn-clock/
  Исходники на GitHub: https://github.com/AlexGyver/dawn-clock
  Нравится, как написан и закомментирован код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2018
  http://AlexGyver.ru/
*/
/*
   Клик в режиме часов -> установка будильника
   Клик в режиме установки будильника -> режим часов
   Удержание в режиме часов -> вкл/выкл будильник
   Удержание в режиме установки будильника -> установка времени
   Клик/удержание в режиме установки времени -> режим часов

   Режим часов: двоеточие моргает раз в секунду
   Установка будильника: цифры вместе с двоеточием моргают
   Установка часов: двоеточие горит, цифры моргают
*/

// *************************** НАСТРОЙКИ ***************************
#define DAWN_TIME 20      // продолжительность рассвета (в минутах)
#define ALARM_TIMEOUT 80  // таймаут на автоотключение будильника, секунды
#define ALARM_BLINK 0     // 1 - мигать лампой при будильнике, 0 - не мигать
#define CLOCK_EFFECT 1    // эффект перелистывания часов: 0 - обычный, 1 - прокрутка, 2 - скрутка
#define BUZZ 1            // пищать пищалкой (1 вкл, 0 выкл)
#define BUZZ_FREQ 800     // частота писка (Гц)

#define DAWN_TYPE 0       // 1 - мосфет (DC диммер), 0 - симистор (AC диммер) СМОТРИ СХЕМЫ
#define DAWN_MIN 50       // начальная яркость лампы (0 - 255) (для сетевых матриц начало света примерно с 50)
#define DAWN_MAX 200      // максимальная яркость лампы (0 - 255)

#define MAX_BRIGHT 7      // яркость дисплея дневная (0 - 7)
#define MIN_BRIGHT 1      // яркость дисплея ночная (0 - 7)
#define	NIGHT_START 23	  // час перехода на ночную подсветку (MIN_BRIGHT)
#define NIGHT_END 7       // час перехода на дневную подсветку (MAX_BRIGHT)
#define LED_BRIGHT 50     // яркость светодиода индикатора (0 - 255)

#define ENCODER_TYPE 1    // тип энкодера (0 или 1). Типы энкодеров расписаны на странице проекта

// ************ ПИНЫ ************
#define CLKe 8        // энкодер
#define DTe 9         // энкодер
#define SWe 10        // энкодер

#define CLK 11        // дисплей
#define DIO 12        // дисплей

#define ZERO_PIN 2    // пин детектора нуля (Z-C) для диммера (если он используется)
#define DIM_PIN 3     // мосфет / DIM(PWM) пин диммера

#define BUZZ_PIN 7    // пищалка (по желанию)
#define LED_PIN 6	    // светодиод индикатор

// ***************** ОБЪЕКТЫ И ПЕРЕМЕННЫЕ *****************
#include "GyverTimer.h"
GTimer_ms halfsTimer(500);
GTimer_ms blinkTimer(800);
GTimer_ms timeoutTimer(15000);
GTimer_ms dutyTimer((long)DAWN_TIME * 60 * 1000 / (DAWN_MAX - DAWN_MIN));
GTimer_ms alarmTimeout((long)ALARM_TIMEOUT * 1000);

#include "GyverEncoder.h"
Encoder enc(CLKe, DTe, SWe);

#include "GyverTM1637.h"
GyverTM1637 disp(CLK, DIO);

#include "EEPROM.h"
#include <CyberLib.h> // шустрая библиотека для таймера

#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc;

boolean dotFlag, alarmFlag, minuteFlag, blinkFlag, newTimeFlag;
int8_t hrs = 21, mins = 55, secs;
int8_t alm_hrs, alm_mins;
int8_t dwn_hrs, dwn_mins;
byte mode;  // 0 - часы, 1 - уст. будильника, 2 - уст. времени

boolean dawn_start = false;
boolean alarm = false;
volatile int tic, duty;

void setup() {
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

  rtc.begin();
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  disp.displayClock(hrs, mins);
  alm_hrs = EEPROM.read(0);
  alm_mins = EEPROM.read(1);
  alarmFlag = EEPROM.read(2);
  alm_hrs = constrain(alm_hrs, 0, 23);
  alm_mins = constrain(alm_mins, 0, 59);
  calculateDawn();      // расчёт времени рассвета
  alarmFlag = constrain(alarmFlag, 0, 1);

  // установка яркости от времени суток
  if ( (hrs >= NIGHT_START && hrs <= 23)
       || (hrs >= 0 && hrs <= NIGHT_END) ) disp.brightness(MIN_BRIGHT);
  else disp.brightness(MAX_BRIGHT);
}

void loop() {
  encoderTick();  // отработка энкодера
  clockTick();    // считаем время
  alarmTick();    // обработка будильника
  settings();     // настройки
  dutyTick();     // управление лампой

  if (minuteFlag && mode == 0 && !alarm) {    // если новая минута и стоит режим часов и не орёт будильник
    minuteFlag = false;
    // выводим время
    if (CLOCK_EFFECT == 0) disp.displayClock(hrs, mins);
    else if (CLOCK_EFFECT == 1) disp.displayClockScroll(hrs, mins, 70);
    else disp.displayClockTwist(hrs, mins, 35);
  }
}

void calculateDawn() {
  // расчёт времени рассвета
  if (alm_mins > DAWN_TIME) {         // если минут во времени будильника больше продолжительности рассвета
    dwn_hrs = alm_hrs;                // час рассвета равен часу будильника
    dwn_mins = alm_mins - DAWN_TIME;  // минуты рассвета = минуты будильника - продолж. рассвета
  } else {                            // если минут во времени будильника меньше продолжительности рассвета
    dwn_hrs = alm_hrs - 1;            // значит рассвет будет часом раньше
    if (dwn_hrs < 0) dwn_hrs = 23;    // защита от совсем поехавших
    dwn_mins = 60 - (DAWN_TIME - alm_mins);   // находим минуту рассвета в новом часе
  }
}

void dutyTick() {
  if (dawn_start || alarm) {        // если рассвет или уже будильник
    if (DAWN_TYPE) {                // если мосфет
      analogWrite(DIM_PIN, duty);   // жарим ШИМ
    }
  }
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

void settings() {
  // *********** РЕЖИМ УСТАНОВКИ БУДИЛЬНИКА **********
  if (mode == 1) {
    if (timeoutTimer.isReady()) mode = 0;   // если сработал таймаут, вернёмся в режим 0
    if (enc.isRight()) {
      alm_mins++;
      if (alm_mins > 59) {
        alm_mins = 0;
        alm_hrs++;
        if (alm_hrs > 23) alm_hrs = 0;
      }
    }
    if (enc.isLeft()) {
      alm_mins--;
      if (alm_mins < 0) {
        alm_mins = 59;
        alm_hrs--;
        if (alm_hrs < 0) alm_hrs = 23;
      }
    }
    if (enc.isRightH()) {
      alm_hrs++;
      if (alm_hrs > 23) alm_hrs = 0;
    }
    if (enc.isLeftH()) {
      alm_hrs--;
      if (alm_hrs < 0) alm_hrs = 23;
    }
    if (enc.isTurn() && !blinkFlag) {     // вывести свежие изменения при повороте
      disp.displayClock(alm_hrs, alm_mins);
      timeoutTimer.reset();               // сбросить таймаут
    }
    if (blinkTimer.isReady()) {
      if (blinkFlag) {
        blinkFlag = false;
        blinkTimer.setInterval(700);
        disp.point(1);
        disp.displayClock(alm_hrs, alm_mins);
      } else {
        blinkFlag = true;
        blinkTimer.setInterval(300);
        disp.point(0);
        disp.clear();
      }
    }
  }

  // *********** РЕЖИМ УСТАНОВКИ ВРЕМЕНИ **********
  if (mode == 2) {
    if (timeoutTimer.isReady()) mode = 0;   // если сработал таймаут, вернёмся в режим 0
    if (!newTimeFlag) newTimeFlag = true;   // флаг на изменение времени
    if (enc.isRight()) {
      mins++;
      if (mins > 59) {
        mins = 0;
        hrs++;
        if (hrs > 23) hrs = 0;
      }
    }
    if (enc.isLeft()) {
      mins--;
      if (mins < 0) {
        mins = 59;
        hrs--;
        if (hrs < 0) hrs = 23;
      }
    }
    if (enc.isRightH()) {
      hrs++;
      if (hrs > 23) hrs = 0;
    }
    if (enc.isLeftH()) {
      hrs--;
      if (hrs < 0) hrs = 23;
    }
    if (enc.isTurn() && !blinkFlag) { // вывести свежие изменения при повороте
      disp.displayClock(hrs, mins);
      timeoutTimer.reset();           // сбросить таймаут
    }
    if (blinkTimer.isReady()) {
      // прикол с перенастройкой таймера, чтобы цифры дольше горели
      disp.point(1);
      if (blinkFlag) {
        blinkFlag = false;
        blinkTimer.setInterval(700);
        disp.displayClock(hrs, mins);
      } else {
        blinkFlag = true;
        blinkTimer.setInterval(300);
        disp.clear();
      }
    }
  }
}

void encoderTick() {
  enc.tick();   // работаем с энкодером
  // *********** КЛИК ПО ЭНКОДЕРУ **********
  if (enc.isClick()) {        // клик по энкодеру
    minuteFlag = true;        // вывести минуты при следующем входе в режим 0
    mode++;                   // сменить режим
    if (mode > 1) {           // выход с режима установки будильника и часов
      mode = 0;
      calculateDawn();        // расчёт времени рассвета
      EEPROM.update(0, alm_hrs);
      EEPROM.update(1, alm_mins);

      // установка яркости от времени суток
      if ( (hrs >= NIGHT_START && hrs <= 23)
           || (hrs >= 0 && hrs <= NIGHT_END) ) disp.brightness(MIN_BRIGHT);
      else disp.brightness(MAX_BRIGHT);

      disp.displayClock(hrs, mins);
    }
    timeoutTimer.reset();     // сбросить таймаут
  }

  // *********** УДЕРЖАНИЕ ЭНКОДЕРА **********
  if (enc.isHolded()) {       // кнопка удержана
    minuteFlag = true;        // вывести минуты при следующем входе в режим 0
    if (dawn_start) {         // если удержана во время рассвета или будильника
      dawn_start = false;     // прекратить рассвет
      alarm = false;          // и будильник
      duty = 0;
      digitalWrite(DIM_PIN, 0);
      if (BUZZ) noTone(BUZZ_PIN);
      return;
    }
    if (mode == 0 && !dawn_start) {   // кнопка удержана в режиме часов и сейчас не рассвет
      disp.point(0);              // гасим кнопку
      alarmFlag = !alarmFlag;     // переключаем будильник
      if (alarmFlag) {
        disp.scrollByte(_empty, _o, _n, _empty, 70);
        analogWrite(LED_PIN, LED_BRIGHT);
      } else {
        disp.scrollByte(_empty, _o, _F, _F, 70);
        digitalWrite(LED_PIN, 0);
      }
      EEPROM.update(2, alarmFlag);
      delay(1000);
      disp.displayClockScroll(hrs, mins, 70);
    } else if (mode == 1) {   // кнопка удержана в режиме настройки будильника
      mode = 2;               // сменить режим
    } else if (mode == 2) {	  // кнопка удержана в режиме настройки часов
      mode = 0;               // сменить режим

      // установка яркости от времени суток
      if ( (hrs >= NIGHT_START && hrs <= 23)
           || (hrs >= 0 && hrs <= NIGHT_END) ) disp.brightness(MIN_BRIGHT);
      else disp.brightness(MAX_BRIGHT);

      disp.displayClock(hrs, mins);
    }
    timeoutTimer.reset();     // сбросить таймаут
  }
}

void alarmTick() {
  if (dawn_start && alarmFlag) {
    if (dutyTimer.isReady()) {    // поднимаем яркость по таймеру
      duty++;
      if (duty > DAWN_MAX) duty = DAWN_MAX;
    }
  }
  if (alarm) {                    // настало время будильника
    if (alarmTimeout.isReady()) { // таймаут будильника
      dawn_start = false;         // прекратить рассвет
      alarm = false;              // и будильник
      duty = 0;
      digitalWrite(DIM_PIN, 0);
      if (BUZZ) noTone(BUZZ_PIN);
    }
    if (blinkTimer.isReady()) {   // мигаем цифрами
      if (blinkFlag) {
        blinkFlag = false;
        blinkTimer.setInterval(700);
        disp.point(1);
        disp.displayClock(hrs, mins);
        if (ALARM_BLINK) duty = DAWN_MAX;     // мигаем светом
        if (BUZZ) tone(BUZZ_PIN, BUZZ_FREQ);  // пищим
      } else {
        blinkFlag = true;
        blinkTimer.setInterval(300);
        disp.point(0);
        disp.clear();
        if (ALARM_BLINK) duty = DAWN_MIN;
        if (BUZZ) noTone(BUZZ_PIN);
      }
    }
  }
}

void clockTick() {
  if (halfsTimer.isReady()) {
    if (newTimeFlag) {
      newTimeFlag = false;
      secs = 0;
      rtc.adjust(DateTime(2014, 1, 21, hrs, mins, 0)); // установка нового времени в RTC
    }
    dotFlag = !dotFlag;
    if (mode == 0) disp.point(dotFlag);                 // выкл/выкл точки
    if (mode == 0) disp.displayClock(hrs, mins);        // костыль, без него подлагивает дисплей
    if (alarmFlag) {
      if (dotFlag) analogWrite(LED_PIN, LED_BRIGHT);    // мигаем светодиодом что стоит аларм
      else digitalWrite(LED_PIN, 0);
    }

    if (dotFlag) {          // каждую секунду пересчёт времени
      secs++;
      if (secs > 59) {      // каждую минуту
        secs = 0;
        mins++;
        minuteFlag = true;
      }
      if (mins > 59) {      // каждый час
        DateTime now = rtc.now();
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();

        // меняем яркость
        if (hrs == NIGHT_START) disp.brightness(MIN_BRIGHT);
        if (hrs == NIGHT_END) disp.brightness(MAX_BRIGHT);
        //mins = 0;
        //hrs++;
        //if (hrs > 23) hrs = 0;  // сутки!
      }

      // после пересчёта часов проверяем будильник!
      if (dotFlag) {
        if (dwn_hrs == hrs && dwn_mins == mins && alarmFlag && !dawn_start) {
          duty = DAWN_MIN;
          dawn_start = true;
        }
        if (alm_hrs == hrs && alm_mins == mins && alarmFlag && dawn_start && !alarm) {
          alarm = true;
          alarmTimeout.reset();
        }
      }
    }
  }
}
