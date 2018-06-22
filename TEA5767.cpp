#include <Arduino.h>
#include <Wire.h>
#include <TEA5767.h>

TEA5767::TEA5767() {
  Wire.begin();
  initializeTransmissionData();
  muted = false;
}

void TEA5767::initializeTransmissionData() {
  transmission_data[FIRST_DATA] = 0;            //MUTE: 0 - not muted
                                                //SEARCH MODE: 0 - not in search mode
	
  transmission_data[SECOND_DATA] = 0;           //No frequency defined yet
	
  transmission_data[THIRD_DATA] = 0xB0;         //10110000
                                                //SUD: 1 - search up
                                                //SSL[1:0]: 01 - low; level ADC output = 5
                                                //HLSI: 1 - high side LO injection
                                                //MS: 0 - stereo ON
                                                //MR: 0 - right audio channel is not muted
                                                //ML: 0 - left audio channel is not muted
                                                //SWP1: 0 - port 1 is LOW
	
  transmission_data[FOURTH_DATA] = 0x10;        //00010000
                                                //SWP2: 0 - port 2 is LOW
                                                //STBY: 0 - not in Standby mode
                                                //BL: 0 - US/Europe FM band
                                                //XTAL: 1 - 32.768 kHz
                                                //SMUTE: 0 - soft mute is OFF
                                                //HCC: 0 - high cut control is OFF
                                                //SNC: 0 - stereo noise cancelling is OFF
                                                //SI: 0 - pin SWPORT1 is software programmable port 1
	
  transmission_data[FIFTH_DATA] = 0x00;         //PLLREF: 0 - the 6.5 MHz reference frequency for the PLL is disabled
                                                //DTC: 0 - the de-emphasis time constant is 50 ms
}

void TEA5767::calculateOptimalHiLoInjection(float freq) {
	byte signalHigh;
	byte signalLow;
	
	setHighSideLOInjection();
	transmitFrequency((float) (freq + 0.45));
	
	signalHigh = getSignalLevel();
	
	setLowSideLOInjection();
	transmitFrequency((float) (freq - 0.45));
	
	signalLow = getSignalLevel();

	hiInjection = (signalHigh < signalLow) ? 1 : 0;
}

void TEA5767::setFrequency(float _frequency) {
	frequency = _frequency;
	unsigned int frequencyW;
	
	if (hiInjection) {
		setHighSideLOInjection();
		frequencyW = 4 * ((frequency * 1000000) + 225000) / 32768;
	} else {
		setLowSideLOInjection();
		frequencyW = 4 * ((frequency * 1000000) - 225000) / 32768;
	}
	
	transmission_data[FIRST_DATA] = ((transmission_data[FIRST_DATA] & 0xC0) | ((frequencyW >> 8) & 0x3F));
	transmission_data[SECOND_DATA] = frequencyW & 0XFF;
}

void TEA5767::transmitData() {
	Wire.beginTransmission(TEA5767_I2C_ADDRESS);
	for (int i=0 ; i<6 ; i++) {
		Wire.write(transmission_data[i]);
	}
	Wire.endTransmission();
	delay(1);
}

void TEA5767::mute() {
	muted = true;
	setSoundOff();
	transmitData();
}

void TEA5767::setSoundOff() {
	transmission_data[FIRST_DATA] |= 0b10000000;
}

void TEA5767::turnTheSoundBackOn() {
	muted = false;
	setSoundOn();
	transmitData();
}

void TEA5767::setSoundOn() {
	transmission_data[FIRST_DATA] &= 0b01111111;
}

boolean TEA5767::isMuted() {
	return muted;
}

void TEA5767::transmitFrequency(float frequency) {
	setFrequency(frequency);
	transmitData();
}

void TEA5767::selectFrequency(float frequency) {
	calculateOptimalHiLoInjection(frequency);
	transmitFrequency(frequency);
}

void TEA5767::selectFrequencyMuting(float frequency) {
	mute();
	calculateOptimalHiLoInjection(frequency);
	transmitFrequency(frequency);
	turnTheSoundBackOn();
}

void TEA5767::readStatus() {
	Wire.requestFrom (TEA5767_I2C_ADDRESS, 5);
	
	if (Wire.available ()) {
		for (int i = 0; i < 5; i++) {
			reception_data[i] = Wire.read();
		}
	}
	delay(1);
}

float TEA5767::readFrequencyInMHz() {
	loadFrequency();
	
	unsigned int frequencyW = (((reception_data[FIRST_DATA] & 0x3F) * 256) + reception_data[SECOND_DATA]);
	return getFrequencyInMHz(frequencyW);
}

void TEA5767::loadFrequency() {
	readStatus();
	
	//Stores the read frequency that can be the result of a search and it´s not yet in transmission data
	//and is necessary to subsequent calls to search.
	transmission_data[FIRST_DATA] = (transmission_data[FIRST_DATA] & 0xC0) | (reception_data[FIRST_DATA] & 0x3F);
	transmission_data[SECOND_DATA] = reception_data[SECOND_DATA];
}

float TEA5767::getFrequencyInMHz(unsigned int frequencyW) {
	if (hiInjection) {
		return (((frequencyW / 4.0) * 32768.0) - 225000.0) / 1000000.0;
	} else {
		return (((frequencyW / 4.0) * 32768.0) + 225000.0) / 1000000.0;
	}
}

void TEA5767::setSearchUp() {
	transmission_data[THIRD_DATA] |= 0b10000000;
}

void TEA5767::setSearchDown() {
	transmission_data[THIRD_DATA] &= 0b01111111;
}

void TEA5767::setSearchLowStopLevel() {
	transmission_data[THIRD_DATA] &= 0b10011111;
	transmission_data[THIRD_DATA] |= (LOW_STOP_LEVEL << 5);
}

void TEA5767::setSearchMidStopLevel() {
	transmission_data[THIRD_DATA] &= 0b10011111;
	transmission_data[THIRD_DATA] |= (MID_STOP_LEVEL << 5);
}

void TEA5767::setSearchHighStopLevel() {
	transmission_data[THIRD_DATA] &= 0b10011111;
	transmission_data[THIRD_DATA] |= (HIGH_STOP_LEVEL << 5);
}

void TEA5767::setHighSideLOInjection() {
	transmission_data[THIRD_DATA] |= 0b00010000;
}

void TEA5767::setLowSideLOInjection() {
	transmission_data[THIRD_DATA] &= 0b11101111;
}

byte TEA5767::searchNextMuting() {
	byte bandLimitReached;
	
	mute();
	bandLimitReached = searchNext();
	turnTheSoundBackOn();
	
	return bandLimitReached;
}

byte TEA5767::searchNext() {
	byte bandLimitReached;
	
	if (isSearchUp()) {
		selectFrequency(readFrequencyInMHz() + 0.1);
	} else {
		selectFrequency(readFrequencyInMHz() - 0.1);
	}
	
	//Turns the search on
	transmission_data[FIRST_DATA] |= 0b01000000;
	transmitData();
		
	while(!isReady()) { }
	//Read Band Limit flag
	bandLimitReached = isBandLimitReached();
	//Loads the new selected frequency
	loadFrequency();
	
	//Turns de search off
	transmission_data[FIRST_DATA] &= 0b10111111;
	transmitData();
	
	return bandLimitReached;
}

byte TEA5767::startsSearchMutingFromBeginning() {
	byte bandLimitReached;
	
	mute();
	bandLimitReached = startsSearchFromBeginning();
	turnTheSoundBackOn();
	
	return bandLimitReached;
}

byte TEA5767::startsSearchMutingFromEnd() {
	byte bandLimitReached;
	
	mute();
	bandLimitReached = startsSearchFromEnd();
	turnTheSoundBackOn();
	
	return bandLimitReached;
}

byte TEA5767::startsSearchFromBeginning() {
	setSearchUp();
	return startsSearchFrom(87.0);
}

byte TEA5767::startsSearchFromEnd() {
	setSearchDown();
	return startsSearchFrom(108.0);
}

byte TEA5767::startsSearchFrom(float frequency) {
	selectFrequency(frequency);
	return searchNext();
}

byte TEA5767::getSignalLevel() {
	//Necessary before read status
	transmitData();
	//Read updated status
	readStatus();
	return reception_data[FOURTH_DATA] >> 4;
}

byte TEA5767::isStereo() {
	readStatus();
	return reception_data[THIRD_DATA] >> 7;
}

byte TEA5767::isReady() {
	readStatus();
	return reception_data[FIRST_DATA] >> 7;
}

byte TEA5767::isBandLimitReached() {
	readStatus();
	return (reception_data[FIRST_DATA] >> 6) & 1;
}

byte TEA5767::isSearchUp() {
	return (transmission_data[THIRD_DATA] & 0b10000000) != 0;
}

byte TEA5767::isSearchDown() {
	return (transmission_data[THIRD_DATA] & 0b10000000) == 0;
}

boolean TEA5767::isStandBy() {
	readStatus();
	return (transmission_data[FOURTH_DATA] & 0b01000000) != 0;
}

void TEA5767::setStereoReception() {
	transmission_data[THIRD_DATA] &= 0b11110111;
	transmitData();
}

void TEA5767::setMonoReception() {
	transmission_data[THIRD_DATA] |= 0b00001000;
	transmitData();
}

void TEA5767::setSoftMuteOn() {
	transmission_data[FOURTH_DATA] |= 0b00001000;
	transmitData();
}

void TEA5767::setSoftMuteOff() {
	transmission_data[FOURTH_DATA] &= 0b11110111;
	transmitData();
}

void TEA5767::muteRight() {
	transmission_data[THIRD_DATA] |= 0b00000100;
	transmitData();
}

void TEA5767::turnTheRightSoundBackOn() {
	transmission_data[THIRD_DATA] &= 0b11111011;
	transmitData();
}

void TEA5767::muteLeft() {
	transmission_data[THIRD_DATA] |= 0b00000010;
	transmitData();
}

void TEA5767::turnTheLeftSoundBackOn() {
	transmission_data[THIRD_DATA] &= 0b11111101;
	transmitData();
}

void TEA5767::setStandByOn() {
	transmission_data[FOURTH_DATA] |= 0b01000000;
	transmitData();
}

void TEA5767::setStandByOff() {
	transmission_data[FOURTH_DATA] &= 0b10111111;
	transmitData();
}

void TEA5767::setHighCutControlOn() {
	transmission_data[FOURTH_DATA] |= 0b00000100;
	transmitData();
}

void TEA5767::setHighCutControlOff() {
	transmission_data[FOURTH_DATA] &= 0b11111011;
	transmitData();
}

void TEA5767::setStereoNoiseCancellingOn() {
	transmission_data[FOURTH_DATA] |= 0b00000010;
	transmitData();
}

void TEA5767::setStereoNoiseCancellingOff() {
	transmission_data[FOURTH_DATA] &= 0b11111101;
	transmitData();
}