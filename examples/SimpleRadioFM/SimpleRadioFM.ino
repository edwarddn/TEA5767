#include <TEA5767.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
TEA5767 radio;

#define RX 10
#define TX 11
SoftwareSerial bluetooth(RX, TX); // RX, TX

int valAnalogRead = 0;
int to = 0;
int too = 0;
float previousFrequency = 0;
float frequency = 0;
float freq;
byte isBandLimitReached = 0;

void setup() {  
  Serial.begin(115200);
  bluetooth.begin(9600);
  initScreen();
}

void loop() {

  for (int i = 0; i <= 30; i++) {
    valAnalogRead += analogRead(A2);
    delay(1);
  }

  valAnalogRead = valAnalogRead / 30;
  int frequencyInt = map(valAnalogRead, 2, 1014, 8800, 10800); //Analog value to frequency from 87.0 MHz to 107.00 MHz
  frequency = frequencyInt / 100.0f;
  
  if (bluetooth.available()) {
    char dados = bluetooth.read();
    if ('0' == dados) {
      bluetooth.println("Desligando!");
      radio.setStandByOn();
    } else if ('1' == dados) {
      bluetooth.println("Ligado!");
      radio.setStandByOff();
    } else if ('2' == dados) {
      bluetooth.println("Mudo!");
      radio.mute();
    } else if ('3' == dados) {
      bluetooth.println("Sem Mudo!");
      radio.turnTheSoundBackOn();
    } else if ('4' == dados) {

      radio.setSearchDown();
      radio.setSearchMidStopLevel();

      bluetooth.println("Pesquisa down em andamento...");
      isBandLimitReached = radio.searchNextMuting();

      bluetooth.print("Limite de banda alcancado? ");
      bluetooth.println(isBandLimitReached ? "Sim" : "Nao");
      lerRadio();
      too = 0;
      to = 0;
    } else if ('5' == dados) {

      radio.setSearchUp();
      radio.setSearchMidStopLevel();

      bluetooth.println("Pesquisa up em andamento...");
      isBandLimitReached = radio.searchNextMuting();

      bluetooth.print("Limite de banda alcancado? ");
      bluetooth.println(isBandLimitReached ? "Sim" : "Nao");
      lerRadio();
      too = 0;
      to = 0;
    }
  }

  if (frequency - previousFrequency >= 0.1f || previousFrequency - frequency >= 0.1f) {

    radio.selectFrequency(frequency);
    previousFrequency = frequency;
    lerRadio();
    too = 0;
    to = 0;
  }

  if (to > 100 && too < 2) {
    lerRadio();
    to = 0;
  }
  if (to > 99 && too < 6) {
    too++;
  }
  if (to < 600) {
    to++;
  }
//  Serial.print("DEC");
//  Serial.print("\n");
//  Serial.print(to);
//  Serial.print("\n");
//  Serial.print(too);
//  Serial.print("\n");
}

void lerRadio() {

  freq = radio.readFrequencyInMHz();
  bool stereo = radio.isStereo();
  int signal_level = radio.getSignalLevel();
  pringDisplay(freq, stereo, signal_level);
}

void initScreen() {

  bluetooth.println("RADIO FM");
  bluetooth.println("EDWARD");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  display.display();       //Inicializa LCD 16 x 2
  delay(1);
  display.clearDisplay();  //Limpa LCD
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(40, 0);
  display.println("RADIO FM");
  display.setTextSize(2);
  display.setCursor(30, 12);
  display.println("EDWARD");
  display.display();
  delay(2000);
}

int atual = 0;
void pringDisplay(float frequencia, bool isStereo, int sinal) {

  int novo = frequencia + isStereo + sinal;
  if (atual != novo) {

    atual = novo;
    display.stopscroll();
    display.clearDisplay();
    display.setTextColor(WHITE);

    display.setTextSize(1);
    display.setCursor(2, 9);
    display.println("FM");

    display.setCursor(30, 10);
    display.setTextSize(2);
    String texto = String(frequencia, 1);
    bluetooth.println(texto + " MHz ");

    if (texto.length() == 4) {
      display.println(" " + texto);
    } else {
      display.println(texto);
    }

    display.setTextSize(1);
    display.setCursor(90, 17);
    display.println(" MHz");

    display.setCursor(0, 25);
    if (isStereo) {
      display.println("Stereo");
    } else {
      display.println("Mono");
    }

    display.setCursor(91, 25);
    display.println("Edward");

    //Barra
    int valor = map(frequencia, 88, 108, 1, 80);
    display.fillRect(43 + (80 - valor), 2, valor, 4, WHITE); //conteudo
    display.drawLine(41, 0, 127, 0, WHITE); //topo
    display.fillRect(40, 0, 2, 8, WHITE); //ponta    
    display.drawLine(41, 7, 127, 7, WHITE); //baixo
    display.fillRect(126, 0, 2, 8, WHITE); //fundo

    //Antena
    display.drawLine(0, 0, 14, 0, WHITE); //topo
    display.drawLine(7, 0, 7, 7, WHITE);//corpo
    display.drawLine(0, 0, 6, 4, WHITE);//barra "\"
    display.drawLine(14, 0, 8, 4, WHITE); //barra "/"

    if (sinal >= 15) {
      display.fillRect(30, 0, 2, 8, WHITE);
    }
    if (sinal >= 14) {
      display.fillRect(27, 1, 2, 7, WHITE);
    }
    if (sinal >= 12) {
      display.fillRect(24, 2, 2, 6, WHITE);
    }
    if (sinal >= 10) {
      display.fillRect(21, 3, 2, 5, WHITE);
    }
    if (sinal >= 8) {
      display.fillRect(18, 4, 2, 4, WHITE);
    }
    if (sinal >= 6) {
      display.fillRect(15, 5, 2, 3, WHITE);
    }
    if (sinal >= 3) {
      display.fillRect(12, 6, 2, 2, WHITE);
    }
    if (sinal >= 1) {
      display.fillRect(9, 7, 2, 1, WHITE);
    }
    display.display();
    delay(1);
  }

}
