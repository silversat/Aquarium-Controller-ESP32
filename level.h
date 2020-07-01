//
//	LEVEL.h
//
bool levelSetupEnded = false;
uint8_t Liquid_level = NULL;

void WaterLevelHandler() {
	if(levelSetupEnded) {
		bool cur_level = digitalRead(PIN_LEVEL_SENSOR);
		if(cur_level != Liquid_level) {
			Liquid_level = cur_level;
			DEBUG("Water level now is %s\n", Liquid_level?"FULL":"LOW");
#if defined SR_WATER_LEVEL
			relais(SR_WATER_LEVEL, Liquid_level?RL_OFF:RL_ON); 
#endif
		}
		if(main_page == 1 and dstatus == DS_IDLE) {
			printString(Liquid_level?"FULL":"LOW ", 13, 1);
#if defined SR_WATER_LEVEL
			printChar(relaisStatus(SR_WATER_LEVEL)?'*':' ', 0, 1);
#endif
		}
	}
}

void LevelSensorInit() {
	pinMode(PIN_LEVEL_SENSOR, INPUT_PULLUP); 
#if defined SR_WATER_LEVEL
	pinMode(SR_WATER_LEVEL, OUTPUT); 
	relais(SR_WATER_LEVEL, RL_OFF); 
#endif
	Liquid_level = 0xAA;				// nonsense data
	levelSetupEnded = true;
	DEBUG("Level sensor OK\n");
}
