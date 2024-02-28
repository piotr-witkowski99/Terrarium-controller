#include <EEPROM.h>
#include <DHT.h>
#include <DS3231.h>
#include <LiquidCrystal.h>

#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int licznik = 0;
bool dzienNoc = 0;
bool opcjaT = false;
bool opcjaW = false;
bool opcjaO = false;
bool opcjaNoc = false;
float jakDlugo = 0;
int zmianaCzas = 0;
bool set = 0;
bool dzien = false;
float dayTemp;
float dayWilg;
float nightTemp;
float nightWilg;
int wybor = 0;

const int rs = 22, en = 23, d4 = 24, d5 = 25, d6 = 26, d7 = 27;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int buttonZmiana = 9;
int buttonGora = 11;
int buttonDol = 10;
int buttonZatwierdz = 8;

DS3231  rtc(SDA, SCL);
Time t, d;

byte termometr[8] = {
  0b00100,
  0b00110,
  0b00100,
  0b00110,
  0b00100,
  0b01110,
  0b01110,
  0b00000
};

byte kropla[8] = {
  0b00100,
  0b00100,
  0b01110,
  0b01110,
  0b10111,
  0b10111,
  0b01110,
  0b00000
};

byte stopnie[8] = {
  0b00010,
  0b00101,
  0b00010,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte strzalkaDown[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};
byte strzalkaUp[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00000
};


int dioda_1 = 52;

int dioda_R = 5;
int dioda_G = 3;
int dioda_B = 2;
int grzalka = 38;
int nawilzacz = 39;

int godzDzien;
int minDzien;
int godzNoc;
int minNoc;

void setup() {

  odczyt();

  pinMode(buttonZmiana, INPUT_PULLUP);
  pinMode(buttonGora, INPUT_PULLUP);
  pinMode(buttonDol, INPUT_PULLUP);
  pinMode(buttonZatwierdz, INPUT_PULLUP);
  Serial.begin(9600);

  rtc.begin();
  dht.begin();

  lcd.begin(16, 2);
  lcd.home();
  lcd.createChar(0, termometr);
  lcd.createChar(1, kropla);
  lcd.createChar(2, stopnie);
  lcd.createChar(3, strzalkaUp);
  lcd.createChar(4, strzalkaDown);

  pinMode(dioda_1, OUTPUT);
  pinMode(dioda_R, OUTPUT);
  pinMode(dioda_G, OUTPUT);
  pinMode(dioda_B, OUTPUT);
  pinMode(grzalka, OUTPUT);
  pinMode(nawilzacz, OUTPUT);

}

void loop() {

  float wilg = dht.readHumidity();
  float temp = dht.readTemperature();
  dzien = czyDzien(godzDzien, minDzien, godzNoc, minNoc);

  if (dzien == true) {
    wDzien(temp, wilg);
  }
  else {
    wNoc(temp, wilg);
  }

  lcd.clear();
  t = rtc.getTime();

  if (czyWcisniety(buttonZatwierdz)) {
    set = !set;
    delay(200);
  }
  if (set == 1) {
    ustawienia();
  }

  lcd.clear();
  lcd.print(rtc.getTimeStr(FORMAT_SHORT));
  lcd.print(" ");
  lcd.print(rtc.getDateStr());

  lcd.setCursor(0, 2);
  lcd.write((byte)0);
  lcd.print(temp);
  lcd.setCursor(5, 2);
  lcd.write((byte)2);
  lcd.print("C");
  lcd.setCursor(10, 2);
  lcd.write((byte)1);
  lcd.print(wilg);
  lcd.setCursor(15, 2);
  lcd.print("%");

  delay(200);
}


bool czyDzien(int godzDzien, int minDzien, int godzNoc, int minNoc) {
  t = rtc.getTime();
  while (true) {
    if (godzDzien < godzNoc) {
      if (t.hour >= godzDzien && t.hour <= godzNoc) {
        if (t.hour == godzDzien && t.min >= minDzien) {
          return true;
          break;
        }
        if (t.hour == godzNoc && t.min < minNoc) {
          return true;
          break;
        }
      }
      if (t.hour > godzDzien && t.hour < godzNoc) {
        return true;
        break;
      }
    }
    else if (godzDzien == godzNoc) {
      if (minDzien < minNoc) {
        if (t.min >= minDzien && t.min < minNoc) {
          return true;
          break;
        }
      }
      else if (minDzien == minNoc) {
        return false;
        break;
      }
      else if (minDzien > minNoc) {
        if ((t.hour >= godzDzien || t.hour <= godzNoc) && (t.min >= minDzien && t.min < minNoc)) {
          return true;
          break;
        }
      }
    }
    else if (godzDzien > godzNoc) {
      if (t.hour >= godzDzien || t.hour <= godzNoc) {
        if (t.hour == godzDzien && t.min >= minDzien) {
          return true;
          break;
        }
        if (t.hour == godzNoc && t.min < minNoc) {
          return true;
          break;
        }
        if (t.hour > godzDzien || t.hour < godzNoc) {
          return true;
          break;
        }
      }
    }
    return false;
    break;
  }
}

void wDzien(float temp, float wilg) {

  digitalWrite(dioda_1, HIGH);
  if (opcjaO) {
    opcjaOswi();
  }
  else {
    analogWrite(dioda_R, 255);
    analogWrite(dioda_G, 255);
    analogWrite(dioda_B, 255);
  }
  if (opcjaT) {
    opcjaTemp(temp);
  }
  else {
    if (temp <= dayTemp - 0.5) {
      digitalWrite(grzalka, LOW);
    }
    if (temp > dayTemp + 0.5 ) {
      digitalWrite(grzalka, HIGH);
    }
  }

  if (opcjaW) {
    opcjaWilg(wilg);
  }
  else {
    if (wilg <= dayWilg - 5) {
      digitalWrite(nawilzacz, LOW);
    }
    if (wilg > dayWilg + 5) {
      digitalWrite(nawilzacz, HIGH);
    }
  }
}

void wNoc(float temp, float wilg) {

  if (opcjaNoc) {
    oswietlenieNoc();
  }
  else {
    digitalWrite(dioda_R, LOW);
    digitalWrite(dioda_G, LOW);
    digitalWrite(dioda_B, LOW);
  }

  digitalWrite(dioda_1, LOW);
  if (opcjaT) {
    opcjaTemp(temp);
  }
  else {
    if (temp <= nightTemp - 1) {
      digitalWrite(grzalka, LOW);
    }
    if (temp > nightTemp) {
      digitalWrite(grzalka, HIGH);
    }
  }
  if (opcjaW) {
    opcjaWilg(wilg);
  }
  else {
    if (wilg <= nightWilg - 5) {
      digitalWrite(nawilzacz, LOW);
    }
    if (wilg > nightWilg) {
      digitalWrite(nawilzacz, HIGH);
    }
  }
}

void ustawienia() {

  while (true) {
    if (set == 0) {
      break;
    }

    if (czyWcisniety(buttonGora)) {
      wybor = wartoscGora(wybor, 7);
      delay(200);
    }
    if (czyWcisniety(buttonDol)) {
      wybor = wartoscDol(wybor, 7);
      delay(200);
    }

    switch (wybor)
    {
      case 0:                                                   // ustawianie czasu dzienego i nocego

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ustaw DZIEN/NOC");
        lcd.setCursor(0, 2);
        lcd.write((byte)3);
        lcd.print(" ");
        lcd.write((byte)4);

        delay(200);
        while (true) {
          if (czyWcisniety(buttonGora)) {
            delay(200);
            break;
          }
          if (czyWcisniety(buttonDol)) {
            delay(200);
            break;
          }
          if (licznik >= 4) {
            licznik = 0;
            set = 1;
            zapis();
            delay(200);
            break;

          }
          if (czyWcisniety(buttonZatwierdz)) {
            set = !set;
            delay(200);
          }
          if (set == 0) {
            while (licznik < 4) {
              switch (licznik) {
                case 0:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Poczatek dnia");
                  lcd.setCursor(0, 2);
                  lcd.print("(Godz)");
                  lcd.setCursor(11, 2);
                  wyswietlCzas(godzDzien, minDzien);

                  if (czyWcisniety(buttonGora)) {
                    godzDzien = wartoscGora(godzDzien, 23);
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    godzDzien = wartoscDol(godzDzien, 23);
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);

                  }
                  break;

                case 1:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Poczatek dnia");
                  lcd.setCursor(0, 2);
                  lcd.print("(Min)");
                  lcd.setCursor(11, 2);
                  wyswietlCzas(godzDzien, minDzien);

                  if (czyWcisniety(buttonGora)) {
                    minDzien = wartoscGora(minDzien, 59);
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    minDzien = wartoscDol(minDzien, 59);
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;

                case 2:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Koniec dnia");
                  lcd.setCursor(0, 2);
                  lcd.print("(Godz)");
                  lcd.setCursor(11, 2);
                  wyswietlCzas(godzNoc, minNoc);

                  if (czyWcisniety(buttonGora)) {
                    godzNoc = wartoscGora(godzNoc, 23);
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    godzNoc = wartoscDol(godzNoc, 23);
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;

                case 3:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Koniec dnia");
                  lcd.setCursor(0, 2);
                  lcd.print("(Min)");
                  lcd.setCursor(11, 2);
                  wyswietlCzas(godzNoc, minNoc);

                  if (czyWcisniety(buttonGora)) {
                    minNoc = wartoscGora(minNoc, 59);
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    minNoc = wartoscDol(minNoc, 59);
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;


                default:
                  break;
              }
            }
          }
        }
        break;


      case 1:                                                   // ustawianie temperatury dzien/noc
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ustaw temp ");
        lcd.write((byte)0);
        lcd.setCursor(0, 2);
        lcd.write((byte)3);
        lcd.print(" ");
        lcd.write((byte)4);
        delay(200);

        while (true) {
          if (czyWcisniety(buttonGora)) {
            delay(200);
            break;

          }
          if (czyWcisniety(buttonDol)) {
            delay(200);
            break;

          }
          if (licznik >= 2) {
            licznik = 0;
            set = 1;
            zapis();
            delay(200);
            break;

          }
          if (czyWcisniety(buttonZatwierdz)) {
            set = !set;
            delay(200);
          }
          if (set == 0) {
            while (licznik < 2) {
              switch (licznik) {
                case 0:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Temp dzien");
                  lcd.setCursor(0, 2);
                  lcd.print(dayTemp);
                  lcd.setCursor(5, 2);
                  lcd.write((byte)2);
                  lcd.print("C");
                  delay(200);

                  if (czyWcisniety(buttonGora)) {
                    dayTemp = dayTemp + 0.5;

                    if (dayTemp > 40) {
                      dayTemp = dayTemp - (40 + 1);
                    }
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    dayTemp = dayTemp - 0.5;

                    if (dayTemp < 0) {
                      dayTemp = dayTemp + (40 + 1);
                    }
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);

                  }
                  break;

                case 1:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Temp noc");
                  lcd.setCursor(0, 2);
                  lcd.print(nightTemp);
                  lcd.setCursor(5, 2);
                  lcd.write((byte)2);
                  lcd.print("C");
                  delay(200);

                  if (czyWcisniety(buttonGora)) {
                    nightTemp = nightTemp + 0.5;

                    if (nightTemp > 40) {
                      nightTemp = nightTemp - (40 + 1);
                    }
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    nightTemp = nightTemp - 0.5;

                    if (nightTemp < 0) {
                      nightTemp = nightTemp + (40 + 1);
                    }
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;

                default:
                  break;
              }
            }
          }
        }
        break;


      case 2:                                                   // ustawianie wilgotnosci dzien/noc
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ustaw wilgotnosc");
        lcd.setCursor(0, 2);
        lcd.write((byte)3);
        lcd.print(" ");
        lcd.write((byte)4);
        delay(200);
        while (true) {
          if (czyWcisniety(buttonGora)) {
            delay(200);
            break;

          }
          if (czyWcisniety(buttonDol)) {
            delay(200);
            break;

          }
          if (licznik >= 2) {
            licznik = 0;
            set = 1;
            zapis();
            delay(200);
            break;

          }
          if (czyWcisniety(buttonZatwierdz)) {
            set = !set;
            delay(200);
          }
          if (set == 0) {
            while (licznik < 2) {
              switch (licznik) {
                case 0:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Wilgotnosc dzien");
                  lcd.setCursor(0, 2);
                  lcd.print(dayWilg);
                  lcd.setCursor(5, 2);
                  lcd.print("%");
                  delay(200);

                  if (czyWcisniety(buttonGora)) {
                    dayWilg = wartoscGora(dayWilg, 94);
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    dayWilg = wartoscDol(dayWilg, 94);
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);

                  }
                  break;

                case 1:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Wilgotnosc noc");
                  lcd.setCursor(0, 2);
                  lcd.print(nightWilg);
                  lcd.setCursor(5, 2);
                  lcd.print("%");
                  delay(200);

                  if (czyWcisniety(buttonGora)) {
                    nightWilg = wartoscGora(nightWilg, 94);
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    nightWilg = wartoscDol(nightWilg, 94);
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);

                  }
                  break;

                default:
                  break;
              }
            }
          }
        }
        break;

      case 3:                                                     //opcje temperatury
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Opcje temperatury");
        lcd.setCursor(0, 2);
        lcd.write((byte)3);
        lcd.print(" ");
        lcd.write((byte)4);
        delay(200);

        while (true) {
          if (czyWcisniety(buttonGora)) {
            delay(200);
            break;

          }
          if (czyWcisniety(buttonDol)) {
            delay(200);
            break;

          }
          if (licznik >= 1) {
            licznik = 0;
            set = 1;
            zapis();
            delay(200);
            break;

          }
          if (czyWcisniety(buttonZatwierdz)) {
            set = !set;
            delay(200);
          }
          if (set == 0) {
            while (licznik < 2) {

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Opcja temp jest:");
              lcd.setCursor(0, 1);
              if (opcjaT == true) {
                lcd.print("wlaczona");
              }
              else {
                lcd.print("wylaczona");
              }
              delay(200);

              if (czyWcisniety(buttonGora)) {
                opcjaT = !opcjaT;
                delay(200);
              }
              else if (czyWcisniety(buttonDol)) {
                opcjaT = !opcjaT;
                delay(200);
              }
              if (czyWcisniety(buttonZatwierdz)) {
                licznik++;
                delay(200);
              }
            }
          }
        }
        break;                  /// tu koniec


      case 4:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Opcje wilgotnosci");
        lcd.setCursor(0, 2);
        lcd.write((byte)3);
        lcd.print(" ");
        lcd.write((byte)4);
        delay(200);
        while (true) {
          if (czyWcisniety(buttonGora)) {
            delay(200);
            break;

          }
          if (czyWcisniety(buttonDol)) {
            delay(200);
            break;

          }
          if (licznik >= 1) {
            licznik = 0;
            set = 1;
            zapis();
            delay(200);
            break;

          }
          if (czyWcisniety(buttonZatwierdz)) {
            set = !set;
            delay(200);
          }
          if (set == 0) {
            while (licznik < 1) {

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Opcja temp jest:");
              lcd.setCursor(0, 2);
              if (opcjaW == true) {
                lcd.print("wlaczona");
              }
              else {
                lcd.print("wylaczona");
              }
              delay(200);

              if (czyWcisniety(buttonGora)) {
                opcjaW = !opcjaW;
                delay(200);
              }
              else if (czyWcisniety(buttonDol)) {
                opcjaW = !opcjaW;
                delay(200);
              }
              if (czyWcisniety(buttonZatwierdz)) {
                licznik++;
                delay(200);
              }
            }
          }
        }
        break;
      case 5:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Opcje oswietlenia");
        lcd.setCursor(0, 2);
        lcd.write((byte)3);
        lcd.print(" ");
        lcd.write((byte)4);
        delay(200);
        while (true) {
          if (czyWcisniety(buttonGora)) {
            delay(200);
            break;

          }
          if (czyWcisniety(buttonDol)) {
            delay(200);
            break;

          }
          if (licznik > 1) {
            licznik = 0;
            set = 1;
            zapis();
            delay(200);
            break;

          }
          if (czyWcisniety(buttonZatwierdz)) {
            set = !set;
            delay(200);
          }
          if (set == 0) {
            while (licznik < 3) {
              switch (licznik) {
                case 0:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Opcja oswie jest:");
                  lcd.setCursor(0, 2);
                  if (opcjaO == true) {
                    lcd.print("wlaczona");
                  }
                  else {
                    lcd.print("wylaczona");
                  }
                  delay(200);

                  if (czyWcisniety(buttonGora)) {
                    opcjaO = !opcjaO;
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    opcjaO = !opcjaO;
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;

                case 1:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Oswietlenie noc:");
                  lcd.setCursor(0, 2);
                  if (opcjaNoc == true) {
                    lcd.print("wlaczona");
                  }
                  else {
                    lcd.print("wylaczona");
                  }
                  delay(200);

                  if (czyWcisniety(buttonGora)) {
                    opcjaNoc = !opcjaNoc;
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    opcjaNoc = !opcjaNoc;
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;

                case 2:
                  if (!opcjaNoc) {
                    licznik++;
                  }
                  else {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Ile oswie noc ?");
                    lcd.setCursor(0, 2);
                    lcd.print("(godz)");
                    lcd.setCursor(11, 2);
                    lcd.print(jakDlugo);
                    lcd.setCursor(14, 2);
                    lcd.print("  ");
                    delay(200);
                    if (czyWcisniety(buttonGora)) {
                      jakDlugo = jakDlugo + 0.5;

                      if (jakDlugo > (ileMinutMiedzyGodzinami(godzNoc, minNoc, godzDzien, minDzien) / 60)) {
                        jakDlugo = jakDlugo - ((ileMinutMiedzyGodzinami(godzNoc, minNoc, godzDzien, minDzien) / 60) + 0.5);
                      }
                      delay(200);
                    }
                    else if (czyWcisniety(buttonDol)) {
                      jakDlugo = jakDlugo - 0.5;

                      if (jakDlugo < 0) {
                        jakDlugo = jakDlugo + ((ileMinutMiedzyGodzinami(godzNoc, minNoc, godzDzien, minDzien) / 60) + 0.5);
                      }
                      delay(200);
                    }
                    else if (czyWcisniety(buttonZatwierdz)) {
                      licznik++;
                      delay(200);
                    }
                  }
                  break;

                default:
                  break;
              }


            }
          }
        }
        break;

      case 6:                                                   // ustawianie daty i godziny
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ustaw GODZ/DATA");
        lcd.setCursor(0, 2);
        lcd.write((byte)3);
        lcd.print(" ");
        lcd.write((byte)4);
        delay(200);
        while (true) {
          if (czyWcisniety(buttonGora)) {
            delay(200);
            break;
          }
          if (czyWcisniety(buttonDol)) {
            delay(200);
            break;
          }
          if (licznik >= 5) {
            licznik = 0;
            set = 1;
            zapis();
            delay(200);
            break;

          }
          if (czyWcisniety(buttonZatwierdz)) {
            set = !set;
            t = rtc.getTime();
            delay(200);
          }
          if (set == 0) {
            int setHour = t.hour;
            int setMin = t.min;
            int setDay = t.date;
            int setMo = t.mon;
            int setYear = t.year;

            while (licznik < 5) {
              switch (licznik) {
                case 0:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Ustaw godzine");
                  lcd.setCursor(0, 2);
                  lcd.print("(h)");
                  lcd.setCursor(11, 2);
                  wyswietlCzas(setHour, setMin);

                  if (czyWcisniety(buttonGora)) {
                    setHour = wartoscGora(setHour, 23);
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    setHour = wartoscDol(setHour, 23);
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);

                  }
                  break;

                case 1:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Ustaw godzine");
                  lcd.setCursor(0, 2);
                  lcd.print("(Min)");
                  lcd.setCursor(11, 2);
                  wyswietlCzas(setHour, setMin);

                  if (czyWcisniety(buttonGora)) {
                    setMin = wartoscGora(setMin, 59);
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    setMin = wartoscDol(setMin, 59);
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;

                case 2:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Ustaw date");
                  lcd.setCursor(0, 2);
                  lcd.print("(DD)");
                  lcd.setCursor(6, 2);
                  lcd.print(setDay);
                  lcd.print(":");
                  lcd.print(setMo);
                  lcd.print(":");
                  lcd.print(setYear);
                  delay(200);


                  if (czyWcisniety(buttonGora)) {
                    setDay = wartoscGora(setDay, 31);
                    if (setDay == 0) {
                      setDay++;
                    }
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    setDay = wartoscDol(setDay, 31);
                    if (setDay == 0) {
                      setDay++;
                    }
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;

                case 3:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Ustaw date");
                  lcd.setCursor(0, 2);
                  lcd.print("(MM)");
                  lcd.setCursor(6, 2);
                  lcd.print(setDay);
                  lcd.print(":");
                  lcd.print(setMo);
                  lcd.print(":");
                  lcd.print(setYear);
                  delay(200);


                  if (czyWcisniety(buttonGora)) {
                    setMo = wartoscGora(setMo, 12);
                    if (setMo == 0) {
                      setMo++;
                    }
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    setMo = wartoscDol(setMo, 12);
                    if (setDay == 0) {
                      setMo++;
                    }
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;

                case 4:
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Ustaw date");
                  lcd.setCursor(0, 2);
                  lcd.print("(YY)");
                  lcd.setCursor(6, 2);
                  lcd.print(setDay);
                  lcd.print(":");
                  lcd.print(setMo);
                  lcd.print(":");
                  lcd.print(setYear);
                  delay(200);


                  if (czyWcisniety(buttonGora)) {
                    setYear = wartoscGora(setYear, 2100);
                    if (setYear == 0) {
                      setYear = 2010;
                    }
                    delay(200);
                  }
                  else if (czyWcisniety(buttonDol)) {
                    setYear = wartoscDol(setYear, 2100);
                    if (setDay == 2009) {
                      setYear = 2100;
                    }
                    delay(200);
                  }
                  if (czyWcisniety(buttonZatwierdz)) {
                    licznik++;
                    delay(200);
                  }
                  break;
                default:
                  break;
              }
            }
            rtc.setTime(setHour, setMin, 0);
            rtc.setDate(setDay, setMo, setYear);
          }
        }
        break;

      case 7:                                                   // powrot do ekranu głównego
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ekran glowny");
        lcd.setCursor(0, 2);
        lcd.write((byte)3);
        lcd.print(" ");
        lcd.write((byte)4);
        delay(200);
        while (true) {
          if (czyWcisniety(buttonGora)) {
            delay(200);
            break;
          }
          if (czyWcisniety(buttonDol)) {
            delay(200);
            break;
          }
          if (czyWcisniety(buttonZatwierdz)) {
            set = !set;
            wybor = 0;
            delay(200);
            break;
          }
        }
        break;


      default:
        break;
    }
  }
}


int wartoscGora(int wartosc, int maxWartosc) {
  wartosc = wartosc + 1;

  if (wartosc > maxWartosc) {
    wartosc = wartosc - (maxWartosc + 1);
  }
  return wartosc;
}

int wartoscDol(int wartosc, int maxWartosc) {
  wartosc = wartosc - 1;

  if (wartosc < 0) {
    wartosc = wartosc + (maxWartosc + 1);
  }
  return wartosc;
}

int zmiana(int wartosc, int maxWartosc) {
  wartosc = wartosc + 1;

  if (wartosc > maxWartosc) {
    wartosc = wartosc - (maxWartosc + 1);
  }
  return wartosc;
}

bool czyWcisniety(int przycisk) {
  if (digitalRead(przycisk) == LOW) {
    delay(20);
    if (digitalRead(przycisk) == LOW) {
      return true;
    }
  }
  else {
    return false;
  }
}

void wyswietlCzas(int Godz, int Min) {
  if (Godz < 10) {
    if (Min >= 10) {
      lcd.print("0");
      lcd.print(Godz);
      lcd.print(":");
      lcd.print(Min);
      delay(200);
    }
    else {
      lcd.print("0");
      lcd.print(Godz);
      lcd.print(":");
      lcd.print("0");
      lcd.print(Min);
      delay(200);
    }
  }
  else if (Godz >= 10 && Min < 10) {
    lcd.print(Godz);
    lcd.print(":");
    lcd.print("0");
    lcd.print(Min);
    delay(200);
  }
  else {
    lcd.print(Godz);
    lcd.print(":");
    lcd.print(Min);
    delay(200);
  }
}

void zapis() {

  for (int i = 0; i < 13; i++) {
    switch (i) {
      case 0:
        EEPROM.write(i, godzDzien);
        break;

      case 1:
        EEPROM.write(i, minDzien);
        break;

      case 2:
        EEPROM.write(i, godzNoc);
        break;

      case 3:
        EEPROM.write(i, minNoc);
        break;

      case 4:
        EEPROM.write(i, dayTemp);
        break;
      case 5:

        EEPROM.write(i, nightTemp);
        break;

      case 6:

        EEPROM.write(i, dayWilg);
        break;

      case 7:

        EEPROM.write(i, nightWilg);
        break;

      case 8:

        EEPROM.write(i, opcjaT);
        break;

      case 9:

        EEPROM.write(i, opcjaW);
        break;
      case 10:

        EEPROM.write(i, opcjaO);
        break;
      case 11:

        EEPROM.write(i, opcjaNoc);
        break;
      case 12:

        EEPROM.write(i, jakDlugo);
        break;

      default:
        break;
    }
  }
}
void odczyt() {
  for (int i = 0; i < 13; i++) {
    switch (i) {
      case 0:
        godzDzien = EEPROM.read(i);
        break;

      case 1:
        minDzien = EEPROM.read(i);
        break;

      case 2:
        godzNoc = EEPROM.read(i);
        break;

      case 3:
        minNoc = EEPROM.read(i);
        break;

      case 4:
        dayTemp = EEPROM.read(i);
        break;

      case 5:
        nightTemp = EEPROM.read(i);
        break;

      case 6:
        dayWilg = EEPROM.read(i);
        break;

      case 7:
        nightWilg = EEPROM.read(i);
        break;

      case 8:
        opcjaT = EEPROM.read(i);
        break;

      case 9:
        opcjaW = EEPROM.read(i);
        break;

      case 10:
        opcjaO = EEPROM.read(i);
        break;

      case 11:
        opcjaNoc = EEPROM.read(i);
        break;

      case 12:
        jakDlugo = EEPROM.read(i);
        break;

      default:
        break;
    }
  }
}

void opcjaTemp(float temp)  {

  t = rtc.getTime();
  int ileMinut = ileMinutMiedzyGodzinami(godzDzien, minDzien, godzNoc, minNoc) / 2;
  int ileMinutNoc = ileMinutMiedzyGodzinami(godzNoc, minNoc, godzDzien, minDzien) / 2;
  float ileStopniDzien = fabs(dayTemp - nightTemp);

  //dzien
  if (czyDzien(godzDzien, minDzien, godzNoc, minNoc)) {
    if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, ileMinut / 6))) {
      if (temp < (dayTemp - 0.5 - 5 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (dayTemp + 0.5 - 5 * (ileStopniDzien / 6)) ) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 3 ), dodawanieMin(godzDzien, minDzien, ileMinut / 3))) {
      if (temp < (dayTemp - 0.5 - 4 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (dayTemp + 0.5 - 4 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 2 ), dodawanieMin(godzDzien, minDzien, ileMinut / 2))) {
      if (temp < (dayTemp - 0.5 - 3 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (dayTemp + 0.5 - 3 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 2 * ileMinut / 3 ), dodawanieMin(godzDzien, minDzien, 2 * ileMinut / 3))) {
      if (temp < (dayTemp - 0.5 - 2 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (dayTemp + 0.5 - 2 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 5 * ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, 5 * ileMinut / 6))) {
      if (temp < (dayTemp - 0.5 - (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (dayTemp + 0.5 - (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else {
      if (temp < dayTemp - 0.5) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > dayTemp + 0.5 ) {
        digitalWrite(grzalka, HIGH);
      }
    }
  }
  else {
    // noc
    if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, ileMinutNoc / 6 ), dodawanieMin(godzNoc, minNoc, ileMinutNoc / 6))) {
      if (temp < (nightTemp - 0.5 + 5 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (nightTemp + 0.5 + 5 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, ileMinutNoc / 3 ), dodawanieMin(godzNoc, minNoc, ileMinutNoc / 3))) {
      if (temp < (nightTemp - 0.5 + 4 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (nightTemp + 0.5 + 4 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, ileMinutNoc / 2 ), dodawanieMin(godzNoc, minNoc, ileMinutNoc / 2))) {
      if (temp < (nightTemp - 0.5 + 3 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (nightTemp + 0.5 + 3 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, 2 * ileMinutNoc / 3 ), dodawanieMin(godzNoc, minNoc, 2 * ileMinutNoc / 3))) {
      if (temp < (nightTemp - 0.5 + 2 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (nightTemp + 0.5 + 2 * (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, 5 * ileMinutNoc / 6 ), dodawanieMin(godzNoc, minNoc, 5 * ileMinutNoc / 6))) {
      if (temp < (nightTemp - 0.5 + (ileStopniDzien / 6))) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > (nightTemp + 0.5 + (ileStopniDzien / 6))) {
        digitalWrite(grzalka, HIGH);
      }
    }
    else {
      if (temp < nightTemp - 0.5) {
        digitalWrite(grzalka, LOW);
      }
      else if (temp > nightTemp + 0.5) {
        digitalWrite(grzalka, HIGH);
      }
    }
  }
}

void opcjaWilg(float wilg)  {

  t = rtc.getTime();
  int ileMinut = ileMinutMiedzyGodzinami(godzDzien, minDzien, godzNoc, minNoc) / 2;
  int ileMinutNoc = ileMinutMiedzyGodzinami(godzNoc, minNoc, godzDzien, minDzien) / 2;
  float ileWilg = fabs(dayWilg - nightWilg);

  //dzien
  if (czyDzien(godzDzien, minDzien, godzNoc, minNoc)) {
    if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, ileMinut / 6))) {
      if (wilg < (dayWilg - 5 + 5 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (dayWilg + 5 + 5 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 3 ), dodawanieMin(godzDzien, minDzien, ileMinut / 3))) {
      if (wilg < (dayWilg - 5 + 4 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (dayWilg + 5 + 4 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 2 ), dodawanieMin(godzDzien, minDzien, ileMinut / 2))) {
      if (wilg < (dayWilg - 5 + 3 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (dayWilg + 5 + 3 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 2 * ileMinut / 3 ), dodawanieMin(godzDzien, minDzien, 2 * ileMinut / 3))) {
      if (wilg < (dayWilg - 5 + 2 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (dayWilg + 5 + 2 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 5 * ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, 5 * ileMinut / 6))) {
      if (wilg < (dayWilg - 5 + (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (dayWilg + 5 + (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else {
      if (wilg < dayWilg - 5) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (dayWilg + 5)) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
  }
  else {
    // noc
    if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, ileMinutNoc / 6 ), dodawanieMin(godzNoc, minNoc, ileMinutNoc / 6))) {
      if (wilg < (nightWilg - 5 - 5 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (nightWilg + 5 - 5 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, ileMinutNoc / 3 ), dodawanieMin(godzNoc, minNoc, ileMinutNoc / 3))) {
      if (wilg < (nightWilg - 5 - 4 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (nightWilg + 5 - 4 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, ileMinutNoc / 2 ), dodawanieMin(godzNoc, minNoc, ileMinutNoc / 2))) {
      if (wilg < (nightWilg - 5 - 3 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (nightWilg + 5 - 3 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, 2 * ileMinutNoc / 3 ), dodawanieMin(godzNoc, minNoc, 2 * ileMinutNoc / 3))) {
      if (wilg < (nightWilg - 5 - 2 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (nightWilg + 5 - 2 * (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, 5 * ileMinutNoc / 6 ), dodawanieMin(godzNoc, minNoc, 5 * ileMinutNoc / 6))) {
      if (wilg < (nightWilg - 5 - (ileWilg / 6))) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (nightWilg + 5 - (ileWilg / 6))) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
    else {
      if (wilg < nightWilg - 5) {
        digitalWrite(nawilzacz, LOW);
      }
      else if (wilg > (nightWilg + 5)) {
        digitalWrite(nawilzacz, HIGH);
      }
    }
  }
}

int dodawanieGodz(int godz1, int min1, int min2) {
  int pomGodz1 = godz1;
  int pomMin1 = min1;
  int pomMin2 = min2;
  int pom = min2 / 60;



  if (pom >= 1) {
    pomGodz1 = pomGodz1 + pom;
    pomMin2 = pomMin2 - pom * 60;
  }
  pomMin1 = pomMin1 + pomMin2;
  if (pomMin1 >= 60) {
    pomGodz1++;
    //pomMin1 = pomMin1 - 60;
  }
  return pomGodz1;
}

int dodawanieMin(int godz1, int min1, int min2) {
  int pomGodz1 = godz1;
  int pomMin1 = min1;
  int pomMin2 = min2;
  int pom = min2 / 60;

  if (pom >= 1) {
    pomGodz1 = pomGodz1 + pom;
    pomMin2 = pomMin2 - pom * 60;
  }
  pomMin1 = pomMin1 + pomMin2;
  if (pomMin1 >= 60) {
    //pomGodz1++;
    pomMin1 = pomMin1 - 60;
  }
  return pomMin1;
}

int ileMinutMiedzyGodzinami(int godz1, int min1, int godz2, int min2) {
  int minuty;
  if (godz2 > godz1) {
    if (min1 <= min2) {
      minuty = ((godz2 - godz1) * 60) + (min2 - min1);
    }
    else {
      minuty = ((godz2 - godz1 - 1) * 60) + (60 - min1) + min2;
    }
  }


  else if (godz2 == godz1) {
    if (min2 > min1) {
      minuty = min2 - min1;
    }
    else if (min2 < min1) {
      minuty = (23 * 60) + (60 - min1) + min2;
    }
    else {
      minuty = 0;
    }

  }
  else {
    minuty = (24 - (godz1 + 1) + godz2) * 60 + (60 - min1) + min2;
  }
  return minuty;
}
void opcjaOswi() {
  int ileMinut = ileMinutMiedzyGodzinami(godzDzien, minDzien, godzNoc, minNoc) / 2;

  if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut), dodawanieMin(godzDzien, minDzien, ileMinut))) {
    if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, ileMinut / 6))) {
      analogWrite(dioda_R, 50);
      analogWrite(dioda_G, 50);
      analogWrite(dioda_B, 50);
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 3 ), dodawanieMin(godzDzien, minDzien, ileMinut / 3))) {
      analogWrite(dioda_R, 100);
      analogWrite(dioda_G, 100);
      analogWrite(dioda_B, 100);
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, ileMinut / 2 ), dodawanieMin(godzDzien, minDzien, ileMinut / 2))) {
      analogWrite(dioda_R, 150);
      analogWrite(dioda_G, 150);
      analogWrite(dioda_B, 150);
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 2 * ileMinut / 3 ), dodawanieMin(godzDzien, minDzien, 2 * ileMinut  / 3))) {
      analogWrite(dioda_R, 200);
      analogWrite(dioda_G, 200);
      analogWrite(dioda_B, 200);
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 5 * ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, 5 * ileMinut  / 6))) {
      analogWrite(dioda_R, 250);
      analogWrite(dioda_G, 250);
      analogWrite(dioda_B, 250);
    }
    else {
      analogWrite(dioda_R, 255);
      analogWrite(dioda_G, 255);
      analogWrite(dioda_B, 255);
    }
  }
  else {
    if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 7 * ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, 7 * ileMinut / 6))) {
      analogWrite(dioda_R, 255);
      analogWrite(dioda_G, 255);
      analogWrite(dioda_B, 255);
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 8 * ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, 8 * ileMinut / 6))) {
      analogWrite(dioda_R, 250);
      analogWrite(dioda_G, 250);
      analogWrite(dioda_B, 250);
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 9 * ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, 9 * ileMinut / 6))) {
      analogWrite(dioda_R, 200);
      analogWrite(dioda_G, 200);
      analogWrite(dioda_B, 200);
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 10 * ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, 10 * ileMinut / 6))) {
      analogWrite(dioda_R, 150);
      analogWrite(dioda_G, 150);
      analogWrite(dioda_B, 150);
    }
    else if (czyDzien(t.hour, t.min, dodawanieGodz(godzDzien, minDzien, 11 * ileMinut / 6 ), dodawanieMin(godzDzien, minDzien, 11 * ileMinut / 6))) {
      analogWrite(dioda_R, 100);
      analogWrite(dioda_G, 100);
      analogWrite(dioda_B, 100);
    }
    else {
      analogWrite(dioda_R, 50);
      analogWrite(dioda_G, 50);
      analogWrite(dioda_B, 50);
    }
  }
};
void oswietlenieNoc() {
  int ileMinut;
  int ileGodz = jakDlugo;
  if (jakDlugo / ileGodz != 1) {
    ileMinut = 30;
  }
  else {
    ileMinut = 0;
  }
  if (czyDzien(t.hour, t.min, dodawanieGodz(godzNoc, minNoc, ileGodz * 60), dodawanieMin(godzNoc, minNoc, ileMinut))) {
    analogWrite(dioda_R, 255);
    analogWrite(dioda_G, 0);
    analogWrite(dioda_B, 0);
  }
  else {
    digitalWrite(dioda_R, LOW);
    digitalWrite(dioda_G, LOW);
    digitalWrite(dioda_B, LOW);
  }
};
