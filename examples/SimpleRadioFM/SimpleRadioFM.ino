#include <TEA5767.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
TEA5767 radio;

bool protecao = false;
bool efeitoProtecao = false;
int valAnalogRead = 0;
int to = 0;
int too = 0;
float previousFrequency = 0;
float frequency = 0;

void setup() {
  Wire.begin(0x10);
  radio.init();
  initScreen();
  Serial.begin(9600);
}

void loop() {

  for(int i; i < 30; i++){
     valAnalogRead = valAnalogRead + analogRead(A0); 
     delay(1);
  }
  
  unsigned char buf[5];  
  valAnalogRead = valAnalogRead / 30;
  int frequencyInt = map(valAnalogRead, 2, 1014, 8800, 10800); //Analog value to frequency from 87.0 MHz to 107.00 MHz 
  frequency = frequencyInt / 100.0f;
 
  if(frequency - previousFrequency >= 0.1f || previousFrequency - frequency >= 0.1f) {
    
    radio.set_frequency(frequency);
    delay(1);
    previousFrequency = frequency;
    lerRadio(buf);
    too = 0;
    to = 0;
    protecao = true;
    efeitoProtecao = false;
  }
  
  if (to > 100 && too < 10) {
    lerRadio(buf);
    too++;
    to = 0;
  } else if (too >= 10) {
    protecaoTela();
  }
  to++;
}

double freq;
void lerRadio(unsigned char *buf) {
  if (radio.read_status(buf) == 1) { 
    int baterry = 100; //porcentagem da bateria
    freq = floor(radio.frequency_available (buf) / 100000 + .5) / 10;
    bool stereo = radio.stereo(buf);
    int signal_level = radio.signal_level(buf);
    pringDisplay(baterry, freq, stereo, signal_level);
  }
}

void initScreen(){
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  display.display();       //Inicializa LCD 16 x 2
  delay(1);
  display.clearDisplay();  //Limpa LCD
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(40,0);
  display.println("RADIO FM");
  display.setTextSize(2);
  display.setCursor(30,12);
  display.println("EDWARD");
  display.display();
  delay(2000);
}


void protecaoTela() {

  if (protecao) {

    protecao = false;
    efeitoProtecao = true;
    
    display.clearDisplay();
    display.setCursor(30,10);
    display.setTextSize(2);
    
    String texto = String(freq,1);
    if(texto.length() == 4) {
      display.println(" " + texto);
    } else {
      display.println(texto);
    }
    
    display.setTextSize(1);
    display.setCursor(90,17);
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
void pringDisplay(int baterry, float frequencia, bool isStereo, int sinal) {
  
  int novo = frequencia + isStereo + sinal;
  if (atual != novo) {
    
    atual = novo;
    display.stopscroll();
    display.clearDisplay();
    display.setTextColor(WHITE);
    
    display.setTextSize(1);
    display.setCursor(2,9);
    display.println("FM");

    display.setCursor(30,10);
    display.setTextSize(2);
    String texto = String(frequencia,1);
    if(texto.length() == 4) {
      display.println(" " + texto);
    } else {
      display.println(texto);
    }
    
    display.setTextSize(1);
    display.setCursor(90,17);
    display.println(" MHz");
  
    display.setCursor(0,25);
    if(isStereo) {
      display.println("Stereo");
    } else {
      display.println("Mono");
    }

    display.setCursor(91,25);
    display.println("Edward");

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
    display.drawLine(0, 0, 14,0, WHITE);//topo
    display.drawLine(7, 0, 7, 7, WHITE);//corpo
    display.drawLine(0, 0, 6, 4, WHITE);//barra "\"
    display.drawLine(14,0, 8, 4, WHITE);//barra "/"

    if (sinal >=15) {
      display.fillRect(30, 0, 2, 8, WHITE);
    }
    if (sinal >=14) {
      display.fillRect(27, 1, 2, 7, WHITE);
    }
    if (sinal >=12) {
      display.fillRect(24, 2, 2, 6, WHITE);
    }
    if (sinal >=10) {
      display.fillRect(21, 3, 2, 5, WHITE);
    }
    if (sinal >=8) {
      display.fillRect(18, 4, 2, 4, WHITE);
    }
    if (sinal >=6) {
      display.fillRect(15, 5, 2, 3, WHITE);
    }
    if (sinal >=3) {
      display.fillRect(12, 6, 2, 2, WHITE);
    }
    if (sinal >=1) {
      display.fillRect(9, 7, 2, 1, WHITE);
    }
    display.display();
    delay(1);
  }
}

