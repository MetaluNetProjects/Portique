/*********************************************************************
 *				firmware for motor X
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <analog.h>
#include <dcmotor.h>
#include <ramp.h>
#include <eeparams.h>

t_delay mainDelay;

DCMOTOR_DECLARE(A);

//-------------  Timer1 macros :  ---------------------------------------- 
//prescaler=PS fTMR1=FOSC/(4*PS) nbCycles=0xffff-TMR1init T=nbCycles/fTMR1=(0xffff-TMR1init)*4PS/FOSC
//TMR1init=0xffff-(T*FOSC/4PS) ; max=65536*4PS/FOSC : 
//ex: PS=8 : T=0.01s : TMR1init=0xffff-15000
//Maximum 1s !!
#define	TMR1init(T) (0xffff-((T*FOSC)/32000)) //ms ; maximum: 8MHz:262ms 48MHz:43ms 64MHz:32ms
#define	TMR1initUS(T) (0xffff-((T*FOSC)/32000000)) //us ; 
#define InitTimer(T) do{ TMR1H=TMR1init(T)/256 ; TMR1L=TMR1init(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define InitTimerUS(T) do{ TMR1H=TMR1initUS(T)/256 ; TMR1L=TMR1initUS(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define TimerOut() (PIR1bits.TMR1IF)

void highInterrupts()
{
	if(PIR1bits.TMR1IF) {
		DCMOTOR_CAPTURE_SERVICE(A);
		InitTimerUS(50UL);
	}
}

void sendMotorState()
{
	static unsigned char buf[20] = { 'B', 10};
	static int ramppos;
	static unsigned len;
	
	len = 2;
	buf[len++] = DCMOTOR_GETPOS(A) >> 8;
	buf[len++] = DCMOTOR_GETPOS(A) & 255;
	buf[len++] = digitalRead(MOTA_END) == MOTA_ENDLEVEL;
	buf[len++] = digitalRead(MOTA_NEAREND) * 2 + digitalRead(MOTA_HIEND);
	buf[len++] = DCMOTOR(A).Vars.PWMConsign >> 8;
	buf[len++] = DCMOTOR(A).Vars.PWMConsign & 255;
	ramppos = (int)rampGetPos(&(DCMOTOR(A).PosRamp));
	buf[len++] = ramppos >> 8;
	buf[len++] = ramppos & 255;
	buf[len++] = DCMOTOR(A).VolVars.homed;
	buf[len++] = '\n';
	fraiseSend(buf,len);
}

void setup(void) {
//----------- Setup ----------------
	fruitInit();

	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module
	analogSelect(0, MOTA_CURRENT);	// assign MotorA current sense to analog channel 0

//----------- dcmotor setup ----------------
	dcmotorInit(A);
	pinModeDigitalIn(MOTA_NEAREND);
	pinModeDigitalIn(MOTA_HIEND);

	DCMOTOR(A).Setting.onlyPositive = 1;
	DCMOTOR(A).Setting.PosWindow = 1;
	DCMOTOR(A).Setting.PwmMin = 50;
	DCMOTOR(A).Setting.PosErrorGain = 5;

//#define HW_PARAMS
#ifdef HW_PARAMS
	DCMOTOR(A).PosRamp.maxSpeed = 1800;
	DCMOTOR(A).PosRamp.maxAccel = 2400;
	DCMOTOR(A).PosRamp.maxDecel = 2400;
	//rampSetPos(&DCMOTOR(C).PosRamp, 0);

	DCMOTOR(A).PosPID.GainP = 40;
	DCMOTOR(A).PosPID.GainI = 1;
	DCMOTOR(A).PosPID.GainD = 0;
	DCMOTOR(A).PosPID.MaxOut = 1023;

	//DCMOTOR(C).VolVars.homed = 0;
#else
	EEreadMain();
#endif

	T1CON=0b00110011;//src=fosc/4,ps=8,16bit r/w,on.
	PIE1bits.TMR1IE=1;  //1;
	IPR1bits.TMR1IP=1;
}

#define DCMOTOR_UPDATE_SYM_PROTECT_(motID) do{ \
	DCMOTOR_FORMATPWM(motID);\
	if((dcmotor_v < 0) && (digitalRead(MOT##motID##_END) == MOT##motID##_ENDLEVEL)) dcmotor_v = 0; \
	if((dcmotor_v > 0) && (digitalRead(MOT##motID##_HIEND) == MOT##motID##_ENDLEVEL)) dcmotor_v = 0; \
	dcmotor_vabs = dcmotor_v < 0 ? -dcmotor_v : dcmotor_v; \
	SET_PWM(MOT##motID##_PWM, dcmotor_vabs); \
	if(dcmotor_v < 0) { digitalClear(M##motID##1);digitalSet(M##motID##2);}\
	else { digitalClear(M##motID##2); digitalSet(M##motID##1);}\
 } while(0)
#define DCMOTOR_UPDATE_SYM_PROTECT(motID) CALL_FUN(DCMOTOR_UPDATE_SYM_PROTECT_,motID)

int count;
byte endSwitchOn;
void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		analogSend();		// send analog channels that changed
		if(digitalRead(MOTA_END) == MOTA_ENDLEVEL) {
			if(!endSwitchOn) {
				endSwitchOn = 1;
				rampInit(&(DCMOTOR(A).PosRamp));
			}
		} else endSwitchOn = 0;
		DCMOTOR_COMPUTE(A, SYM_PROTECT);
		if(count++ > 10) {
			count = 0;
			sendMotorState();
		}
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
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
		case 120 : DCMOTOR_INPUT(A) ; break;
		case 130 : rampStop(&(DCMOTOR(A).PosRamp)); break;
	}
}

// EEPROM

void EEdeclareMain()
{
	DCMOTOR_DECLARE_EE(A);
}
