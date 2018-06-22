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

bool protecao = false;
bool efeitoProtecao = false;
int valAnalogRead = 0;
int to = 0;
int too = 0;
float previousFrequency = 0;
float frequency = 0;
float freq;
byte isBandLimitReached = 0;

float volt = 0;
float bMin = 2.8;
float bMax = 3.7;


void setup() {
  
  Serial.begin(115200);
  bluetooth.begin(9600);
  initScreen();
}

void loop() {

  for (int i; i < 30; i++) {
    volt = volt + ((analogRead(A2) * 5.0) / 1024.0);
    delay(1);
  }
  volt = volt / 30;

  float volts = volt;
  if (volt > bMax) {
    volts = bMax;
  } else if (volt < bMin) {
    volts = bMin;
  }
  int baterry = map((volts * 100), (bMin * 100), (bMax * 100), 0, 100);

  for (int i; i < 30; i++) {
    valAnalogRead = valAnalogRead + analogRead(A0);
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
      lerRadio(baterry);
      too = 0;
      to = 0;
    } else if ('5' == dados) {

      radio.setSearchUp();
      radio.setSearchMidStopLevel();
      
      bluetooth.println("Pesquisa up em andamento...");
      isBandLimitReached = radio.searchNextMuting();
      
      bluetooth.print("Limite de banda alcancado? ");
      bluetooth.println(isBandLimitReached ? "Sim" : "Nao");
      lerRadio(baterry);
      too = 0;
      to = 0;
    }    
  }

  if (frequency - previousFrequency >= 0.1f || previousFrequency - frequency >= 0.1f) {

    radio.selectFrequency(frequency);
    previousFrequency = frequency;
    lerRadio(baterry);
    too = 0;
    to = 0;
    protecao = true;
    efeitoProtecao = false;
  }

  if (to > 100 && too < 6) {
    lerRadio(baterry);
    to = 0;
  } else if (too >= 16) {
    protecaoTela();
  }
  
  if (to > 100 && too < 16) {
    too++;
  }
  to++;
}

void lerRadio(int baterry) {

    freq = radio.readFrequencyInMHz();
    bool stereo = radio.isStereo();
    int signal_level = radio.getSignalLevel();
    pringDisplay(baterry, freq, stereo, signal_level, volt);
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


void protecaoTela() {

  if (protecao) {

    protecao = false;
    efeitoProtecao = true;

    display.clearDisplay();
    display.setCursor(30, 10);
    display.setTextSize(2);

    String texto = String(freq, 1);
    if (texto.length() == 4) {
      display.println(" " + texto);
    } else {
      display.println(texto);
    }

    display.setTextSize(1);
    display.setCursor(90, 17);
    display.println(" MHz");
    display.display();
    delay(1);

    to = 10000;
  }

  if (efeitoProtecao && to > 1000) {

    int randNumber = random(1, 5);
    if (randNumber == 1) {
      display.startscrollright(0x00, 0x0F);
    } else if (randNumber == 2) {
      display.startscrollleft(0x00, 0x0F);
    } else if (randNumber == 3) {
      display.startscrolldiagright(0x00, 0x07);
    } else if (randNumber == 4) {
      display.startscrolldiagleft(0x00, 0x07);
    }
    to = 0;
  }
}

int atual = 0;
void pringDisplay(int baterry, float frequencia, bool isStereo, int sinal, float voltagem) {

  int novo = baterry + frequencia + isStereo + sinal + voltagem;
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
    //display.println("Edward");
    display.println(String(voltagem, 2) + "v");

    //Bateria
    int valor = map(baterry, 0, 100, 1, 20);
    display.fillRect(104 + (20 - valor), 2, valor, 4, WHITE); //conteudo
    display.drawLine(101, 0, 127, 0, WHITE); //topo
    display.fillRect(100, 0, 2, 3, WHITE);//barra ponta
    display.drawLine(97, 2, 101, 2, WHITE); //topo ponta
    display.fillRect(96, 2, 2, 4, WHITE); //ponta
    display.drawLine(97, 5, 101, 5, WHITE); //fim ponta
    display.fillRect(100, 5, 2, 3, WHITE);//barra fim ponta
    display.drawLine(101, 7, 127, 7, WHITE); //baixo
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

