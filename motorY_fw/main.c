/*********************************************************************
 *				firmware for motor X
 *********************************************************************/

#define BOARD 8X2A

#include <fruit.h>
#include <analog.h>
#include <dcmotor.h>
#include <ramp.h>
#include <eeparams.h>

t_delay mainDelay;

DCMOTOR_DECLARE(C);

void setup(void) {
//----------- Setup ----------------
	fruitInit();

	//pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	//digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module
	//analogSelect(0, MOTC_CURRENT);	// assign MotorC current sense to analog channel 0

//----------- dcmotor setup ----------------
	dcmotorInit(C);

	DCMOTOR(C).Setting.onlyPositive = 1;
	DCMOTOR(C).Setting.PosWindow = 2;
	DCMOTOR(C).Setting.PwmMin = 150;
	DCMOTOR(C).Setting.PosErrorGain = 6;

//#define HW_PARAMS
#ifdef HW_PARAMS
	DCMOTOR(C).PosRamp.maxSpeed = 1800;
	DCMOTOR(C).PosRamp.maxAccel = 2400;
	DCMOTOR(C).PosRamp.maxDecel = 2400;
	//rampSetPos(&DCMOTOR(C).PosRamp, 0);

	DCMOTOR(C).PosPID.GainP = 40;
	DCMOTOR(C).PosPID.GainI = 1;
	DCMOTOR(C).PosPID.GainD = 0;
	DCMOTOR(C).PosPID.MaxOut = 1023;

	//DCMOTOR(C).VolVars.homed = 0;
#else
	EEreadMain();
#endif

}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		analogSend();		// send analog channels that changed
		DCMOTOR_COMPUTE(C,ASYM);
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		//digitalWrite(LED, c!='0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}
	else if(c=='W') { 	// WRITE: save eeprom
		if((fraiseGetChar() == 'R') && (fraiseGetChar() == 'I') && (fraiseGetChar() == 'T') && (fraiseGetChar() == 'E'))
			EEwriteMain();
	}
}

void fraiseReceive() // receive raw
{
	unsigned char c;
	c=fraiseGetChar();

	switch(c) {
		case 120 : DCMOTOR_INPUT(C) ; break;
	}
}

// EEPROM

void EEdeclareMain()
{
	DCMOTOR_DECLARE_EE(C);
}
