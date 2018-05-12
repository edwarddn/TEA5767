#include <TEA5767.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
TEA5767 radio;

int analogPin = A0;
int val = 0;
int frequencyInt = 0;
float frequency = 102.49;
float previousFrequency = 0;
int signal_level;
int previousSignal;

void setup() {
  Wire.begin(0x10);
  radio.init();
  initScreen();
  Serial.begin(9600);
}

int to = 0;
int too = 0;
void loop() {

  for(int i;i<30;i++){
     val = val + analogRead(analogPin); 
     delay(1);
  }
  
  val = val/30;
  frequencyInt = map(val, 2, 1014, 8800, 10800); //Analog value to frequency from 87.0 MHz to 107.00 MHz 
  frequency = frequencyInt/100.0f;
 
  unsigned char buf[5];
  if(frequency - previousFrequency >= 0.1f || previousFrequency - frequency >= 0.1f) {
    
    radio.set_frequency(frequency);
    delay(1);
    previousFrequency = frequency;
    lerRadio(buf);
    too = 0;
  }

  if (to > 100 && too < 60) {
    lerRadio(buf);
    to = 0;
    too++;
  }
  to++;
}

void lerRadio(unsigned char *buf) {
  if (radio.read_status(buf) == 1) { 
    double current_freq = floor (radio.frequency_available (buf) / 100000 + .5) / 10;
    bool stereo = radio.stereo(buf);
    signal_level = radio.signal_level(buf);
    previousSignal = signal_level;
    pringDisplay(current_freq, stereo, signal_level);
  }
}

void initScreen(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  display.display();  //Inicializa LCD 16 x 2
  delay(1000);
  display.clearDisplay();       //Limpa LCD
}

int atual = 0;
void pringDisplay(float frequency, bool isStereo, int sinal) {
  
  int novo = frequency + isStereo + sinal;
  if (atual != novo) {
    
    atual = novo;
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("FM");
  
    display.setTextSize(2);
    display.setCursor(10,10);
      
    String texto = String(frequency,1);
    if(texto.length() == 4) {
      display.println(" " + texto + " MHz");
    } else {
      display.println(texto + " MHz");
    }
  
    display.setTextSize(1);
    display.setCursor(0,25);
    if(isStereo) {
      sinal = 15;
      display.println("stereo");
    } else {
      display.println("mono");
    }
  
    if (sinal >=15) {
      display.drawLine(127, 0, 127, 6, WHITE);
      display.drawLine(126, 0, 126, 6, WHITE);
    } 
  
    if (sinal >=11) {
      display.drawLine(123, 2, 123, 6, WHITE);
      display.drawLine(122, 2, 122, 6, WHITE);
    } 
    
    if (sinal >=9) { 
      display.drawLine(119, 4, 119, 6, WHITE);
      display.drawLine(118, 4, 118, 6, WHITE);
    } 
    
    if (sinal >=7) {
      display.drawLine(115, 6, 115, 6, WHITE);
      display.drawLine(114, 6, 114, 6, WHITE);
    }
    display.display();
    delay(1);
  }
}

