//
//	PH_SENSOR.h
//
//	Neutral PH voltage = 1500.00mV
//	Acid PH voltage = 2032.44mV
//
// In order to guarantee precision, a temperature sensor such as DS18B20 is needed, to execute automatic temperature compensation.
// You can send commands in the serial monitor to execute the calibration.
// Serial Commands:
//
//  PH Calibration：
//   enterph -> enter the PH calibration mode
//   calph   -> calibrate with the standard buffer solution, two buffer solutions(4.0 and 7.0) will be automaticlly recognized
//   exitph  -> save the calibrated parameters and exit from PH calibration mode
//
//  EC Calibration：
//   enterec -> enter the EC calibration mode
//   calec   -> calibrate with the standard buffer solution, two buffer solutions(1413us/cm and 12.88ms/cm) will be automaticlly recognized
//   exitec  -> save the calibrated parameters and exit from EC calibration mode
//
#define PH_SAMPLES_NUM		10
#define EC_SAMPLES_NUM		10

#define RES2				820.0
#define ECREF				200.0

//#define DEBUG_PH
//#define DEBUG_EC

#include "DFRobot_PH.h"
#include "DFRobot_EC.h"

bool phSetupEnded = false;
bool ecSetupEnded = false;
unsigned long phTimer;
unsigned long ecTimer;
float neutralVoltage, acidVoltage;
float kvalueHigh, kvalueLow;
float PHavg, ECavg;

DFRobot_PH ph;
DFRobot_EC ec;

//------------------------- FUNCTIONS DECLARATION -----------------------
void PH_SensorInit();
void EC_SensorInit();

menu_type CalMenu[] = {
	{1, "SETUP"},						// il primo valore contiene la voce di menu selezionata per ultima
	{DS_SETUP_CALIBRATION_PH, "pH"},
	{DS_SETUP_CALIBRATION_EC, "EC/DTS"}
};

void Water_PH_Handler( float tmed ) {
	static float voltagePH[PH_SAMPLES_NUM];
	static float valuePH[PH_SAMPLES_NUM];
	static uint8_t ph_idx = 0;
	
	if(phSetupEnded) {
		if((phTimer + PH_READ_INTERVAL) < millis()) {								//time interval: 1s
			voltagePH[ph_idx] = analogRead(PIN_PH_SENSOR) / ADC_RESOLUTION * 5000; 	// read the PH voltage (0~3.0V)
			if(voltagePH[ph_idx] <= 3000) {											// max 3.0V = 3000mV
				valuePH[ph_idx] = ph.readPH(voltagePH[ph_idx], tmed);  				// convert voltage to pH with temperature compensation
			}
			PHavg = calcRingBufAverage(valuePH, PH_SAMPLES_NUM);

			#if defined(DEBUG_PH)		
				Serial.print("===> voltagePH: ");
				Serial.print(voltagePH[ph_idx]/1000);
				Serial.print(", valuePH: ");
				Serial.print(PHavg);
				Serial.print(", tmed: ");
				Serial.print(tmed);
				Serial.println();
			#endif
			if(dstatus == DS_IDLE) {
				if(main_page == 0) {
					printString(isnan(PHavg) ? NOT_AVAILABLE : ftoa(buff, PHavg), 15, 2);
				} else if(main_page == 2) {
					printString(isnan(PHavg) ? NOT_AVAILABLE : ftoa(buff, PHavg), 6, 1);
#if defined SR_PH
					printChar(relaisStatus(SR_PH)?'*':' ', 0, 1);
#endif
				}
			}
			if(++ph_idx >= PH_SAMPLES_NUM) ph_idx = 0;				// increment index and check for outbound
			phTimer = millis();										// reset PH timer
		}
	}
//	enable PH calibtration by remote
//	ph.calibration(calcRingBufAverage(voltagePH, PH_SAMPLES_NUM), tmed);     	// PH calibration process by Serial CMD
}

void Water_EC_Handler( float tmed ) {
	static float valueEC[EC_SAMPLES_NUM];
	static float voltageEC[EC_SAMPLES_NUM];
	static uint8_t ec_idx = 0;
	
	if(ecSetupEnded) {
		if((ecTimer + EC_READ_INTERVAL) < millis()) {								//time interval: 1s
			voltageEC[ec_idx] = analogRead(PIN_EC_SENSOR) / ADC_RESOLUTION * 5000; 	// read the EC voltage (0~3.4V)
			if(voltageEC[ec_idx] <= 3400) {											// max 3.4V = 3400mV
				valueEC[ec_idx] = ec.readEC(voltageEC[ec_idx], tmed);  				// convert voltage to EC with temperature compensation
			}
			ECavg = calcRingBufAverage(valueEC, EC_SAMPLES_NUM);

			#if defined(DEBUG_EC)		
				Serial.print("===> voltageEC: ");
				Serial.print(calcRingBufAverage(voltageEC, EC_SAMPLES_NUM)/1000);
				Serial.print(", valueEC: ");
				Serial.print(ECavg);
				Serial.print(", tmed: ");
				Serial.print(tmed);
				Serial.println();
			#endif
			if(dstatus == DS_IDLE) {
				if(main_page == 0) {
					printString(isnan(ECavg) ? NOT_AVAILABLE : ftoa(buff, ECavg), 15, 3);
				} else if(main_page == 2) {
					printString(isnan(ECavg) ? NOT_AVAILABLE : ftoa(buff, ECavg), 6, 2);
#if defined SR_EC
					printChar(relaisStatus(SR_EC)?'*':' ', 0, 1);
#endif
				}
			}
			if(++ec_idx >= EC_SAMPLES_NUM) ec_idx = 0;				// increment index and check for outbound
			ecTimer = millis();										// reset PH timer
		}
	}
}

void sensorsCalibration_PH( float tmed ) {
	static uint8_t phCalibrationStep;
	static float nVoltage, aVoltage;
	
	if(CheckInitBit(true)) {
		phCalibrationStep = 0;
		nVoltage = 0.0;
		aVoltage = 0.0;
		displayClear();
		printStringCenter("Put PH probe into", 0);
		printStringCenter("the 4.0 or 7.0", 1);
		printStringCenter("buffer solution", 2);
		DEBUG("Entered PH sensor calibration utility\n");
	} else if(phCalibrationStep == 0) {
		printBlinkingString(">> OK when ready <<");
		if(kp_new == IR_MENU) {
			SetInitBit(DS_SETUP_CALIBRATION);
			DEBUG("exit\n");
		} else if(kp_new == IR_OK) {
			phCalibrationStep++;
		}
	} else if(phCalibrationStep == 1) {
		float voltage = analogRead(PIN_PH_SENSOR) / ADC_RESOLUTION * 5000;	// read the PH voltage (0~3.0V)
		
		displayClear();
		printStringCenter("Buffer solution", 0);
		if((voltage > 1322) and (voltage < 1678)) {					// buffer solution:7.0{
			nVoltage = voltage;
			printStringCenter("PH 7.0", 1);
		} else if((voltage > 1854) and (voltage < 2210)) {			//buffer solution:4.0
			aVoltage = voltage;
			printStringCenter("PH 4.0", 1);
		} else {
			printStringCenter("ERROR", 1);
			printStringCenter("Try again", 2);
		}
		phCalibrationStep++;
	} else if(phCalibrationStep == 2) {
		printBlinkingString(">> OK  <<");
		if(kp_new == IR_MENU) {
			SetInitBit(DS_SETUP_CALIBRATION);
		} else if(kp_new == IR_OK)	{
			phCalibrationStep++;
		}
	} else if(phCalibrationStep == 3) {
		if(aVoltage == 0 and nVoltage == 0) {
			SetInitBit(DS_SETUP_CALIBRATION);
		} else {
			if(nVoltage > 0) {
				NvramWriteAnything(NVRAM_PH_NEUTRAL, nVoltage);
				StaticMemoryCommit();
			}
			if(aVoltage > 0) {
				NvramWriteAnything(NVRAM_PH_ACID, aVoltage);
				StaticMemoryCommit();
			}
			PH_SensorInit();
			SetInitBit(DS_SETUP_CALIBRATION);
		}
	}
}

void sensorsCalibration_EC( float tmed ) {
	static uint8_t ecCalibrationStep;
	static float kTemp, kTempHigh, kTempLow;
	static float voltage, KValue;
	
	if(CheckInitBit(true)) {
		ecCalibrationStep = 0;
		kTempHigh = 0.0;
		kTempLow = 0.0;
		displayClear();
		printStringCenter("Put EC probe into", 0);
		printStringCenter("1413us or 12.88ms/cm", 1);
		printStringCenter("buffer solution", 2);
		DEBUG("Entered EC sensor calibration utility\n");
	} else if(ecCalibrationStep == 0) {
		printBlinkingString(">> OK when ready <<");
		if(kp_new == IR_MENU) {
			SetInitBit(DS_SETUP_CALIBRATION);
			DEBUG("exit\n");
		} else if(kp_new == IR_OK) {
			displayClear();
			printString("Read value: ", 1, 1);
			ecCalibrationStep++;
		}
	} else {
		voltage = analogRead(PIN_EC_SENSOR) / ADC_RESOLUTION * 5000;	// read the EC voltage (0~3.4V)
		KValue = ec.readEC(voltage, tmed);								// convert voltage to EC with temperature compensation
		if(ecCalibrationStep == 1) {
			printString(ftoa(buff, KValue), 13, 1);
			if(kp_new == IR_MENU) {
				SetInitBit(DS_SETUP_CALIBRATION);
			} else if(kp_new == IR_OK) {
				ecCalibrationStep++;
			}
			printBlinkingString(">> OK  <<");
			delay(100);
		} else if(ecCalibrationStep == 2) {
			printStringCenter("Buffer solution", 0);					// 1413us/cm or 12.88ms/cm
			float rawEC = ec.getRawEC();
			if((rawEC > 0.9) && (rawEC < 1.9)) {
				printStringCenter("1413us/cm", 1);
				kTempLow = 1.413*(1.0+0.0185*(tmed-25.0));
			} else if((rawEC > 9) && (rawEC < 16.8)) {
				printStringCenter("12.88ms/cm", 1);
				kTempHigh = 12.88*(1.0+0.0185*(tmed-25.0));
			} else {
				printStringCenter("ERROR", 1);
				printStringCenter("Try again", 2);
			}
			ecCalibrationStep++;
		} else if(ecCalibrationStep == 3) {
			printBlinkingString(">> OK  <<");
			if(kp_new == IR_MENU) {
				SetInitBit(DS_SETUP_CALIBRATION);
			} else if(kp_new == IR_OK)	{
				ecCalibrationStep++;
			}
		} else if(ecCalibrationStep == 4) {
			if(kTempHigh == 0 and kTempLow == 0) {
				SetInitBit(DS_SETUP_CALIBRATION);
			} else {
				if(kTempHigh > 0) {
					NvramWriteAnything(NVRAM_EC_KVALUEHIGH, kTempHigh);
					StaticMemoryCommit();
					kvalueHigh = kTempHigh;
				}
				if(kTempLow > 0) {
					NvramWriteAnything(NVRAM_EC_KVALUELOW, kTempLow);
					StaticMemoryCommit();
					kvalueLow = kTempLow;
				}
				PH_SensorInit();
				SetInitBit(DS_SETUP_CALIBRATION);
			}
		}
	}
}

void PH_SensorInit() {
	int count;
	DEBUG("PH sensor values: ");
	
	count = NvramReadAnything(NVRAM_PH_NEUTRAL, neutralVoltage);
	if(neutralVoltage <= 0 or neutralVoltage > 0xFFFF or count != 4) {		// if value not set, write defaults to nvram
		neutralVoltage = 1500.00;
		NvramWriteAnything(NVRAM_PH_NEUTRAL, neutralVoltage);
		StaticMemoryCommit();
		count = NvramReadAnything(NVRAM_PH_NEUTRAL, neutralVoltage);
	}
	DEBUG("neutral = %s (%d bytes)", ftoa(buff, neutralVoltage), count);

	count = NvramReadAnything(NVRAM_PH_ACID, acidVoltage);
	if(acidVoltage <= 0 or acidVoltage > 0xFFFF or count != 4) {
		acidVoltage = 2032.44;
		NvramWriteAnything(NVRAM_PH_ACID, acidVoltage);
		StaticMemoryCommit();
		count = NvramReadAnything(NVRAM_PH_ACID, acidVoltage);
	}
	DEBUG(", acid = %s (%d bytes)", ftoa(buff, acidVoltage), count);

    ph.begin(neutralVoltage, acidVoltage);
	phTimer = millis();						// start PH measuring timer
	phSetupEnded = true;
	DEBUG(" ==> OK\n");
}

void EC_SensorInit() {
	int count;
	DEBUG("EC sensor K values: ");
	
	count = NvramReadAnything(NVRAM_EC_KVALUEHIGH, kvalueHigh);
	if(kvalueHigh <= 0.00 or kvalueHigh > 0xFFFF or count != 4) {
		kvalueHigh = 1.00;
		NvramWriteAnything(NVRAM_EC_KVALUEHIGH, kvalueHigh);
		StaticMemoryCommit();
		count = NvramReadAnything(NVRAM_EC_KVALUEHIGH, kvalueHigh);
	}
	DEBUG("high = %s (%d bytes)", ftoa(buff, kvalueHigh), count);
	
	count = NvramReadAnything(NVRAM_EC_KVALUELOW, kvalueLow);
	if(kvalueLow <= 0.00 or kvalueLow > 0xFFFF or count != 4) {
		kvalueLow = 1.00;
		NvramWriteAnything(NVRAM_EC_KVALUELOW, kvalueLow);
		StaticMemoryCommit();
		count = NvramReadAnything(NVRAM_EC_KVALUELOW, kvalueLow);
	}
	DEBUG(", low = %s (%d bytes)", ftoa(buff, kvalueLow), count);
    ec.begin(kvalueLow, kvalueHigh);
	ecTimer = millis();											// start EC measuring timer
	ecSetupEnded = true;
	DEBUG(" ===> OK\n");
}

