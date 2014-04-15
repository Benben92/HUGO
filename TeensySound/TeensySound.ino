//#include <arm_math.h>
#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Bounce.h>

#include <Metro.h>

#define MOD_RATE 10 // modulation rate in ms

#include "Delay.h"
#include "FormantFilter.h"
#include "Sequences.h"

#define Test1_PIN 1
#define Test0_PIN 0



Bounce b_test1 = Bounce(Test1_PIN,15);
Bounce b_test0   = Bounce(Test0_PIN,15);


const int myInput = AUDIO_INPUT_MIC;
//const int myInput = AUDIO_INPUT_LINEIN;


AudioInputI2S       audioInput;         // audio shield: mic or line-in
Delay 	staticDelay;
AudioFilterBiquad    formantFilter(ToneFilter);
AudioOutputI2S      audioOutput;        // audio shield: headphones & line-out

// Create Audio connections between the components
// Both channels of the audio input go to the FIR filter
AudioConnection c2(audioInput, 0, staticDelay, 0);
AudioConnection c3(staticDelay, 0, formantFilter, 0);
AudioConnection c4(formantFilter, 0, audioOutput, 0);
AudioConnection c5(formantFilter, 0, audioOutput, 1);

// AudioConnection c4(staticDelay, 0, audioOutput, 0);
// AudioConnection c5(staticDelay, 0, audioOutput, 1);

AudioControlSGTL5000 audioShield;


void setup() {

	Serial.begin(9600);
	//while (!Serial) ;
	delay(3000);

	pinMode(Test1_PIN,INPUT_PULLUP);
	pinMode(Test0_PIN,INPUT_PULLUP);

	// It doesn't work properly with any less than 8
	AudioMemory(12);

	audioShield.enable();
	audioShield.inputSelect(myInput);
	audioShield.volume(90);
	
	setSopranA();

	Serial.println("setup done");
	
	test.push_back(firstFormantSeq);
	test.push_back(secondFormantSeq);
}

// audio volume
int volume = 0;
int oldValue = 0;
int activeSeq = 0;

Metro MonitorMetro = Metro(1000);
Metro ReadMetro = Metro(10);
Metro TimingMetro = Metro(MOD_RATE);

void loop()
{
	//////////////////
	//Mem and CPU Usage  
	// Change this to if(1) for measurement output and to if(0) to stop it
	if(1) {
		if(MonitorMetro.check() == 1) {
			Serial.print("Proc = ");
			Serial.print(AudioProcessorUsage());
			Serial.print(" (");    
			Serial.print(AudioProcessorUsageMax());
			Serial.print("),  Mem = ");
			Serial.print(AudioMemoryUsage());
			Serial.print(" (");    
			Serial.print(AudioMemoryUsageMax());
			Serial.println(")");
		}
	}
	// volume control
	// every 10 ms, check for adjustment
	if (ReadMetro.check() == 1) {
		int n = analogRead(15);
		if (n != volume) {
			volume = n;
			audioShield.volume((float)n / 10.23);
		}
	}
	
	if (TimingMetro.check() == 1) {
	
		if (test[activeSeq].seqCounter <= test[activeSeq].seqLength)
		{		
			test[activeSeq].seqProceed();
		}
		else
		{
			if (test[activeSeq].loop == true)
			{
				test[activeSeq].reset();
				test[activeSeq].seqProceed();
			}			
		}		
	}
	
	

	// update the two buttons
	b_test0.update();
	b_test1.update();


	// if pin1 is grounded
	if(b_test1.fallingEdge()) {

	}

	// If pin1 is open
	if(b_test1.risingEdge()) {

	}

	// if pin0 is grounded
	if(b_test0.fallingEdge()) {
		staticDelay.hold(true);
		activeSeq = 1;
	}
	// if pin 0 is open
	if(b_test0.risingEdge()) {
		staticDelay.hold(false);
		activeSeq = 0;
	}
}



