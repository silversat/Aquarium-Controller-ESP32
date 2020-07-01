//
//	TURBIDITY.h
//
//	y = -1120.4 * (x^2) + 5742.3 * x - 4352.9
//	NTU = -1120.4 * (voltage^2) + (5742.3 * voltage) - 4352.9
//
#define NTU_SAMPLES_NUM		30
#define NTU_MAX_ALLOWED		1000
//#define DEBUG_TURBIDITY

bool turbSetupEnded = false;
unsigned long ntuTimer;
float ntu;

void WaterTurbidityHandler() {
	static float valueNTU[NTU_SAMPLES_NUM];
	static uint8_t ntu_idx = 0;

	if(turbSetupEnded) {
		int turbidityRead = analogRead(PIN_TURBIDITY_SENSOR);
		float voltage = turbidityRead * (5.0 / ADC_RESOLUTION); 		// Convert the analog reading (0 - 1023) to a voltage (0 - 5V)
		valueNTU[ntu_idx] = (-1120.4 * sq(voltage)) + (5742.3 * voltage) - 4352.9;

		if((ntuTimer + NTU_READ_INTERVAL) < millis()) {					//time interval: 1s
			float NTUavg = calcRingBufAverage(valueNTU, NTU_SAMPLES_NUM);
				
			if( abs(int(NTUavg - ntu)) > 1 ) {
#if defined DEBUG_TURBIDITY		
				Serial.print("Water turbidity is ");
				Serial.print(NTUavg);
				Serial.print(" ntu");
				Serial.println();
#endif		
				ntu = NTUavg;
#if defined SR_TURBIDITY
				if(ntu < NTU_MAX_ALLOWED) {						// MTU < 0.5 => 4.1±0.3V when temperature is 10~50℃.
					relais(SR_TURBIDITY, RL_OFF); 
				} else {										// MTU > 0.5
					relais(SR_TURBIDITY, RL_ON); 
				}
#endif
			}
			if(main_page == 1 and dstatus == DS_IDLE) {
				printSpaces(7, 13, 2);
				printString(isnan(NTUavg) ? NOT_AVAILABLE : ftoa(buff, ntu), 13, 2);
#if defined SR_TURBIDITY
				printChar(relaisStatus(SR_TURBIDITY)?'*':' ', 0, 2);
#endif
			}
			ntuTimer = millis();								// reset NTU timer
		}
		if(++ntu_idx >= NTU_SAMPLES_NUM) ntu_idx = 0;	// increment index and check for outbound
	}
}

void TurbiditySensorInit() {
	pinMode(PIN_TURBIDITY_SENSOR, INPUT); 
#if defined SR_TURBIDITY
	pinMode(SR_TURBIDITY, OUTPUT); 
	relais(SR_TURBIDITY, RL_OFF); 
#endif
	ntu = 0;
	ntuTimer = millis();								// start NTU measuring timer
	turbSetupEnded = true;
	DEBUG("Turbidity sensor OK\n");
}
