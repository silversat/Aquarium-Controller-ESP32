//
//	ALARM.h
//
// Dichiarazioni e funzione per i segnali acustici

#define SERIAL_RX_PIN			16
#define SERIAL_TX_PIN			17
#define ALARM_SERIAL_BAUD		9600
#define SIRENE_RESET_TIMER		5000			// ping period in miilis 
#define PONG_RESPONSE_TIME		2000			// max response-wait-time in millis

bool alarmStatus = false;
bool AlarmSirenePresent = false;
unsigned long sirene_millis;
unsigned long ping_millis;
extern bool alrmsonoro;
extern int defcon;
String inputString;	     		    			// a string to hold incoming data

void alarm( bool status, bool cycle ) {
	if(AlarmSirenePresent) {
		if(status) {
			Serial2.print("alarm_on");
		} else {
			Serial2.print("no_alarm");
		}
	} else {	
		if(status and cycle) {
			beepOn();
		} else {
			beepOff();
		}
	}
}

void AlarmSireneHandle() {
	if((millis() - sirene_millis > SIRENE_RESET_TIMER) or (millis() < sirene_millis)) {	
		sirene_millis = millis();
		ping_millis = millis();
		Serial2.print("ping");
	}

	if(ping_millis > 0) {
		if(millis() < ping_millis) {	
			ping_millis = millis();
		}
		if(millis() - ping_millis > PONG_RESPONSE_TIME) {
			if(Serial2.available()) {
				inputString = Serial2.readString();
				if(inputString == "pong") {
					if(!AlarmSirenePresent) {
						DEBUG("Alarm sirene OK\n");
						AlarmSirenePresent = true;
					}
				}
			} else {
				if(AlarmSirenePresent) {
					DEBUG("Alarm sirene is Down\n");
					AlarmSirenePresent = false;
				}
			}
			ping_millis = 0;
		}
	}
}

void AlarmInit() {
	Serial2.begin(ALARM_SERIAL_BAUD);//, SERIAL_8N1, SERIAL_RX_PIN, SERIAL_TX_PIN);			// serial port #2 used for automatic alarm system. default pin RX:16 TX:17
	delay(100);																			// wait serial port to stabilize
	Serial2.setTimeout(300); 
	DEBUG("Alarm sirene OK: waiting for connection\n");
}
