//============================================================================================
//
//		AQUARIUM CONTROLLER
//		version 		@ SK_VERSION
//		last rev.date 	@ SK_DATE
//
//		Original project by riciweb (http://forum.arduino.cc/index.php?topic=141419.0)
//		modded by silversat (09/2013)
//
//		This controller has been developed onto an Arduino MEGA2560 board. 
//		The hardware includes:
//		- 8 relais parallel board (hobbycomponents.com)
//		- I2C 20x4 LCD display or i2c OLED 32x128, 64x128, 128x128
//		- uses esp32 internal RTC/nvram
//		- one or two DS18b20 temperature sensor
//		- IR receiver (TK19) associated with an apple tv remote control
//		- a buzzer
//
//		Most of the hardware is configurable (configuration.h)
//
//		Compile with 	Board = ESP32 Dev Module
//						Partition scheme = Default 4MB with spiffs (1.2MB App/1.5MB SPIFFS)
//
//============================================================================================
#define SK_VERSION				"3.9.1"
#define SK_DATE					"01-07-2020"
#define SK_AUTHOR				"c.benedetti"
#define SK_FEATURE				"esp32"

//#define IR_KEYBOARD
#define INTERRUPT_ATTR IRAM_ATTR
#define DEBUG(...) Serial.printf( __VA_ARGS__ )		// activate debug
#ifndef DEBUG 
	#define DEBUG(...)
#else
	#define DEBUG_MESSAGES
#endif

#if not defined(ARDUINO_ARCH_ESP32)
	#message "This scetch is written for multicore ESP32 mcu"
#endif

#include <math.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <ArduinoOTA.h>
#include <SPIFFS.h>

#define BUFF_SIZE			24
#define MAIN_PAGE_FIRST		0
#define MAIN_PAGE_LAST		3
#define	CORE_0				0
#define	CORE_1				1

typedef struct {
	char* 			ssid = NULL;
	char*			password = NULL;
} net_type;
net_type *nets = NULL;

uint8_t 		parameters_save;
uint8_t			dstatus;					// dispatcher status container
uint16_t 		pwm_resolution;
uint16_t 		kp_new;
char 			buff[BUFF_SIZE];
int				main_page = 0;
const char		*confirm_msg = "* CONFIRM *";

#include "configuration.h"				// load board configuration (pins, etc)
#include "functions.h"
#include "keyboard.h"
#include "buzzer.h"
#include "alarm.h"
#if defined(OLED_32) || defined(OLED_64) || defined(OLED_128)
	#include "oled_i2c.h"
#else
	#include "lcd_i2c.h"
#endif
#include "p_memory.h"
#include "nvram.h"
#include "datetime.h"
#include "menu.h"
#include "lights.h"
#if defined(RELE_PARALLEL)				// see defines in config.h
	#include "scheda_rele.h"
#else
	#include "scheda_rele_i2c.h"
#endif
#include "temperature.h"
#include "ph_ec_sensor.h"
#include "level.h"
#include "turbidity.h"

//TaskHandle_t 	Task1;
//TaskHandle_t 	Task2;

IPAddress 		moduleIp;
AsyncWebServer	webServer(HTTP_PORT);
AsyncWebSocket 	webSocket("/ws");
//AsyncEventSource events("/events");
WiFiMulti 		wifiMulti;
WiFiClient	 	client;

uint8_t 		starting_mode;
uint8_t 		restart_mode = NO_RESTART;
char* 			modulename;
char* 			admin_password;
float 			tmed = 0;
int 			deviceMode;
int				ssidRequestTrigger = NULL;
bool			bSsid, bName, bAdmp;
bool			otaEnabled = false;
bool			otaInProgress = false;
double 			restartTimer = 0;
double 			restartTimerTarget;


//==============================================================================
//							SERVICE HANDLER FUNCTIONS
//==============================================================================
void dispatcher() {
	switch(dstatus & DS_INIT_MASK) {
		case DS_IDLE:
			NormalOperation();
			break;
			
		case DS_SETUP:
			ScorriMenu(MainMenu, sizeof(MainMenu));
			break;
							
		case DS_SETUP_DATETIME:
			datetime = ImpostaDataOra(); 
			break;

		case DS_SETUP_CALIBRATION:
			ScorriMenu(CalMenu, sizeof(CalMenu));
			break;

		case DS_SETUP_CALIBRATION_PH:
			sensorsCalibration_PH(tmed);
			break;

		case DS_SETUP_CALIBRATION_EC:
			sensorsCalibration_EC(tmed);
			break;

		case DS_SETUP_TIMERUN:
			fastTimeSetup();
			break;

		case DS_SETUP_TEMP:
			WaterTemperatureSetup(); 
			break;

		case DS_SETUP_INFOLIGHTS:
			InfoLuci();
			break;
		
		case DS_SETUP_LIGHTS:
			ScorriMenu(LightsMenu, sizeof(LightsMenu));
			break;
		
		case DS_SETUP_LIGHTS_0:
			ImpostaFunzLinee();
			break;

		case DS_SETUP_LIGHTS_1:
			ImpDatiFotoperiodo(LIGHT_LINE_1);
			break;

		case DS_SETUP_LIGHTS_2:
			ImpDatiFotoperiodo(LIGHT_LINE_2);
			break;

		case DS_SETUP_LIGHTS_3:
			ImpDatiFotoperiodo(LIGHT_LINE_3);
			break;

		case DS_SETUP_LIGHTS_4:
			ImpDatiFotoperiodo(LIGHT_LINE_4);
			break;

		case DS_SETUP_LIGHTS_5:
			ImpDatiFotoperiodo(LIGHT_LINE_5);
			break;

		case DS_SETUP_LIGHTS_6:
			ImpDatiFotoperiodo(LIGHT_LINE_6);
			break;
		
		default:
			NormalOperation();
	}
}

void handleTimers() {
	if(ntpswitch) {
		if((ntpPollTimer + ntpPollTimerTarget) < millis()) {
			getNTPtime();
			ntpPollTimer = millis();				// restart timer
		} else if(ntpPollTimer > millis()) {		// if timer rolled over the 0
			ntpPollTimer = millis();				// then restart it
		}
	}
	if(webSocket.count() > 0) {
		char buf[16];
		static tm time = datetime;
		if(time.tm_sec != datetime.tm_sec) {
			time.tm_hour = datetime.tm_hour;
			time.tm_min = datetime.tm_min;
			time.tm_sec = datetime.tm_sec;
			webSocket.printfAll("TIME=%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);

			if(time.tm_mday != datetime.tm_mday) {
				time.tm_mday = datetime.tm_mday;
				time.tm_mon = datetime.tm_mon;
				time.tm_year = datetime.tm_year;
				webSocket.printfAll("DATE=%02d-%02d-%04d", time.tm_mday, time.tm_mon, time.tm_year);
			}
			
			static int dlavg = 0;
			if(dlavg != calcLuxAverage()) {
				dlavg = calcLuxAverage();
				webSocket.printfAll("DLAVG=%3d&#37;", dlavg);
			}
			
			static int llavg = 0;
			if(llavg != calcLunarAverage()) {
				llavg = calcLunarAverage();
				webSocket.printfAll("LLAVG=%3d&#37;", llavg);
			}

			if(Tsensor1 or Tsensor2) {
				static int watertemp = 0;
				if(watertemp != tmed) {
					watertemp = tmed;
					webSocket.printfAll("WATER_TEMP=%s", ftoa(buf, tmed));
				}
			}

			static bool waterlevel = false;
			if(waterlevel != Liquid_level) {
				waterlevel = Liquid_level;
				webSocket.printfAll("WATER_LEVEL=%s", waterlevel?"FULL":"LOW ");
			}

			if(!isnan(ntu)) {
				static int waterturb = 0;
				if(waterturb != ntu) {
					waterturb = ntu;
					webSocket.printfAll("WATER_TURB=%s", ftoa(buf, waterturb));
				}
			}

			if(!isnan(PHavg)) {
				static int waterph = 0;
				if(waterph != PHavg) {
					waterph = PHavg;
					webSocket.printfAll("WATER_PH=%s", ftoa(buf, waterph));
				}
			}

			if(!isnan(ECavg)) {
				static int waterec = 0;
				if(waterec != ECavg) {
					waterec = ECavg;
					webSocket.printfAll("WATER_EC=%s", ftoa(buf, waterec));
				}
			}

			static bool rele[SR_RELAIS_NUM];
			for(uint8_t i = 0; i < SR_RELAIS_NUM; i++) {
				uint8_t idx = (i+1);
				if(rele[i] != relaisStatus(idx)) {
					rele[i] = relaisStatus(idx);
					webSocket.printfAll("RELE%d=%s", idx, rele[i]?"ON ":"OFF");
				}
			}

			static uint8_t light[LIGHT_LINE_NUMBER];			// 0=dark, 1=Inc, 2=Dec, 3=full
			PlafoTemp tmp;
			for(uint8_t i = 0; i < LIGHT_LINE_NUMBER; i++) {
				getPlafoAdjustedTimings(&tmp, i);
				buf[0] = '\0';
				
				if(tmp.minsCurr >= tmp.minsOn and tmp.minsCurr < tmp.minsFA) {				//  - alba
					if(light[i] != LIGHT_INC) {
						light[i] = LIGHT_INC;
						strcpy(buf, "orange>Inc");
					}						
				} else if(tmp.minsCurr >= tmp.minsFA and tmp.minsCurr < tmp.minsIT) {		//	- luce piena
					if(light[i] != LIGHT_FULL) {
						light[i] = LIGHT_FULL;
						strcpy(buf, "red>Full");
					}						
				} else if(tmp.minsCurr >= tmp.minsIT and tmp.minsCurr < tmp.minsOff) {		// 	- tramonto
					if(light[i] != LIGHT_DEC) {
						light[i] = LIGHT_DEC;
						strcpy(buf, "orange>Dec");
					}						
				} else {																	//  - buio
					if(light[i] != LIGHT_DARK) {
						light[i] = LIGHT_DARK;
						strcpy(buf, "black>Dark");
					}						
				}
				
				if(strlen(buf) > 0) {
					webSocket.printfAll("LIGHT%d=<font color=%s</font>", i, buf);
				}
			}
			webSocket.cleanupClients();
		} else if(ssidRequestTrigger != NULL) {						// websocket ssid request handler
			int cli = ssidRequestTrigger;
			ssidRequestTrigger = NULL;								// reset trigger
			
			int result = WiFi.scanNetworks();
			String buff = "NETS=Networks found: " + String(result) + "<br>";
			webSocket.text(cli, buff);
			for(int i = 0; i < result; i++) {
				buff = String("NETS=");
				buff += String(i+1) + ") " + WiFi.SSID(i);
				buff += ", rssi: " + String(WiFi.RSSI(i));
				buff += ", enc: " + String(decodeEncryption(WiFi.encryptionType(i)));
				buff += ", chan: " + String(WiFi.channel(i));
				buff += "<br>";
				webSocket.text(cli, buff);
			}
		}
	}
}

void handleParametersSave( uint8_t whatToSave ) {
	if(whatToSave == SAVE_SSID_PSW) {
		saveNvramSSIDPSW();
	} else if(whatToSave == SAVE_MODULENAME) {
		saveNvramNAME();
	} else if(whatToSave == SAVE_TIMERS) {
		saveNvramSystemTimers();
	} else if(whatToSave == SAVE_LIGHTS) {
		saveNvramLightStatus();
	} else if(whatToSave == SAVE_DATETIME) {
		saveNvramDatetime();
	} else if(whatToSave == SAVE_TEMP) {
		saveNvramTemperature();
	} else if(whatToSave == SAVE_OTA) {
		saveNvramOTA();
	} else if(whatToSave == SAVE_RUNMODE) {
		saveRunMode();
	} else if(whatToSave == SAVE_ADMIN) {
		saveNvramAdminPsw();
	} else if(whatToSave == SAVE_RESTART) {
		saveNvramRestart();
	}
	parameters_save = SAVE_NOTHING;
}
	
void handleRestartTimer() {
	if((restartTimer + restartTimerTarget) < millis()) {
		ESP.restart();
	} else if(restartTimer > millis()) {		// if timer rolled over the 0
		restartTimer = millis();				// then restart it
	}
}

bool resetRestartTimer() {
	restartTimer = millis();
}

void NormalOperation() {
	if(kp_new == IR_MENU) {
		dstatus = DS_SETUP;								// switch dispatcher to enter menu
		SetInitBit();
	} else if(kp_new == IR_OK) {
		main_page = MAIN_PAGE_FIRST;
		SetInitBit();
	} else if(kp_new == IR_RIGHT) {
		main_page++;
		if(main_page > MAIN_PAGE_LAST) main_page = MAIN_PAGE_FIRST;
		SetInitBit();
	} else if(kp_new == IR_LEFT) {
		main_page--;
		if(main_page < MAIN_PAGE_FIRST) main_page = MAIN_PAGE_LAST;
		SetInitBit();
	} else {	
		if(CheckInitBit(true)) {							// check and clear init bit.
			displayClear();									// clear display
			if(main_page == MAIN_PAGE_FIRST) {
				printString("AQUARIUM CONTROLLER", 0, 0);	// Scrivo sul display il titolo della schermata 
				printString("T:", 0, 2);
				printString("PH:", 12, 2);
				printString("EC:", 12, 3);
			} else if(main_page == 1) {
				printStringCenter("Water status 1", 0);
				printString("    Level:", 2, 1);
				printString("Turbidity:", 2, 2);
				printString("Temp:", 2, 3);
			} else if(main_page == 2) {
				printStringCenter("Water status 2", 0);
				printString("PH:", 2, 1);
				printString("EC:", 2, 2);
				printString("   ", 2, 3);
			} else if(main_page == 3) {
				sprintf(buff, "System: %s", SK_FEATURE);
				printStringCenter(buff, 0);
				sprintf(buff, "Version: %s", SK_VERSION);
				printStringCenter(buff, 1);
				sprintf(buff, "Date: %s", SK_DATE);
				printStringCenter(buff, 2);
				sprintf(buff, "Author: %s", SK_AUTHOR);
				printStringCenter(buff, 3);
			}
		}
		if(main_page == 0) {
			printDateTime(datetime);						// Stampo data e ora
			sprintf(buff, "DLMed:%3d%%", calcLuxAverage());
			printString(buff, 0,3);
		} else if(main_page == 1) {
		}
	}
}

//==============================================================================
//									LOOP
//==============================================================================
void loop()	{
	if(deviceMode != DEVICE_MODE_HALT) {
		if(deviceMode == DEVICE_MODE_NORMAL) {
			//-------------------------------------------------------
			//	These tasks have to be executed each loop cycle
			//  only if OTA not in progress
			//-------------------------------------------------------
			if(otaInProgress == false) {
				kp_new = ReadKeyboard();		// read keyboard or IR remote
				tmed = WaterTemperatureHandler();
				datetime = getDateTime();
	
				LightsHandler();
//				AlarmSireneHandle();
				dispatcher();
				handleTimers();

				WaterLevelHandler();
				WaterTurbidityHandler();
				Water_PH_Handler(tmed);
				Water_EC_Handler(tmed);

				handleParametersSave(parameters_save);
			}
			//-------------------------------------------------------
			if(WiFi.status() == WL_CONNECTED) {
				if(otaEnabled) {
					ArduinoOTA.handle();			// IDE OTA update handler
				}
			}
		} else if(deviceMode == DEVICE_MODE_AP) {
			handleRestartTimer();
		}
	}
}

void loopTask1( void * pvParameters ) {
	DEBUG("Task '%s' running on core %d\n", pcTaskGetTaskName(NULL), xPortGetCoreID());
	for(;;) {
		micros(); 									//update overflow
		//=============== TASKS to execute =================
		//==================================================
		vTaskDelay(10);								// fix watchdog issue
	}
	vTaskDelete(NULL);
}

void loopTask2( void * pvParameters ) {
	DEBUG("Task '%s' running on core %d\n", pcTaskGetTaskName(NULL), xPortGetCoreID());
	for(;;) {
		micros(); 									//update overflow
		//=============== TASKS to execute =================
		//==================================================
		vTaskDelay(10);								// fix watchdog issue
	}
	vTaskDelete(NULL);
}

//==============================================================================
//									SETUP
//==============================================================================

bool getNvramSSIDPSW() {
	bool ret = false;
	int readPos = NVRAM_SSID_PSWD;
	char buffer[NVRAM_SSID_PSWD_LEN+1];

	if(nets != NULL) {
		for(int cnt = 0; cnt < NVRAM_SSID_PSWD_NUM; cnt++) {		// clear structs array
			delete(nets[cnt].ssid);
			delete(nets[cnt].password);
		}
		delete(nets);
	}
	nets = new net_type[NVRAM_SSID_PSWD_NUM];
	
	for(int cnt = 0; cnt < NVRAM_SSID_PSWD_NUM; cnt++) {
		//----------------- read SSID
		readStaticMemoryString(buffer, readPos, NVRAM_SSID_PSWD_LEN);
		nets[cnt].ssid = new char[strlen(buffer)+1];
		strcpy(nets[cnt].ssid, buffer);
		if(strlen(buffer) > 0) {
			ret = true;							// at least one ssid was found
		}
		readPos += NVRAM_SSID_PSWD_LEN;

		//----------------- read PASSWORD
		readStaticMemoryString(buffer, readPos, NVRAM_SSID_PSWD_LEN);
		nets[cnt].password = new char[strlen(buffer)+1];
		strcpy(nets[cnt].password, buffer);		// password may be blank (?)
		readPos += NVRAM_SSID_PSWD_LEN;
	}
	return ret;
}

bool getNvramNAME() {
	char buffer[NVRAM_MODULE_NAME_LEN+1];
	if(modulename != NULL) {
		delete(modulename);
	}
	readStaticMemoryString(buffer, NVRAM_MODULE_NAME, NVRAM_MODULE_NAME_LEN);
	if(strlen(buffer) == 0) {
		strcpy(buffer, SK_FEATURE);
	}
	modulename = new char[strlen(buffer)+1];
	strcpy(modulename, buffer);
	return (strlen(modulename) > 0);
}

bool getNvramADMPSW() {
	char buffer[NVRAM_ADMIN_PSWD_LEN+1];

	if(admin_password != NULL) {
		delete(admin_password);
	}
	readStaticMemoryString(buffer, NVRAM_ADMIN_PSWD, NVRAM_ADMIN_PSWD_LEN);
	if(strlen(buffer) < 8) {
		buffer[0] = 0x00;
	}
	admin_password = new char[strlen(buffer)+1];
	strcpy(admin_password, buffer);
	return (strlen(admin_password) > 0);
}

uint8_t getNvramStartingMode() {
	uint8_t result = readStaticMemory(NVRAM_WIFI_STARTING_MODE);
	if(result == 0x00) {
		return STARTING_MODE_NORMAL;
	} else {
		writeStaticMemory(NVRAM_WIFI_STARTING_MODE, 0x00);		// reset to STA mode for the next start
		StaticMemoryCommit();
		return STARTING_MODE_AP;
	}
}

bool startNetwork() {
	bool ret = false;
	int cnt = 20;
	
	for(int cnt = 0; cnt < NVRAM_SSID_PSWD_NUM; cnt++) {
		if(strlen(nets[cnt].ssid) > 0) {
			wifiMulti.addAP(nets[cnt].ssid, nets[cnt].password);
		}
	}
	
	DEBUG("Connecting to Network");   
    while(wifiMulti.run() != WL_CONNECTED and cnt-- > 0) {
		DEBUG(".");
		delay(500);
	}
	if(cnt > 0) {
		moduleIp = WiFi.localIP();
#ifdef DEBUG_MESSAGES
		byte mac[6];
		WiFi.macAddress(mac);
		Serial.printf("\nConnected with IP %d.%d.%d.%d", moduleIp[0], moduleIp[1], moduleIp[2], moduleIp[3]);
		Serial.printf(", MAC %02X-%02X-%02X-%02X-%02X-%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		Serial.printf("Access point ssid '%s', rssi %d, channel %d\n", WiFi.SSID(), WiFi.RSSI(), WiFi.channel());
#endif		
		//-------------------- mDNS RESPONDER -------------------------
		if(MDNS.begin(modulename)) {
			DEBUG("MDNS responder started as '%s.local'\n", modulename);
		}
		deviceMode = DEVICE_MODE_NORMAL;
		ret = true;
	} else {
		DEBUG(" timeout reached\n");
	}
	return ret;
}

bool startAP() {
	bool ret = false;

	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	WiFi.disconnect();
	delay(100);
	ret = WiFi.softAP(modulename);
	if(ret) {
		moduleIp = WiFi.softAPIP();
		DEBUG("Module succesfully switched to AP mode at IP %d.%d.%d.%d\n", moduleIp[0], moduleIp[1], moduleIp[2], moduleIp[3]);
		if(MDNS.begin(modulename)) {
			DEBUG("MDNS responder started as '%s.local'\n", modulename);
		}
		deviceMode = DEVICE_MODE_AP;
		startWebServer();
	} else {
		deviceMode = DEVICE_MODE_HALT;
		DEBUG("Failed to switch to AP mode: system halted\n");
	}
	return ret;
}

bool initSystemTimers() {
	int iValue = readStaticMemory(NVRAM_RESTART_TIMER);
	if(iValue < 10 or iValue > 240) iValue = 120;		// set default to 120 secs
	restartTimerTarget = iValue*1000;

	DEBUG("System timers:  AP reset=%d (secs)\n", iValue);

	restartTimer = millis();
}	

void WireInit() {
	DEBUG("Wire init ");
	if(Wire.begin()) {
		Wire.setClock(WIRE_BUS_CLOCK);
		Wire.setTimeOut(WIRE_BUS_TIMEOUT);		// i2c bus timeout in mS (default is 50mS)
		DEBUG("@ SDA: %d, SCL: %d, frequency: %dHz, timeout: %dmS\n", SDA, SCL, Wire.getClock(), Wire.getTimeOut());
	} else {
		DEBUG("fault\n");
	}
}

void initSPIFFS() {
	if(SPIFFS.begin(true)) {			// Make sure we can read the file system
		DEBUG("SPIFFS successfully mounted: total space %d bytes, used %d bytes\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());
//		listDir(SPIFFS, "/", 0);
	} else {
		DEBUG("Error mounting SPIFFS\n");
	}
}

void initOTA( bool enable = true ) {
	otaInProgress = false;		
	DEBUG("OTA update ");
	
	if(enable) {
		otaEnabled = readStaticMemory(NVRAM_OTA_ENABLED) == 0x01;
	} else {
		otaEnabled = false;
	}
	
	if(otaEnabled) {
		ArduinoOTA.setRebootOnSuccess(true);
		ArduinoOTA.setHostname(modulename);
//		ArduinoOTA.setPort(3232);
		
		ArduinoOTA.onStart([]() {
			KeyboardDisable();					// disable keyboard interrupts
			otaInProgress = true;
			DEBUG("Start updating ");
			if(ArduinoOTA.getCommand() == U_FLASH) {
				DEBUG("fw");
			} else {
				DEBUG("fs");
			}
			DEBUG("...\nProgress\n");
		});
		
		ArduinoOTA.onEnd([]() {
			DEBUG("End of job\n");
			delay(3000);
			ESP.restart();
		});

		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			DEBUG("\r %d %% ", (progress / (total / 100)));
		});
		
		ArduinoOTA.onError([](ota_error_t error) {
			DEBUG("Error: ");
			if (error == OTA_AUTH_ERROR) {
				DEBUG("Auth Failed\n");
			} else if (error == OTA_BEGIN_ERROR) {
				DEBUG("Begin Failed\n");
			} else if (error == OTA_CONNECT_ERROR) {
				DEBUG("Connect Failed\n");
			} else if (error == OTA_RECEIVE_ERROR) {
				DEBUG("Receive Failed\n");
			} else if (error == OTA_END_ERROR) {
				DEBUG("End Failed\n");
			} else {
				DEBUG("Unknown\n");
			}
			delay(3000);
			ESP.restart();
		});
		
		ArduinoOTA.begin();
		DEBUG("enabled\n");
	} else {
		ArduinoOTA.end();
		DEBUG("disabled\n");
	}
}

bool saveNvramSSIDPSW() {
	int writePos = NVRAM_SSID_PSWD;

	for(int cnt = 0; cnt < NVRAM_SSID_PSWD_NUM; cnt++) {
		updateStaticMemoryString(nets[cnt].ssid, writePos, NVRAM_SSID_PSWD_LEN);
		writePos += NVRAM_SSID_PSWD_LEN;
		updateStaticMemoryString(nets[cnt].password, writePos, NVRAM_SSID_PSWD_LEN);
		writePos += NVRAM_SSID_PSWD_LEN;
	}
	StaticMemoryCommit();
}

void saveNvramNAME() {
	writeStaticMemoryString(modulename, NVRAM_MODULE_NAME, NVRAM_MODULE_NAME_LEN);
	StaticMemoryCommit();
	DEBUG("Modified NAME: '%s', len: %d\n", modulename, strlen(modulename));
	MDNS.end();
	MDNS.begin(modulename);
}

void saveNvramSystemTimers() {
	updateStaticMemory(NVRAM_RESTART_TIMER, (uint8_t)(restartTimerTarget/1000));
	StaticMemoryCommit();
}

void saveNvramOTA() {
	bool res = (readStaticMemory(NVRAM_OTA_ENABLED) == OTA_ENABLE);
	if(otaEnabled != res) {
		writeStaticMemory(NVRAM_OTA_ENABLED, otaEnabled?OTA_ENABLE:OTA_DISABLE);
		initOTA(otaEnabled);
	}
	StaticMemoryCommit();
}

void saveRunMode() {
	if(!fastTimeRun) {
		 LoadAllLightStatus();
	}
}

void saveNvramAdminPsw() {
	updateStaticMemoryString(admin_password, NVRAM_ADMIN_PSWD, NVRAM_ADMIN_PSWD_LEN);
	StaticMemoryCommit();
	DEBUG("Admin password ");
	if(strlen(admin_password) == 0) {
		DEBUG("removed\n");
	} else {
		DEBUG("modified: '%s', len: %d\n", admin_password, strlen(admin_password));
	}
}

void saveNvramRestart() {
//	DEBUG("Restart mode bits: 0x%02X - %08sb\n", restart_mode, itoa(restart_mode, buff, 2));
	if((restart_mode & RESTART_AP) == RESTART_AP) {
		writeStaticMemory(NVRAM_WIFI_STARTING_MODE, STARTING_MODE_AP);
		StaticMemoryCommit();
		DEBUG("AP mode set... ");
	}
	if((restart_mode & RESTART_CLEAR) == RESTART_CLEAR) {
		clearNvram();
		DEBUG("NVram cleared... ");
	}
	if((restart_mode & RESTART_SIMPLE) == RESTART_SIMPLE) {
		DEBUG(" Restarting system...\n");
		ESP.restart();
	}
}

void startWebServer() {
	if(deviceMode == DEVICE_MODE_AP) {
		webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/config_ap.html", String(), false, processor); });
	} else {
		webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/index.html", String(), false, processor); });
		webServer.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request) {	request->send(SPIFFS, "/config.html", String(), false, processor); });
	}
	webServer.on("/ssid.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/ssid.html", String(), false, processor); });
	webServer.on("/ssidexec.html", HTTP_POST, handleSsidExec);
	webServer.on("/modulename.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/modulename.html", String(), false, processor); });
	webServer.on("/modulenameexec.html", HTTP_POST, handleModuleNameExec);
	webServer.on("/systemtimers.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/systemtimers.html", String(), false, processor); });
	webServer.on("/systemtimersexec.html", HTTP_POST, handleSystemTimersExec);
	webServer.on("/ota.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/ota.html", String(), false, processor); });
	webServer.on("/otaexec.html", HTTP_POST, handleOtaExec);
	webServer.on("/admin.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/admin.html", String(), false, processor); });
	webServer.on("/adminexec.html", HTTP_POST, handleAdminExec);
	webServer.on("/restart.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/restart.html", String(), false, processor); });
	webServer.on("/restartexec.html", HTTP_POST, handleRestartExec);
	webServer.on("/runmode.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/runmode.html", String(), false, processor); });
	webServer.on("/runmodeexec.html", HTTP_POST, handleRunmodeExec);
	webServer.on("/runmodecontrol", HTTP_GET, handleRunmodeControl);
	webServer.on("/datetime.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/datetime.html", String(), false, processor); });
	webServer.on("/datetimeexec.html", HTTP_POST, handleDatetimeExec);
	webServer.on("/lightsparams.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/lightsparams.html", String(), false, processor); });
	webServer.on("/lightsparamsexec.html", HTTP_POST, handleLightsParametersExec);
	webServer.on("/lightsdispvalues.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/lightsdispvalues.html", String(), false, processor); });
	webServer.on("/temperature.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/temperature.html", String(), false, processor); });
	webServer.on("/temperatureexec.html", HTTP_POST, handleTemperatureExec);

	webServer.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/style.css", "text/css"); });
	webServer.on("/basic_scripts.js", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/basic_scripts.js", "text/javascript"); });
	webServer.on("/ssid_scripts.js", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/ssid_scripts.js", "text/javascript"); });
	webServer.on("/ws_scripts.js", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/ws_scripts.js", "text/javascript"); });
	webServer.on("/blank.jpg", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/blank.jpg", "image/jpg"); });
	webServer.on("/modify.jpg", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/modify.jpg", "image/jpg"); });
	webServer.onNotFound([](AsyncWebServerRequest *request) { request->send(404, "text/plain", "WARNING: you have reached an unknown page"); });

	// setup websocket handler
	webSocket.onEvent(onWsEvent);
	webServer.addHandler(&webSocket);

	// start webserver & websocket server
	webServer.begin();
	DEBUG("HTTP & websocket server started and listening on port %d\n", HTTP_PORT);
}		
	
void setup() {
	#ifdef DEBUG_MESSAGES
		Serial.begin(SERIAL_BAUD);	// serial port #1 used for debugging
		delay(100);					// wait serial port to stabilize
		Serial.printf("\nAquarium Controller (%s) v.%s by %s (%s)\n", SK_FEATURE, SK_VERSION, SK_AUTHOR, SK_DATE);
		Serial.print("Initializing system...\n");
	#endif

	initSPIFFS();						// init SPI File System
	
	WireInit();							// init wire bus
	NvRamInit();						// Initialize Non Volatile RAM
	bSsid = getNvramSSIDPSW();
	bName = getNvramNAME();
	bAdmp = getNvramADMPSW();
	starting_mode = getNvramStartingMode();
	
	DEBUG("Module Name: '%s'\n", modulename);
	DEBUG("PA Admin Password ");
	if(bAdmp) {
		DEBUG("SET\n");
	} else {
		DEBUG("NOT set\n");
	}

	//-------------------------------------------------------------
	//---------------- CONFIG SYSTEM RESOLUTIONS ------------------
	//-------------------------------------------------------------
	analogReadResolution(ADC_RESOLUTION);
	pwm_resolution = pow(2, PWM_BITS)-1; 
	DEBUG("Analog ADC/DAC set to %d bits (resolution %d)\n", ADC_RES_BITS, int(ADC_RESOLUTION));
	DEBUG("PWM set to %d bits, (resolution %d)\n", PWM_BITS, int(pwm_resolution));
			
	//-------------------------------------------------------------
	//------------------- CREATE NEEDED TASKS ---------------------
	//-------------------------------------------------------------
	// PARAMETERS: task function, name, stack size, parameter, priority, handle, core to pin task to
							
//	xTaskCreatePinnedToCore(loopTask1, "Sensors", 10000, NULL, 1, &Task1, CORE_0);
//	delay(500); 

//	xTaskCreatePinnedToCore(loopTask2, "Task2", 10000, NULL, 1, &Task2, CORE_0);
//	delay(500); 
	//-------------------------------------------------------------
	
	if(bSsid and starting_mode == STARTING_MODE_NORMAL) {
		if(startNetwork()) {
			//-------------------------------------------------------------
			//-------------- START HERE ALL NEEDED SYSTEMS ----------------
			//-------------------------------------------------------------
			RtcInit();						// Initialize Real Time Clock
			startWebServer();
			initSystemTimers();
			initOTA();

			DisplayInit();					// Initialize LCD/I2C display
//			AlarmInit();					// Initialize automatic alarm sirene (arduino pro-mini)
			BuzzerInit();					// Initialize Buzzer
			RelaisInit();					// Relais board init
#ifdef IR_KEYBOARD
			KeyboardInit();					// Keyboard and IR receiver init [init before PwmLightsInit()]
#endif
			PwmLightsInit() ;				// PWM Lights init
			TempSensorsInit();				// Temperature sensors initialization
			PH_SensorInit();				// PH sensor initialization
			EC_SensorInit();				// PH sensor initialization
			LevelSensorInit();				// Level sensor initialization
			TurbiditySensorInit();			// Turbidity sensor initialization

			dstatus = DS_IDLE_INIT;			// startup Dispatcher status
			alrmsonoro = true;
			main_page = MAIN_PAGE_FIRST;
			parameters_save = SAVE_NOTHING;

			DEBUG("System v.%s (%s) setup ended\n\n", SK_VERSION, SK_FEATURE);
			return;
		}
	}
	startAP();
	initSystemTimers();
	//-------------------------------------------------------------
}

//===========================================================================================
//									SPIFF HANDLING FUNCTIONS
//===========================================================================================
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root) {
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()) {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file) {
        if(file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

//===========================================================================================
//									WEB-SOCKET HANDLING FUNCTIONS
//===========================================================================================
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
	if(type == WS_EVT_CONNECT) {
		DEBUG("ws[%s][%u] connect\n", server->url(), client->id());
	} else if(type == WS_EVT_DISCONNECT) {
//		DEBUG("ws[%s][%u] disconnect\n", server->url(), client->id());
	} else if(type == WS_EVT_ERROR) {
		DEBUG("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
	} else if(type == WS_EVT_DATA) {
		if(strncmp((char*)data, "ssid request", len) == 0) {
			ssidRequestTrigger = client->id();
		}
	} else if(type == WS_EVT_PONG) {
	}
}

//===========================================================================================
//									HTML PRE-PROCESSOR
//===========================================================================================
String processor(const String& var) {
	char buf[64];

	if(var == "MODULENAME") {
		return String(modulename);
	} else if(var == "MODULETITLE") {
		String ret = String(modulename);
		ret.toUpperCase();
		return ret;
	} else if(var == "ADMIN_PASSWD") {
		return String(admin_password);
	} else if(var == "VERSION") {
		sprintf(buf, "%s v.%s", SK_DATE, SK_VERSION);
		return String(buf);
	} else if(var == "DATE") {
		sprintf(buf, "%02d-%02d-%04d", datetime.tm_mday, datetime.tm_mon, datetime.tm_year);
		return String(buf);
	} else if(var == "DATE_SET") {
		sprintf(buf, "%04d-%02d-%02d", datetime.tm_year, datetime.tm_mon, datetime.tm_mday);
		return String(buf);
	} else if(var == "TIME") {
		sprintf(buf, "%02d:%02d:%02d", datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
		return String(buf);
	} else if(var == "TIME_MIN") {
		uint16_t mins = (datetime.tm_hour*60)+datetime.tm_min;
		uint16_t decmins = mins > MINUTES_PER_DAY ? mins-MINUTES_PER_DAY : mins;
		sprintf(buf, "<font color=blue>%4d</font> <font color=red>[%4d]</font>", decmins, mins);
		return String(buf);
	} else if(var == "DATETIME") {
		sprintf(buf, "%02d-%02d-%04d %02d:%02d:%02d", datetime.tm_mday, datetime.tm_mon, datetime.tm_year, datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
		return String(buf);
	} else if(var == "NETWORK") {
		return WiFi.SSID();
	} else if(var == "PWM_RESOLUTION") {
		return String(pwm_resolution);
	} else if(var == "RUNMODE") {
		return String(fastTimeRun ? "Fast" : "Normal");
	} else if(var == "APRESET") {
		return String(int(restartTimerTarget/1000));
	} else if(var == "OTASWITCH") {
		return String(otaEnabled ? "En" : "Dis") + "abled";
	} else if(var == "DLAVG") {
		sprintf(buf, "%3d%", calcLuxAverage());
		return String(buf);
	} else if(var == "LLAVG") {
		float avg = ((uint32_t)Plafo[LIGHT_LINE_5].pwmValue * pwm_resolution / (uint32_t)Plafo[LIGHT_LINE_5].pwmMax);
		sprintf(buf, "%3d%", int((avg * 100) / pwm_resolution));
		return String(buf);
	} else if(var.substring(0,4) == "RELE") {
		return String(relaisStatus(var.substring(4).toInt())?"ON ":"OFF");
		
	} else if(var.substring(0,4) == "NTP_") {
		return processorNtp(var);
	
	} else if(var.substring(0,6) == "WATER_") {
		return processorWater(var);
	
	} else if(var.substring(0,6) == "LIGHT_") {
		return processorLight(var);
	
	} else if(var.substring(0,6) == "RADIO_") {
		return processorRadio(var);

	} else if(var.substring(0,5) == "SSID_") {
		return processorSsid(var);
	}
	return String("");
}

String processorSsid( String var ) {
	char buf[16];
	var = var.substring(5);
	
	if(var.substring(0,5) == "NAME_") {
		uint8_t index = var.substring(5).toInt();
		return String(nets[index].ssid);
	} else if(var.substring(0,5) == "PASS_") {
		uint8_t index = var.substring(5).toInt();
		return String(nets[index].password);
	}
	return String();
}		

String processorNtp( String var ) {
	char buf[16];
	var = var.substring(4);
	
	if(var == "ENABLE") {
		return String(ntpswitch ? "En" : "Dis") + "abled";
	} else if(var == "SERVER") {
		return String(ntpserver==NULL?"N/A":ntpserver);
	} else if(var == "PERIOD") {
		return String(int(ntpPollTimerTarget/MILLISECS_PER_MIN));
	}
	return String();
}		

String processorWater( String var ) {
	char buf[16];
	var = var.substring(6);

	if(var == "LEVEL") {
		return String(Liquid_level?"FULL":"LOW ");
	} else if(var == "PH") {
		return String(isnan(PHavg) ? NOT_AVAILABLE : ftoa(buf, PHavg));
	} else if(var == "EC") {
		return String(isnan(ECavg) ? NOT_AVAILABLE : ftoa(buf, ECavg));
	} else if(var == "TURB") {
		return String(isnan(ntu) ? NOT_AVAILABLE : ftoa(buf, ntu));
	} else if(var == "TEMP_SET") {
		return String(Tempacqua);
	} else if(var == "TEMP") {
		if(Tsensor1 or Tsensor2) {
			return String(ftoa(buf, tmed));
		} else {
			return String(NOT_AVAILABLE);
		}
	}
	return String();
}
	
String processorLight( String var ) {
	char buf[32];
	var = var.substring(6);

	if(var.substring(0,3) == "ON_") {
		int i = var.substring(3).toInt();
		uint16_t minsOn = Plafo[i].minsOn;
		if(minsOn > MINUTES_PER_DAY) minsOn -= MINUTES_PER_DAY;
		sprintf(buf, "%02d:%02d", getTimeHour(minsOn), getTimeMin(minsOn));
	} else if(var.substring(0,3) == "FA_") {
		int i = var.substring(3).toInt();
		uint16_t minsFA = Plafo[i].minsFA;
		if(minsFA > MINUTES_PER_DAY) minsFA -= MINUTES_PER_DAY;
		sprintf(buf, "%02d:%02d", getTimeHour(minsFA), getTimeMin(minsFA));
	} else if(var.substring(0,3) == "IT_") {
		int i = var.substring(3).toInt();
		uint16_t minsIT = Plafo[i].minsIT;
		if(minsIT > MINUTES_PER_DAY) minsIT -= MINUTES_PER_DAY;
		sprintf(buf, "%02d:%02d", getTimeHour(minsIT), getTimeMin(minsIT));
	} else if(var.substring(0,4) == "OFF_") {
		int i = var.substring(4).toInt();
		uint16_t minsOff = Plafo[i].minsOff;
		if(minsOff > MINUTES_PER_DAY) minsOff -= MINUTES_PER_DAY;
		sprintf(buf, "%02d:%02d", getTimeHour(minsOff), getTimeMin(minsOff));
	} else if(var.substring(0,7) == "MIN_ON_") {
		int i = var.substring(7).toInt();
		uint16_t minsOn = Plafo[i].minsOn;
		sprintf(buf, "<font color=%s>%4d</font>", minsOn > MINUTES_PER_DAY ? "red":"blue", minsOn);
	} else if(var.substring(0,7) == "MIN_FA_") {
		int i = var.substring(7).toInt();
		uint16_t minsFA = Plafo[i].minsFA;
		sprintf(buf, "<font color=%s>%4d</font>", minsFA > MINUTES_PER_DAY ? "red":"blue", minsFA);
	} else if(var.substring(0,7) == "MIN_IT_") {
		int i = var.substring(7).toInt();
		uint16_t minsIT = Plafo[i].minsIT;
		sprintf(buf, "<font color=%s>%4d</font>", minsIT > MINUTES_PER_DAY ? "red":"blue", minsIT);
	} else if(var.substring(0,8) == "MIN_OFF_") {
		int i = var.substring(8).toInt();
		uint16_t minsOff = Plafo[i].minsOff;
		sprintf(buf, "<font color=%s>%4d</font>", minsOff > MINUTES_PER_DAY ? "red":"blue", minsOff);
	} else if(var.substring(0,4) == "FAD_") {
		int i = var.substring(4).toInt();
		uint16_t minsFad = Plafo[i].minsFad;
		if(minsFad > MINUTES_PER_DAY) minsFad -= MINUTES_PER_DAY;
		sprintf(buf, "%02d:%02d", getTimeHour(minsFad), getTimeMin(minsFad));
	} else if(var.substring(0,8) == "MIN_FAD_") {
		int i = var.substring(8).toInt();
		uint16_t minsFad = Plafo[i].minsFad;
		sprintf(buf, "<font color=%s>%4d</font>", minsFad > MINUTES_PER_DAY ? "red":"blue", minsFad);
	} else if(var.substring(0,7) == "STATUS_") {
		PlafoTemp tmp;
		getPlafoAdjustedTimings(&tmp, var.substring(7).toInt());
		if(tmp.minsCurr >= tmp.minsOn and tmp.minsCurr < tmp.minsFA) {				//  - alba
			sprintf(buf,"<font color=orange>Inc</font>");
		} else if(tmp.minsCurr >= tmp.minsFA and tmp.minsCurr < tmp.minsIT) {		//	- luce piena
			sprintf(buf, "<font color=red>Full</font>");
		} else if(tmp.minsCurr >= tmp.minsIT and tmp.minsCurr < tmp.minsOff) {		// 	- tramonto
			sprintf(buf, "<font color=orange>Dec</font>");
		} else {																	//  - buio
			sprintf(buf, "Dark");
		}
	} else if(var.substring(0,5) == "MODE_") {
		int i = var.substring(5).toInt();
		decodePowerState(buf, Plafo[i].workingMode);
	} else if(var.substring(0,4) == "PWM_") {
		int i = var.substring(4).toInt();
		sprintf(buf, "%d", Plafo[i].pwmMax);
	} else if(var.substring(0,5) == "DESC_") {
		int i = var.substring(5).toInt();
		sprintf(buf, "%s", plafoNames[i]);
	}
	return String(buf);
}	

String processorRadio( String var ) {
	char buf[32];
	var = var.substring(6);
	if(var.substring(0,5) == "MODE_") {
		uint8_t mode = var.substring(5,6).toInt();
		uint8_t index = var.substring(7).toInt();
		sprintf(buf, "'%d'%s", mode, Plafo[index].workingMode==mode?" checked":"");
	} else if(var.substring(0,4) == "NTP_") {
		uint8_t mode = var.substring(4).toInt();
		sprintf(buf, "'%s'%s", mode==0?"off":"on", ntpswitch==mode?" checked":"");
	} else if(var.substring(0,4) == "OTA_") {
		uint8_t mode = var.substring(4).toInt();
		sprintf(buf, "'%s'%s", mode==0?"off":"on", otaEnabled==mode?" checked":"");
	} else if(var.substring(0,4) == "RUN_") {
		uint8_t mode = var.substring(4).toInt();
		sprintf(buf, "'%s'%s", mode==0?"normal":"fast", fastTimeRun==mode?" checked":"");
	}		
	return String(buf);
}
	
//===========================================================================================
//									HTML HANDLING FUNCTIONS
//===========================================================================================

void handleSsid( AsyncWebServerRequest *request ) {
	String buff = initHtmlPage("SSID/Password set");

	resetRestartTimer();
	
	int result = WiFi.scanNetworks();
	buff += "Networks found " + String(result) + ":";
	
	for(int i = 0; i < result; i++) {
		buff += "<br>" + String(i+1) + ") " + WiFi.SSID(i);
		buff += ", rssi: " + String(WiFi.RSSI(i));
		buff += ", enc: " + String(decodeEncryption(WiFi.encryptionType(i)));
		buff += ", chan: " + String(WiFi.channel(i));
	}
	buff += "<br>" + initForm("ssid");
	
	for(int i = 0; i < NVRAM_SSID_PSWD_NUM; i++) {
		String idx = String(i);
		buff += "<br><label>SSID" + idx + ": </label><input name='ssid" + idx + "' length=32 value='" + String(nets[i].ssid) + "'>";
		buff += " <label>PSWD: </label><input name='pass" + idx + "' length=32 value='" + String(nets[i].password) + "'>";
	}
	buff += getP(form_end_ok);
	buff += generateApHomeLink();
	sendHtmlPage(request, buff);
}

void handleSsidExec( AsyncWebServerRequest *request ) {
	uint8_t len;
	String val;
	
	for(int cnt = 0; cnt < NVRAM_SSID_PSWD_NUM; cnt++) {
		if(request->hasParam("ssid"+String(cnt), true)) {
			delete(nets[cnt].ssid);
			val = request->getParam("ssid"+String(cnt), true)->value();
			len = val.length()+1;
			nets[cnt].ssid = new char[len];
			val.toCharArray(nets[cnt].ssid, len);
		}
		if(request->hasParam("pass"+String(cnt), true)) {
			delete(nets[cnt].password);
			val = request->getParam("pass"+String(cnt), true)->value();
			len = val.length()+1;
			nets[cnt].password = new char[len];
			val.toCharArray(nets[cnt].password, len);
		}
	}
	parameters_save = SAVE_SSID_PSW;

	if(deviceMode == DEVICE_MODE_AP) {
		printRefreshHeader(request);
	} else {
		printRefreshHeader(request, "/config.html", 3);
	}
}

void handleModuleNameExec( AsyncWebServerRequest *request ) {
	int i;
	if(request->hasParam("name", true)) {
		String name = request->getParam("name", true)->value();
		uint8_t len = name.length();
		if(len++ > 0) {
			delete(modulename);
			modulename = new char[len];
			name.toCharArray(modulename, len);
			parameters_save = SAVE_MODULENAME;
		}
	}
	if(deviceMode == DEVICE_MODE_AP) {
		printRefreshHeader(request);
	} else {
		printRefreshHeader(request, "/config.html");
	}
}

void handleSystemTimersExec( AsyncWebServerRequest *request ) {
	uint8_t iTimer = (uint8_t)request->arg("restimer").toInt();
	if(iTimer < 10 or iTimer > 240) iTimer = 120;	// range is 10-240, default is 2 mins (1 byte)
	restartTimerTarget = iTimer*1000;
	parameters_save = SAVE_TIMERS;

	if(deviceMode == DEVICE_MODE_AP) {
		printRefreshHeader(request);
	} else {
		printRefreshHeader(request, "/config.html");
	}
}

void handleLightsParametersExec( AsyncWebServerRequest *request ) {
	uint8_t workingMode;
	uint16_t minsOn, minsOff, minsFad, minsFA, minsIT, pwmMax;

	for(int i = 0; i < LIGHT_PWM_CHANNELS; i++) {
		String value;
		if(request->hasParam("houron"+String(i), true)) {
			value = request->getParam("houron"+String(i), true)->value();
			minsOn = value.substring(0,2).toInt()*60 + value.substring(3,5).toInt();
		}
		if(request->hasParam("houroff"+String(i), true)) {
			value = request->getParam("houroff"+String(i), true)->value();
			minsOff = value.substring(0,2).toInt()*60 + value.substring(3,5).toInt();
		}
		if(request->hasParam("hourfad"+String(i), true)) {
			value = request->getParam("hourfad"+String(i), true)->value();
			minsFad = value.substring(0,2).toInt()*60 + value.substring(3,5).toInt();
		}
		if(request->hasParam("pwmmax"+String(i), true)) {
			pwmMax = request->getParam("pwmmax"+String(i), true)->value().toInt();
		}
		if(request->hasParam("type"+String(i), true)) {
			workingMode = (uint8_t)request->getParam("type"+String(i), true)->value().toInt();
		}
		minsFA = minsOn + minsFad;								//  Calcolo ora e minuti di fine alba (FA)
		minsIT = minsOff - minsFad;								//  Calcolo ora e minuti di inizio tramonto (IT)
		
		if(minsFA < minsOn) minsFA += MINUTES_PER_DAY;			// adjust day overflow
		if(minsIT < minsFA) minsIT += MINUTES_PER_DAY;
		if(minsOff < minsIT) minsOff += MINUTES_PER_DAY;

		if(minsOn >= 0 and minsOn <= MINUTES_PER_2DAYS) {
			Plafo[i].minsOn = minsOn;
		}
		if(minsOff >= 0 and minsOff <= MINUTES_PER_2DAYS) {
			Plafo[i].minsOff = minsOff;
		}
		if(minsFA >= 0 and minsFA <= MINUTES_PER_2DAYS) {
			Plafo[i].minsFA = minsFA;
		}
		if(minsIT >= 0 and minsIT <= MINUTES_PER_2DAYS) {
			Plafo[i].minsIT = minsIT;
		}
		if(pwmMax >= 0 and pwmMax <= pwm_resolution) {
			Plafo[i].pwmMax = pwmMax;
		}
		if(workingMode == POWER_OFF or workingMode == POWER_ON or workingMode == POWER_AUTO) {
			Plafo[i].workingMode = workingMode;
		}
//		DEBUG("SET LIGHTS ===> minsOn: %d, minsOff: %d, minsFad: %d, minsFA: %d, minsIT: %d, pwmMax: %d, workingMode: %d\n",Plafo[i].minsOn,Plafo[i].minsOff,Plafo[i].minsFad,Plafo[i].minsFA,Plafo[i].minsIT,Plafo[i].pwmMax,Plafo[i].workingMode);
	}			
	parameters_save = SAVE_LIGHTS;
	printRefreshHeader(request, "/config.html");
}

void handleDatetimeExec( AsyncWebServerRequest *request ) {
	bool nvcommit = false;

	if(request->hasParam("ntpswitch", true)) {
		bool ntpsw = (request->getParam("ntpswitch", true)->value()=="on");
		if(ntpswitch != ntpsw) {
			ntpswitch = ntpsw;			/// ?????? quando a questa viene assegnato false
			nvcommit = true;
		}
	}
	if(request->hasParam("ntp_srv", true)) {
		String newntp = request->getParam("ntp_srv", true)->value();
		uint8_t len = newntp.length();
		if(len++ > 0) {
			if(newntp != String(ntpserver)) {
				delete(ntpserver);
				ntpserver = new char[len];
				newntp.toCharArray(ntpserver, len);
				nvcommit = true;
			}
		}
	}		
	if(request->hasParam("ntppoll", true)) {
		int poll = request->getParam("ntppoll", true)->value().toInt();
		if(poll > 0 and poll != int(ntpPollTimerTarget / MILLISECS_PER_MIN)) {
			ntpPollTimerTarget = poll * MILLISECS_PER_MIN;
			nvcommit = true;
		}
	}
	if(request->hasParam("date", true) and request->hasParam("time", true)) {
		struct tm dtimp = datetime;
		
		String date = request->getParam("date", true)->value();
		String time = request->getParam("time", true)->value();
		
		dtimp.tm_year = date.substring(0,4).toInt();
		dtimp.tm_mon = date.substring(5,7).toInt();
		dtimp.tm_mday = date.substring(8).toInt();
		dtimp.tm_hour = time.substring(0,2).toInt();
		dtimp.tm_min = time.substring(3,5).toInt();
		dtimp.tm_sec = time.substring(6).toInt();
		setRtcTime(dtimp);
//		DEBUG("===> datetime: %02d-%02d-%04d %02d:%02d:%02d\n", dtimp.tm_mday, dtimp.tm_mon, dtimp.tm_year, dtimp.tm_hour, dtimp.tm_min, dtimp.tm_sec);
	}		
	if(nvcommit) {
		parameters_save = SAVE_DATETIME;
	}
	printRefreshHeader(request, "/config.html");
}

void handleTemperatureExec( AsyncWebServerRequest *request ) {
	if(request->hasParam("temperature", true)) {
		Tempacqua = request->getParam("temperature", true)->value().toFloat();
	}
	parameters_save = SAVE_TEMP;
	printRefreshHeader(request, "/config.html");
}

void handleOtaExec( AsyncWebServerRequest *request ) {
	if(request->hasParam("ota", true)) {
		otaEnabled = (request->getParam("ota", true)->value() == "on");
		parameters_save = SAVE_OTA;
	}
	printRefreshHeader(request, "/config.html");
}

void handleAdminExec( AsyncWebServerRequest *request ) {
	if(request->hasParam("action", true)) {
		if(request->getParam("action", true)->value() == "Remove") {
			delete(admin_password);
			admin_password = new char[1];
			admin_password[0] = 0x00;
			parameters_save = SAVE_ADMIN;
		} else if(request->getParam("action", true)->value() == "OK") {
			String pass = request->getParam("password", true)->value();
			if(pass != String(admin_password)) {
				uint8_t len = pass.length();
				if(len++ >= 8) {
					delete(admin_password);
					admin_password = new char[len];
					pass.toCharArray(admin_password, len);
					parameters_save = SAVE_ADMIN;
				}
			}
		}
	}
	printRefreshHeader(request, "/config.html");
}

void handleRestartExec( AsyncWebServerRequest *request ) {
	restart_mode = NO_RESTART;

	if(request->hasParam("clear", true)) {
		if(request->getParam("clear", true)->value() == "Clear") {
			restart_mode = restart_mode | RESTART_CLEAR;
		}
	}
	if(request->hasParam("apmode", true)) {
		if(request->getParam("apmode", true)->value() == "Force") {
			restart_mode = restart_mode | RESTART_AP;
		}
	}
	if(request->hasParam("restart", true)) {
		if(request->getParam("restart", true)->value() == "Restart") {
			restart_mode = restart_mode | RESTART_SIMPLE;
			parameters_save = SAVE_RESTART;
		}
	}

	if(parameters_save == SAVE_RESTART) {
		printRefreshHeader(request, "/", 1, "Restarting...");
	} else {
		printRefreshHeader(request, "/config.html");
	}
}

void handleRunmodeExec( AsyncWebServerRequest *request ) {
	if(request->hasParam("runmode", true)) {
		bool fastmode = (request->getParam("runmode", true)->value() == "fast");
		if(fastmode != fastTimeRun) {
			fastTimeRun = fastmode;
			parameters_save = SAVE_RUNMODE;
		}
	}
	printRefreshHeader(request, "/runmode.html");
}

void handleRunmodeControl( AsyncWebServerRequest *request ) {
	if(fastTimeRun) {
		if(request->hasParam("control", false)) {
			String control = request->getParam("control", false)->value();
			if(control == "Inc") {
				getDateTime( false, +1 );
			} else if(control == "Dec") {
				getDateTime( false, -1 );
			}
		}
	}
	request->send(200, "text/plain", "OK");	
}

//------------------------------ HEADERS and FOOTERS ---------------------------------

String initLink( String link, String title ) {
	return "<br><a href='/" + link + "'>" + title + "</a>";
}
	
String initForm( String title ) {
	return getP(form_init1) + title + "exec.html" + getP(form_init2);
}
	
String initHtmlPage( String title ) {
	String name = String(modulename);
	name.toUpperCase();
	return getP(html_init) + getP(color_blue) + name + getP(color_rest) + " - " + title + "</h1>";
}

String generateApHomeLink() {
	return deviceMode==DEVICE_MODE_AP?getP(home_link):getP(config_link);
}

void sendHtmlPage( AsyncWebServerRequest *request, String buff ) {
	buff += getP(html_exit);
	request->send(200, "text/html", buff);
}

void printRefreshHeader(AsyncWebServerRequest *request ) {
	printRefreshHeader( request, "/", 0, "" );
}

void printRefreshHeader( AsyncWebServerRequest *request, int time ) {
	printRefreshHeader( request, "/", time, "" );
}

void printRefreshHeader( AsyncWebServerRequest *request, String url ) {
	printRefreshHeader( request, url, 0, "" );
}
	
void printRefreshHeader( AsyncWebServerRequest *request, String url, int time ) {
	printRefreshHeader( request, url, time, "" );
}
	
void printRefreshHeader( AsyncWebServerRequest *request, String url, int time, String message ) {
	String buff;
	String sTime = String(time);

	buff += "<!doctype html><head><META HTTP-EQUIV='REFRESH' CONTENT='" + sTime + "; URL=";
	if(url.length() == 0) {
		buff += "/";
	} else {
		buff += url;
	}
	buff += "'></head>";
	if(message.length() != 0) {
		buff += "<body><h1>" + message + "</h1></body>";
	}	
	buff += "</html>";
	request->send(200, "text/html", buff);
}

//===================================== END OF HTML_HANDLERS===================================
