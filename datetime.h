//
//	DateTime.h
//
/*	tm_sec		Seconds, between 0 and 60 inclusive (60 allows for leap seconds).
	tm_min		Minutes, between 0 and 59 inclusive.
	tm_hour		Hours, between 0 and 23 inclusive.
	tm_mday		Day of the month, between 1 and 31 inclusive.
	tm_mon		Month, between 0 (January) and 11 (December).
	tm_year		Year (since 1900), can be negative for earlier years.
	tm_wday1	Day of week, between 0 (Sunday) and 6 (Saturday).
	tm_yday		Number of days elapsed since last January 1, between 0 and 365 inclusive.
	tm_isdst	Daylight Savings Time flag: positive means DST in effect, zero means DST not in effect, 
				negative means no information about DST is available. Although for mktime(), 
				negative means that it should decide if DST is in effect or not. 

	int    tm_sec;      Seconds [0,59]. 
	int    tm_min;      Minutes [0,59]. 
	int    tm_hour;     Hour [0,23]. 
	int    tm_mday;     Day of month [1,31]. 
	int    tm_mon;      Month of year [0,11]. 
	int    tm_year;     Years since 1900. 
	int    tm_wday;     Day of week [0,6] (Sunday =0). 
	int    tm_yday;     Day of year [0,365]. 
	int    tm_isdst;    Daylight Savings flag. 
*/

#if defined(USE_RTC_DS3231) || defined(USE_RTC_DS1307)
	#include "RTClib.h"
	#if defined(USE_RTC_DS3231)
		RTC_DS3231	rtc;
	#elif defined(USE_RTC_DS1307)
		RTC_DS1307	rtc;
	#endif
#endif

extern		void LoadAllLightStatus();
extern		void restartLights();
			void RtcInit();

char* 		ntpserver = NULL;
byte 		giornimese[12] = {31,28,31,30,31,30,31,31,30,31,30,31};  // creo un'array per il controllo sull'immisione dei giorni
bool 		ntpswitch = false;
bool 		rtcSetupEnded = false;
bool 		extRtcPresent = false;			// if an external RTC in present and running
bool		fastTimeRun = false;
double 		ntpPollTimer = 0;
double 		ntpPollTimerTarget;
struct tm	datetime;

void checkTimeConsistency( tm *dt ) {
	if(dt->tm_mday < 1 or dt->tm_mday > 31) dt->tm_mday = 1;
	if(dt->tm_mon < 1 or dt->tm_mon > 12) dt->tm_mon = 1;
	if(dt->tm_year < 1900) dt->tm_year = 1900;
	if(dt->tm_hour < 0 or dt->tm_hour > 23) dt->tm_hour = 0;
	if(dt->tm_min < 0 or dt->tm_min > 59) dt->tm_min = 0;
	if(dt->tm_sec < 0 or dt->tm_sec > 59) dt->tm_sec = 0;
}

#if defined(USE_RTC_DS3231) || defined(USE_RTC_DS1307)
bool getExternalRtcTime( struct tm *dt ) {
	bool ret = false;

	if(extRtcPresent) {

#if defined(USE_RTC_DS3231)
		if(rtc.lostPower())
#elif defined(USE_RTC_DS1307)
		if(!rtc.isrunning())
#endif		
		{
			DEBUG("External RTC lost power, getting compile time\n");
			// following line sets the RTC to the date & time this sketch was compiled
			rtc.adjust(DateTime(__DATE__,__TIME__));
			// This line sets the RTC with an explicit date & time, for example to set
			// January 21, 2014 at 3am you would call:
			// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
		}
		DateTime dtim = rtc.now();
		dt->tm_year = dtim.year();
		dt->tm_mon = dtim.month();
		dt->tm_mday = dtim.day();
		dt->tm_hour = dtim.hour();
		dt->tm_min = dtim.minute();
		dt->tm_sec = dtim.second();
		checkTimeConsistency(dt);
		ret = true;
	}
	return ret;
}
#endif

bool getRtcTime( struct tm *dt ) {
	bool ret = false;
	
	if(getLocalTime(dt)) {
		dt->tm_year += 1900;
		dt->tm_mon += 1;
		checkTimeConsistency(dt);
		ret = true;
#if defined(USE_RTC_DS3231) || defined(USE_RTC_DS1307)
	} else {
		ret = getExternalRtcTime(dt);
#endif	
	}
	return ret;
}

void setRtcTime( tm dtimp ) {
	datetime.tm_year = dtimp.tm_year;
	datetime.tm_mon = dtimp.tm_mon;
	datetime.tm_mday = dtimp.tm_mday;
	datetime.tm_hour = dtimp.tm_hour;
	datetime.tm_min = dtimp.tm_min;
	datetime.tm_sec = dtimp.tm_sec;
#if defined(USE_RTC_DS3231) || defined(USE_RTC_DS1307)
	rtc.adjust(DateTime(datetime.tm_year, datetime.tm_mon, datetime.tm_mday, datetime.tm_hour, datetime.tm_min, datetime.tm_sec));
#endif		
}

uint32_t myMillis() {
	if(fastTimeRun) {
		return millis()*120;		// 120mS al mS = 2min/sec
	} else {
		return millis();
	}
}

tm getDateTime( bool renewTime=false, int incHour=0 ) {
	if(fastTimeRun or renewTime) {
		static uint32_t timer = myMillis();
		
		if(renewTime) {
			getRtcTime(&datetime);
			timer = myMillis();
		}

		if(incHour != 0) {						// manually increment/decrement hour (key up/down)
			datetime.tm_hour += incHour;
			datetime.tm_sec = 0;
			if(datetime.tm_hour >= 24) datetime.tm_hour -= 24;
			if(datetime.tm_hour < 0) datetime.tm_hour += 24;
			restartLights();
		}
		
		if(myMillis() - timer > 10000) {			// 1min/500mS = 2min/sec
			datetime.tm_min++;
			datetime.tm_sec = 0;
			if(datetime.tm_min >= 60) {
				datetime.tm_min -= 60;
				datetime.tm_hour++;
				if(datetime.tm_hour >= 24) datetime.tm_hour -= 24;
				if(datetime.tm_hour < 0) datetime.tm_hour += 24;
			}
			timer = myMillis();
		}
	} else {
		getRtcTime(&datetime);
	}
	return datetime;
}

void printTime( tm dt, uint8_t col=0, uint8_t row=1 ) {     		// Creata per semplificare il codice dell'impostazione di data e ora
	char buff[12];
	sprintf(buff, "%02d:%02d:%02d", dt.tm_hour, dt.tm_min, dt.tm_sec);
	printString(buff, col, row);
}

void printDateTime( tm dt, uint8_t col=0, uint8_t row=1 ) {     	// Creata per semplificare il codice dell'impostazione di data e ora
	char buff[24];
	sprintf(buff, "%02d-%02d-%04d  %02d:%02d:%02d", dt.tm_mday, dt.tm_mon, dt.tm_year, dt.tm_hour, dt.tm_min, dt.tm_sec);
	printString(buff, col, row);
}

void displayRunStatus( bool status, uint8_t row ) {
	if(status) {
		printStringCenter(" Fast ", row);
	} else {
		printStringCenter("Normal", row);
	}
}

void fastTimeSetup() {
	static bool confirm, status;
	
	if(CheckInitBit(true)) {					// if di inizializzazazione della procedura: viene eseguita una sola volta
		status = fastTimeRun;					// false=NORMAL, true=FAST	
		displayClear();
		printStringCenter("Time Run Mode", 0);
		displayRunStatus( status, 2);
		confirm = true;
	}
	if(kp_new == IR_OK and confirm) {
		confirm = false;
		kp_new = IR_NONE;		// avoid 'conferma' skipping
	} else if(kp_new == IR_RIGHT or kp_new == IR_UP or kp_new == IR_LEFT or kp_new == IR_DOWN) {
		status = !status;
		displayRunStatus( status, 2);
	} else if(kp_new == IR_MENU) {
		SetInitBit(DS_SETUP);
	}

	if(!confirm) {
		if(fastTimeRun != status) {
			fastTimeRun = status;
			if(!fastTimeRun) {
				getDateTime(true);
			}
			LoadAllLightStatus();
		}
		SetInitBit(DS_SETUP);
	}
}

void saveNvramDatetime() {
	updateStaticMemory(NVRAM_NTP_ENABLED, ntpswitch);
	updateStaticMemoryInt(NVRAM_NTP_POLL_MINS, int(ntpPollTimerTarget/MILLISECS_PER_MIN));
	updateStaticMemoryString(ntpserver, NVRAM_NTP_SERVER, NVRAM_NTP_SERVER_LEN);
	StaticMemoryCommit();
	RtcInit();
}

tm ImpostaDataOra() {  						// Funzione per impostazione data
	static uint8_t datotempo;
	static bool confirm = true;
	static struct tm dtimp;
	
	if(CheckInitBit(true)) {				// if di inizializzazazione della procedura: viene eseguita una sola volta
		getRtcTime(&dtimp);
		datotempo = 1;
		displayClear();
		printStringCenter("Date-Time Setup", 0);

		if((datetime.tm_year % 4) == 0) {
			giornimese[1] = 29;				// se l'anno e' bisestile febbraio ha 29 giorni...
		} else {
			giornimese[1] = 28;				// altrimenti febbraio ha 28 giorni
		}
	}
	printDateTime(dtimp);					// Stampo data e ora

	if(confirm == true) {	
		switch(kp_new) {
			case IR_OK:	
				datotempo = 10;
				kp_new = IR_NONE;		// avoid 'conferma' skipping
				confirm = false; 		// disattivo questa if in modo che Il tasto OK funzioni solo con le if di case datotempo = 6
				break;
			case IR_MENU:
				SetInitBit(DS_SETUP);
				break;
			case IR_RIGHT:
				ScrollHandler(datotempo, 1, 6, ACT_INC);
				break;
			case IR_LEFT:
				ScrollHandler(datotempo, 1, 6, ACT_DEC);
				break;
			default:
				break;
		}		
	}

	switch(datotempo) { 
		case 1:  											// Imposto giorno
			stampafrecce(0, 2, 0, 18);						// print two arrows
			displayClearRow(3);
			if(kp_new == IR_UP) {
				ScrollHandler((uint8_t&)dtimp.tm_mday, 1, giornimese[dtimp.tm_mon-1], ACT_INC);		// doesn't work correctly on year change!!!!!!!
			} else if(kp_new == IR_DOWN) {
				ScrollHandler((uint8_t&)dtimp.tm_mday, 1, giornimese[dtimp.tm_mon-1], ACT_DEC);
			}
			break;

		case 2:  									// Imposto mese
			stampafrecce(0, 2, 3, 15);
			if(kp_new == IR_UP) {
				ScrollHandler((uint8_t&)(dtimp.tm_mon), 1, 12, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler((uint8_t&)(dtimp.tm_mon), 1, 12, ACT_DEC);
			}
			break;

		case 3:  						 			// Imposto anno
			stampafrecce(0, 2, 6, 4, 4);
			if(kp_new == IR_UP) {
				dtimp.tm_year++;
			} else if(kp_new == IR_DOWN) {
				dtimp.tm_year--;
			}
			break;

		case 4:  									// Imposto ora
			stampafrecce(0, 2, 12, 5);
			if(kp_new == IR_UP) {
				ScrollHandler((uint8_t&)dtimp.tm_hour, 0, 23, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler((uint8_t&)dtimp.tm_hour, 0, 23, ACT_DEC);
			}
			break;

		case 5:  									// Imposto minuti
			stampafrecce(0, 2, 15, 3);
			if(kp_new == IR_UP) {
				ScrollHandler((uint8_t&)dtimp.tm_min, 0, 59, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler((uint8_t&)dtimp.tm_min, 0, 59, ACT_DEC);
			}
			break;

		case 6:  									// Imposto secondi
			stampafrecce(0, 2, 18, 0);
			if(kp_new == IR_UP) {
				ScrollHandler((uint8_t&)dtimp.tm_sec, 0, 59, ACT_INC);
			} else if(kp_new == IR_DOWN) {
				ScrollHandler((uint8_t&)dtimp.tm_sec, 0, 59, ACT_DEC);
			}
			break;

		case 10:
			if(!confirm) {	
				printBlinkingString(confirm_msg);
				if(kp_new == IR_OK or kp_new == IR_MENU) {	
					if(kp_new == IR_OK) {
						setRtcTime(dtimp);
					}	
					confirm = true;
					dstatus = DS_IDLE_INIT;
				}
				if(kp_new == IR_RIGHT || kp_new == IR_LEFT || kp_new == IR_UP || kp_new == IR_DOWN) {	
					datotempo = 1;
					confirm = true;
				}
			}
			break;
	}
}

char* getNTPserver( char* ntpsrv ) {
	char buffer[NVRAM_NTP_SERVER_LEN+1];
	readStaticMemoryString(buffer, NVRAM_NTP_SERVER, NVRAM_NTP_SERVER_LEN);
	if(strlen(buffer) == 0) {
		getP(buffer, ntp_default);
	}
	if(ntpsrv != NULL) delete(ntpsrv);
	ntpsrv = new char[strlen(buffer)+1];
	strcpy(ntpsrv, buffer);
	DEBUG("NTP server set to '%s'\n", ntpsrv);
	return ntpsrv;
}

void getNTPtime() {
	const long  gmtOffset_sec = 3600;
	const int   daylightOffset_sec = 3600;
	/*
		int    tm_sec;   //   Seconds [0,59]. 
		int    tm_min;   //   Minutes [0,59]. 
		int    tm_hour;  //   Hour [0,23]. 
		int    tm_mday;  //   Day of month [1,31]. 
		int    tm_mon;   //   Month of year [0,11]. 
		int    tm_year;  //   Years since 1900. 
		int    tm_wday;  //   Day of week [0,6] (Sunday =0). 
		int    tm_yday;  //   Day of year [0,365]. 
		int    tm_isdst; //   Daylight Savings flag. 
	*/

	configTime(gmtOffset_sec, daylightOffset_sec, ntpserver);
}

void RtcInit() {
	bool ntpsync = false;
	
	ntpPollTimerTarget = readStaticMemoryInt(NVRAM_NTP_POLL_MINS);
	if(ntpPollTimerTarget <= 0 or ntpPollTimerTarget > MINUTES_PER_DAY) {
		ntpPollTimerTarget = 60;
		writeStaticMemoryInt(NVRAM_NTP_POLL_MINS, ntpPollTimerTarget);
		StaticMemoryCommit();
	}
	DEBUG("NTP poll period set to %d mins\n", int(ntpPollTimerTarget));
	ntpPollTimerTarget *= MILLISECS_PER_MIN;	// 1 min = 60secs * 1000mSecs

	ntpserver = getNTPserver(ntpserver);
	ntpswitch = readStaticMemory(NVRAM_NTP_ENABLED);
	if(ntpswitch) {
		getNTPtime();
		ntpPollTimer = millis();					// start timer
		DEBUG("NTP enabled\n");
	} else {
		DEBUG("NTP disabled\n");
	}

	ntpsync = getRtcTime(&datetime);
	if(ntpsync) {
		DEBUG("Internal RTC set to NTP time: %02d-%02d-%04d %02d:%02d:%02d\n", datetime.tm_mday, datetime.tm_mon, datetime.tm_year, datetime.tm_hour, datetime.tm_min, datetime.tm_sec );
	} else {
		DEBUG("Failed to obtain NTP time\n");
	}
	
#if defined(USE_RTC_DS3231) || defined(USE_RTC_DS1307)
	if(extRtcPresent = rtc.begin(false)) {		// do not begin wire!
		DEBUG("%s RTC module found @ 0x%02X, time got from ", rtc.getDeviceType(), rtc.getDeviceAddress());
		if(ntpsync) {
			rtc.adjust(DateTime(datetime.tm_year, datetime.tm_mon, datetime.tm_mday, datetime.tm_hour, datetime.tm_min, datetime.tm_sec));
			DEBUG("internal RTC\n");
		} else {
			getExternalRtcTime(&datetime);
			DEBUG("external RTC\n");
		}
	} else {
		DEBUG("Couldn't find RTC module\n");
	}
#endif

	fastTimeRun = false;
	rtcSetupEnded = true;
	DEBUG("Current time is %02d-%02d-%4d %02d:%02d:%02d\n", datetime.tm_mday, datetime.tm_mon, datetime.tm_year, datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
}
