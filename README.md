![PROJECT_PHOTO](https://github.com/AlexGyver/dawn-clock/blob/master/proj_img.jpg)
# Часы-рассвет на Arduino
* [Описание проекта](#chapter-0)
* [Папки проекта](#chapter-1)
* [Схемы подключения](#chapter-2)
* [Материалы и компоненты](#chapter-3)
* [Как скачать и прошить](#chapter-4)
* [FAQ](#chapter-5)
* [Полезная информация](#chapter-6)
[![AlexGyver YouTube](http://alexgyver.ru/git_banner.jpg)](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)

<a id="chapter-0"></a>
## Описание проекта
Часы с будильником и источником света, который имитирует рассвет  
#### Особенности:
 - Плавно включает свет до будильника
 - Поддерживает светильники постоянного и переменного тока
 - Имеет кучу настроек "под себя", начиная от яркости, заканчивая эффектами переключения на дисплее

<a id="chapter-1"></a>
## Папки
**ВНИМАНИЕ! Если это твой первый опыт работы с Arduino, читай [инструкцию](#chapter-4)**
- **libraries** - библиотеки проекта. Заменить имеющиеся версии (в этом проекте внешних библиотек нет)
- **firmware** - прошивки для Arduino
- **schemes** - схемы подключения компонентов

<a id="chapter-2"></a>
## Схемы
![SCHEME](https://github.com/AlexGyver/dawn-clock/blob/master/schemes/scheme1.jpg)
![SCHEME](https://github.com/AlexGyver/dawn-clock/blob/master/schemes/scheme2.jpg)

<a id="chapter-3"></a>
## Материалы и компоненты
### Ссылки оставлены на магазины, с которых я закупаюсь уже не один год
Источники света лучше брать "тёплые", т.к. рассвет именно жёлтый!
* Arduino NANO 328p – искать
* https://ali.ski/tI7blh
* https://ali.ski/O4yTxb
* https://ali.ski/6_rFIS
* https://ali.ski/gb92E-
* Giant4 (Россия)
* БП 5V Али искать
* https://ali.ski/DItEG
* https://ali.ski/t3YFfU
* БП 12 Вольт https://ali.ski/WOWezD
* RTC DS3231
* https://ali.ski/lqQM2d
* https://ali.ski/u6mZas
* Дисплей – искать
* https://ali.ski/8pQi9M
* https://ali.ski/-nBZs
* https://ali.ski/wumit
* Энкодер – искать
* https://ali.ski/c-hjE
* https://ali.ski/OygY3d
* https://ali.ski/II_efl
* MOSFET модуль – искать
* https://ali.ski/SMPgB
* Диммер модуль https://ali.ski/mjLKl
* Лента светодиодная https://ali.ski/Vtkc_F
* Матрица 220V https://ali.ski/zoe1HT
* Резисторы и светодиоды можно найти в ЛЮБОМ магазине радиодеталей или радиорынке. Хотите китайский опт? Держите!
* Светодиоды https://ali.ski/tAUCB
* Резисторы 220 Ом (220 R) https://ali.ski/TAN2C
* Резисторы 100 Ом (100 R)https://ali.ski/TAN2C
* Пищалка https://ali.ski/oL5Ugs
* Провода https://ali.ski/rto9T5

## Вам скорее всего пригодится
* [Всё для пайки (паяльники и примочки)](http://alexgyver.ru/all-for-soldering/)
* [Недорогие инструменты](http://alexgyver.ru/my_instruments/)
* [Все существующие модули и сенсоры Arduino](http://alexgyver.ru/arduino_shop/)
* [Электронные компоненты](http://alexgyver.ru/electronics/)
* [Аккумуляторы и зарядные модули](http://alexgyver.ru/18650/)

<a id="chapter-4"></a>
## Как скачать и прошить
* [Первые шаги с Arduino](http://alexgyver.ru/arduino-first/) - ультра подробная статья по началу работы с Ардуино, ознакомиться первым делом!
* Скачать архив с проектом
> На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**
* Установить библиотеки в  
`C:\Program Files (x86)\Arduino\libraries\` (Windows x64)  
`C:\Program Files\Arduino\libraries\` (Windows x86)
* **Подключить внешнее питание 5 Вольт**
* Подключить Ардуино к компьютеру
* Запустить файл прошивки (который имеет расширение .ino)
* Настроить IDE (COM порт, модель Arduino, как в статье выше)
* Настроить что нужно по проекту
* Нажать загрузить
* Скачать GyverMatrixBT с Play Market
* Пользоваться  

## Настройки в коде
    DAWN_TIME 20      // продолжительность рассвета (в минутах)
    ALARM_TIMEOUT 80  // таймаут на автоотключение будильника, секунды
    ALARM_BLINK 0     // 1 - мигать лампой при будильнике, 0 - не мигать
    CLOCK_EFFECT 1    // эффект перелистывания часов: 0 - обычный, 1 - прокрутка, 2 - скрутка
    BUZZ 1            // пищать пищалкой (1 вкл, 0 выкл)
    BUZZ_FREQ 800     // частота писка (Гц)

    DAWN_TYPE 0       // 1 - мосфет (DC диммер), 0 - симистор (AC диммер) СМОТРИ СХЕМЫ
    DAWN_MIN 50       // начальная яркость лампы (0 - 255) (для сетевых матриц начало света примерно с 50)
    DAWN_MAX 200      // максимальная яркость лампы (0 - 255)

    MAX_BRIGHT 7      // яркость дисплея дневная (0 - 7)
    MIN_BRIGHT 1      // яркость дисплея ночная (0 - 7)
    NIGHT_START 23	  // час перехода на ночную подсветку (MIN_BRIGHT)
    NIGHT_END 7       // час перехода на дневную подсветку (MAX_BRIGHT)
    LED_BRIGHT 50     // яркость светодиода индикатора (0 - 255)

    ENCODER_TYPE 1    // тип энкодера (0 или 1). Типы энкодеров расписаны на странице проекта
	
<a id="chapter-5"></a>
## FAQ
### Основные вопросы
В: Как скачать с этого грёбаного сайта?  
О: На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**

В: Скачался какой то файл .zip, куда его теперь?  
О: Это архив. Можно открыть стандартными средствами Windows, но думаю у всех на компьютере установлен WinRAR, архив нужно правой кнопкой и извлечь.

В: Я совсем новичок! Что мне делать с Ардуиной, где взять все программы?  
О: Читай и смотри видос http://alexgyver.ru/arduino-first/

В: Вылетает ошибка загрузки / компиляции!
О: Читай тут: https://alexgyver.ru/arduino-first/#step-5

В: Сколько стоит?  
О: Ничего не продаю.

### Вопросы по этому проекту

<a id="chapter-6"></a>
## Полезная информация
* [Мой сайт](http://alexgyver.ru/)
* [Основной YouTube канал](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)
* [YouTube канал про Arduino](https://www.youtube.com/channel/UC4axiS76D784-ofoTdo5zOA?sub_confirmation=1)
* [Мои видеоуроки по пайке](https://www.youtube.com/playlist?list=PLOT_HeyBraBuMIwfSYu7kCKXxQGsUKcqR)
* [Мои видеоуроки по Arduino](http://alexgyver.ru/arduino_lessons/)