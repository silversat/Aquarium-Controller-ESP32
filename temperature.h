//
//	TEMPERATURE.h
//
OneWire oneWire(PIN_ONE_WIRE_BUS);				// define one-wire bus instance
DallasTemperature sensors(&oneWire);			// define temp sensors on the bus
DeviceAddress Termometro1, Termometro2;			// temp sensor addresses
boolean Tsensor1 = false;						// found temp sensor #1 during setup one-wire bus scan
boolean Tsensor2 = false;						// found temp sensor #2 during setup one-wire bus scan
unsigned long readTimer = 0;

float Tempacqua;
bool tempSetupEnded = false;
bool alrmsonoro = true;
bool alarm_cycle = false;
unsigned long alarm_millis;

//----- DEFCON settings -----
#define DEFCON_1	1		// the higher DEFCON (catastrofe!)
#define DEFCON_2	2		//
#define DEFCON_3	3		//
#define DEFCON_4	4		//
#define DEFCON_5	5		// the lowest DEFCON (all right)
int	defcon = DEFCON_5;

#ifdef IR_REMOTE_KEYBOARD

void WaterTemperatureSetup() { 
	static bool confirm = true;
	static float TempSet, TempOld;
	
	if(CheckInitBit(true)) {		// Leggo in memoria il valore impostato e predispongo la schermata del display
		TempSet = Tempacqua;
		if(TempSet > TEMP_ALLOWED_MAX or TempSet < TEMP_ALLOWED_MIN) {
			TempSet = TEMP_ALLOWED_DEFAULT;
		}
		TempOld = TempSet;
		displayClear();
		printStringCenter("Water Temp Setup", 0);
		printString("Temp.:", 0, 2);
	}

	printNumber(TempSet, 6, 2);

	if(confirm) {
		stampafrecce(6, 3, 0, 1); 
		stampafrecce();

		switch(kp_new) {
			case IR_UP:	
				if(TempSet < TEMP_ALLOWED_MAX) {
					TempSet = TempSet + 0.5;
				} else {
					buzerrore();
				}
				break;	

			case IR_DOWN:	
				if(TempSet > TEMP_ALLOWED_MIN) {
					TempSet = TempSet - 0.5;
				} else {
					buzerrore();
				}
				break;	

			case IR_OK:
				confirm = false; // disattivo questa if in modo che Il tasto OK funzioni solo con la if di conferma definitiva
				break;

			case IR_MENU:
				dstatus = DS_IDLE_INIT;
				break;
		}
	} else {	
		printBlinkingString(confirm_msg);
		if(kp_new == IR_OK or kp_new == IR_MENU)	{	
			if(kp_new == IR_OK and TempOld != TempSet)	{	
				Tempacqua = TempSet;
				parameters_save = SAVE_TEMP;
			}
			confirm = true;
			dstatus = DS_IDLE_INIT;
		}

		if((kp_new == IR_RIGHT) || (kp_new == IR_LEFT) || (kp_new == IR_UP) || (kp_new == IR_DOWN)) {	
			printSpaces(20, 0, 3);
			confirm = true;
		}
	}
}

#endif

float getSensorTemperature( uint8_t sensor ) {
	float temp = NAN;
	if(sensor == 1 and Tsensor1) {
		 temp = sensors.getTempC(Termometro1);
	}
	if(sensor == 2 and Tsensor2) {
		 temp = sensors.getTempC(Termometro2);
	}
	return temp;
}

float getWaterTempeAverage() {
	static float tempRing[TEMP_SAMPLES_NUM];
	static float tmed;
	static uint8_t idx = 0;
	float t1, t2;
	
	if (millis() - readTimer > TEMP_READ_INTERVAL) {	
		readTimer = millis();
		sensors.requestTemperatures();
		
		if(Tsensor1) {
			t1 = sensors.getTempC(Termometro1);
//			DEBUG("T1=%s", ftoa(buff, t1));
		}	
			
		if(Tsensor2) {	
			t2 = sensors.getTempC(Termometro2);
//			DEBUG(", T2=%s", ftoa(buff, t2));
		}	
			
		if(Tsensor1 && Tsensor2) {
			tempRing[idx] = (t1 + t2)/2;
		} else {
			if(Tsensor1) {
				tempRing[idx] = t1;
			} else if(Tsensor2) {
				tempRing[idx] = t2;
			} else {
				tempRing[idx] = 0;
			}
		}
		
		tmed = calcRingBufAverage(tempRing, TEMP_SAMPLES_NUM);
//		if(tmed != 0) {
//			DEBUG(", Tmed=%s\n", ftoa(buff, tmed));
//		}	
		if(++idx >= TEMP_SAMPLES_NUM) idx = 0;			// increment ring index and check for outbound
	}
	return tmed;
}

float WaterTemperatureHandler() {
	static float tempMed = 0.0;
	
	if(tempSetupEnded) {
		tempMed = getWaterTempeAverage();

		if((tempMed < (Tempacqua - TEMP_RANGE)) or (tempMed > (Tempacqua + TEMP_RANGE))) {
			defcon = DEFCON_3;
		} else if(defcon == DEFCON_3) {
			defcon = DEFCON_4;
		}
		
		switch( defcon ) {
			case DEFCON_1:
				break;
				
			case DEFCON_2:
				break;
				
			case DEFCON_3:
				if(kp_new == IR_MENU and alrmsonoro) {		// if ESC key pressed, deactivate acoustic alarm.
					alrmsonoro = false;
					beepOff();								// stop alarm
				}
				if((millis() - alarm_millis > 500) or (millis() < alarm_millis)) {	
					alarm_millis = millis();
					alarm_cycle = !alarm_cycle;
				}
				if(dstatus == DS_IDLE) {
					if(alarm_cycle) {
						if(main_page == 0) {
							if((Tsensor1 or Tsensor2) and tempMed <= TEMP_ALLOWED_MAX) {
								printString(ftoa(buff, tempMed), 3, 2);
								printChar(0b011011111);
							} else {
//								printSpaces(7, 3, 2);
								printString(NOT_AVAILABLE, 3, 2);
							}
						}
						if(alrmsonoro) {
							alarm(true, true);				// status = in alarm, beep ON
						}
					} else {
						if(main_page == 0) {
							printSpaces(7, 3, 2);
						}
						alarm(true, false);					// status = in alarm, beep OFF
					}
				}
				break;
				
			case DEFCON_4:
				alrmsonoro = true;		// reset audible alarm
				beepOff();				// stop alarm
				defcon = DEFCON_5;		// restore DEFCON to 5
				break;
				
			case DEFCON_5:
				if(dstatus == DS_IDLE and main_page == 0) {
					if((Tsensor1 or Tsensor2) and tempMed <= TEMP_ALLOWED_MAX) {
						printString(ftoa(buff, tempMed), 3, 2);
						printChar(0b011011111);
					} else {
//						printSpaces(7, 3, 2);
						printString(NOT_AVAILABLE, 3, 2);
					}
				}
				break;
		}
		
#if defined SR_WATER_HEATER
		if(defcon < 5) {
			relais(SR_WATER_HEATER, RL_ON); 
		} else {
			relais(SR_WATER_HEATER, RL_OFF);
		}
#endif
		if(main_page == 1 and dstatus == DS_IDLE) {
			float temp;
			temp = getSensorTemperature(1);
			printString(isnan(temp) ? NOT_AVAILABLE : ftoa(buff, temp), 8, 3);
			temp = getSensorTemperature(2);
			printString(isnan(temp) ? NOT_AVAILABLE : ftoa(buff, temp), 14, 3);
#if defined SR_WATER_HEATER
			printChar(relaisStatus(SR_WATER_HEATER)?'*':' ', 0, 3);
#endif
		}
	}
	return tempMed;
}

void saveNvramTemperature() {
	if(Tempacqua >= 20 and Tempacqua <= 40) {
		updateStaticMemoryInt(NVRAM_TEMP_ADDR, int(Tempacqua / 0.5));
		StaticMemoryCommit();
	}
}

void TempSensorsInit() {
	Tempacqua = (float)readStaticMemoryInt(NVRAM_TEMP_ADDR) * 0.5;		// read desired water temp from nvram
	if(Tempacqua > TEMP_ALLOWED_MAX) {
		Tempacqua = TEMP_ALLOWED_DEFAULT;
		DEBUG("System temp lost: using %s\n", ftoa(buff, Tempacqua));
		writeStaticMemoryInt(NVRAM_TEMP_ADDR, int(Tempacqua / 0.5));
		StaticMemoryCommit();
	}
	
	sensors.begin();
	Tsensor1 = sensors.getAddress(Termometro1, 0);		// get sensor 1 address, if present (TsensorX true if sensor present, TermometroX contains the address)
	Tsensor2 = sensors.getAddress(Termometro2, 1);		// same for sensor 2

	if(Tsensor1) {
		printDeviceFound(1, Termometro1);
	} else {
		printDeviceNotFound(1);
	}
	
	if(Tsensor2) {
		printDeviceFound(2, Termometro2);
	} else {
		printDeviceNotFound(2);
	}
	
	if(Tsensor1 && Tsensor2) {												// at least one sensor is present	
		sensors.setResolution(Termometro1, TEMP_SENSOR_RESOLUTION);			// configure it
		if(Termometro1 == Termometro2) {									// if only one sensor is present, both Termometro1 and Termometro2 have the same address
			Tsensor2 = false;												// if so, disable sensor 2
		} else {
			sensors.setResolution(Termometro2, TEMP_SENSOR_RESOLUTION);		// else configure it
		}
	}
#if defined SR_WATER_HEATER
	relais(SR_WATER_HEATER, RL_OFF); 
#endif
	tempSetupEnded = true;
	DEBUG("Temperature sensors OK\n");
}
