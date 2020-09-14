//
//	Display configuration
//

#define	SERIAL_BAUD					115200	// seriale monitor port

//============================ IR REMOTE CONFIGURATION
#define IR_REMOTE_KEYBOARD		// ATTENTION !!! problems when using IR receiver and internal ESP32 simulated EEPROM (issue to be resolved)

//============================ DISPLAY CONFIGURATION
//#define OLED_32	0x3C					// if defined, 128x32 oled display is used, otherwise LCD 20x4
//#define OLED_64	0x3C					// if defined, 128x64 oled display is used, otherwise LCD 20x4
//#define OLED_128	0x78					// if defined, 128x128 oled display is used, otherwise LCD 20x4

//============================ RTC NVRAM CONFIGURATION
//#define USE_RTC_DS3231
#define USE_RTC_DS1307
#define USE_RTC_NVRAM						// use 24c32 nvram on RTC board or EEPROM (if not defined)

//============================ WIRE BUS DEFINITIONS
#define WIRE_BUS_CLOCK				100000	// 100.000 - 400.000 Hz
#define WIRE_BUS_TIMEOUT			50		// default 50mS

//============================ 
#define RELE_PARALLEL						// if defined, use a parallel rele board, otherwise use a serial i2c rele board (PCF8574)

#define	HTTP_PORT					80
#define PWM_FREQUENCY				5000
#define PWM_BITS					10
#define ADC_RESOLUTION				4095.0
#define ADC_RES_BITS				12

#define	BLINK_TIMER					500		// blinking intervam mS
#define	MINUTES_PER_DAY				1440
#define	MINUTES_PER_2DAYS			MINUTES_PER_DAY*2
#define	SECONDS_PER_DAY				86400
#define	WS_POLL_TIMER				1000
#define	MILLISECS_PER_MIN			60000
#define	NOT_AVAILABLE				" N/A "
#define STARTING_MODE_NORMAL		0
#define STARTING_MODE_AP			0xFF
#define NETWORK_START_RETRIES		3		// after these retries, starts in AP mode.
#define DEVICE_MODE_NORMAL			1
#define DEVICE_MODE_AP				2
#define DEVICE_MODE_HALT			9

#define OTA_ENABLE					0x01
#define OTA_DISABLE					0xFF

#define NO_RESTART					0x00	// 0000 0000
#define RESTART_SIMPLE				0x10	// 0001 0000
#define RESTART_AP					0x20	// 0010 0000
#define RESTART_CLEAR				0x0A	// 0000 1010

//
//	Arduino PIN configuration
//
#define HELTEC_ONBOARD_LED		25

#define LIGHT_PWM_CHANNELS		5		// number of PWM light channels
#define PIN_LIGHT_PWM_1			4		// PWM pin for led line 1
#define PIN_LIGHT_PWM_2			16		// PWM pin for led line 2
#define PIN_LIGHT_PWM_3			17		// PWM pin for led line 3
#define PIN_LIGHT_PWM_4			18		// PWM pin for led line 4
#define PIN_LIGHT_PWM_5			19		// PWM pin for led line 5

#define PIN_RELAIS_1			12		// WARNING: you must configure 'relaisPin' array in 'scheda_rele.h' according with these defines
#define PIN_RELAIS_2			14		//		and SR_RELAIS_NUM into the relais board section below. these are for the parallel relais board
#define PIN_RELAIS_3			27		
#define PIN_RELAIS_4			26
#define PIN_RELAIS_5			
#define PIN_RELAIS_6			
#define PIN_RELAIS_7			
#define PIN_RELAIS_8			

#define PIN_ONE_WIRE_BUS		33
#define PIN_IR_RECEIVER			32
#define PIN_TURBIDITY_SENSOR	35
#define PIN_LEVEL_SENSOR		34
#define PIN_PH_SENSOR			39
#define PIN_EC_SENSOR			38
#define PIN_BUZZER				37

//
// relais board
//
#define	RL_ON					HIGH		// for inverted logic (RL_ON = LOW) change here
#define	RL_OFF					LOW			//		and here
#define	SR_RELAIS_NUM			0x04
#define	SR_WATER_HEATER			0x01		// water heater = relais #1
#define	SR_WATER_LEVEL			0x02		// water level = relais #2
//#define	SR_TURBIDITY			0x03		// turbidity relais #3
#define	SR_PH					0x04		// PH alarm relais #4
#define	SR_EC					0x05		// EC alarm relais #5
#define	SR_CO2_VALVE			0x06		// CO2 valve
#define	SR_LUNAR_LIGHT			0x07		// lunar light = relais #7
#define	SR_WATER_VALVE_DRAIN	0x08		// drain water valve
#define	SR_WATER_VALVE_LOAD		0x09		// load water valve
//
// turbidity (0-3000 NTU (1 NTU = 1 mg/Liter)
//
#define	TURBIDITY_MIN			0.5			// min turbidity (hysteresis)
#define	TURBIDITY_MAX			10			// max turbidity (hysteresis)
//
// temperature
//
#define	TEMP_SENSOR_RESOLUTION	10			// temperature sensor resolution in bit (9-12)
#define	TEMP_RANGE				2.5			// max delta-Temp allowed before alarm occurs
#define	TEMP_READ_INTERVAL		3000		// temperature time lapse (mSecs)
#define	PH_READ_INTERVAL		1000		// PH time lapse (mSecs)
#define	EC_READ_INTERVAL		1000		// PH time lapse (mSecs)
#define	NTU_READ_INTERVAL		1000		// NTU time lapse (mSecs)
#define	TEMP_ALLOWED_MIN		10			// min allowed temperature when setting
#define	TEMP_ALLOWED_MAX		40			// max allowed temperature when setting
#define	TEMP_ALLOWED_DEFAULT	28			// default temperature when <min or >max in setting
#define TEMP_SAMPLES_NUM		10			// ring buffer temperature samples number
//
//	LIGHTS
//
#define	LIGHT_DARK				0
#define	LIGHT_INC				1
#define	LIGHT_DEC				2
#define	LIGHT_FULL				3

#define	FADING_INTERVAL			100			// mSecs
#define	POWER_OFF				0			// Power OFF / mode OFF
#define	POWER_ON				1			// Power ON / mode ON
#define	POWER_AUTO				2			// Power mode AUTO
#define	POWER_ON_INC			3			// Power ON/increasing
#define	POWER_ON_DEC			4			// Power ON/decreasing
#define	LIGHT_LINE_NUMBER		LIGHT_PWM_CHANNELS			// total number of light lines
#define	DAYLIGHT_LINE_NUMBER	4			// total number of daylight lines (no moon light)
#define	LIGHT_LINE_1			0
#define	LIGHT_LINE_2			1
#define	LIGHT_LINE_3			2
#define	LIGHT_LINE_4			3
#define	LIGHT_LINE_5			4
#define	LIGHT_LINE_6			5
//
//	LIGHTS SETUP FASES
//
#define LS_ON_HOUR				1
#define LS_ON_MIN				2
#define LS_OFF_HOUR				3
#define LS_OFF_MIN				4
#define LS_PAGE1_CONFIRM		5
#define LS_FAD_HOUR				6
#define LS_FAD_MIN				7
#define LS_PAGE2_CONFIRM		8

//	SAVE PARAMETERS CODES
#define SAVE_NOTHING			0
#define SAVE_MODULENAME			1
#define SAVE_TIMERS				2
#define SAVE_DATETIME			3
#define SAVE_RUNMODE			4
#define SAVE_RESTART			5
#define SAVE_ADMIN				6
#define SAVE_OTA				7
#define SAVE_TEMP				8
#define SAVE_LIGHTS				9
#define SAVE_SSID_PSW			10
#define SAVE_CALIBRATION		11

//
//	EEPROM/NVRAM data position
//
//	PH calibration data from 0x00 to 0x09
//	PH calibration data from 0x0A to 0x13
//	PLAFONIERE: 6 * 12 bytes each = 72 starting by NVRAM_START_ADDR
//
#define	NVRAM_PH_NEUTRAL			0x00	// address where PH neutral voltage is stored (4 bytes)
#define	NVRAM_PH_ACID				0x04	// address where PH acid voltage is stored (4 bytes)
#define	NVRAM_EC_KVALUEHIGH			0x08	// address where EC kvalue high is stored (4 bytes)
#define	NVRAM_EC_KVALUELOW			0x0C	// address where EC kvalue low is stored (4 bytes)
#define	NVRAM_TEMP_ADDR				0x14	// address where TEMPERATURE is stored (2 bytes)

#define	NVRAM_START_ADDR			0x20
#define	PLAFO_MODE					0		// 0=OFF, 1=ON, 2=AUTO
#define	PLAFO_MAX_FADING			1		// Max fading value set
#define	PLAFO_MINS_ON				3		// NEW VERSION DATA: 2bytes each for INT saves
#define	PLAFO_MINS_OFF				5
#define	PLAFO_MINS_FA				7
#define	PLAFO_MINS_IT				9

#define NVRAM_SSID_PSWD				0x400	// 1024 is start nvram position
#define NVRAM_SSID_PSWD_NUM			3		// 32+32*3 = 192 bytes
#define NVRAM_SSID_PSWD_LEN			32		// usr1 1024-1055, psw1 1056-1087, 
											// usr2 1088-1119, psw2 1120-1151,
											// usr3 1152-1183, psw3 1184-1215 ...

#define NVRAM_UDP_IP				0x500	// (1280 dec) 4 bytes				// 1280-1283
#define NVRAM_UDP_PORT				0x504	// 2 bytes							// 1284-1285
#define NVRAM_WIFI_STARTING_MODE	0x506	// 0x00=normal, ... , 0xFF=AP		// 1286
#define NVRAM_OTA_ENABLED			0x507	// OTA enabled switch				// 1287
#define	NVRAM_RESTART_TIMER			0x508	// 1 byte restart timer in secs		// 1288
#define	NVRAM_NTP_ENABLED			0x509	// 1 byte switch ntp				// 1289
#define	NVRAM_NTP_POLL_MINS			0x50A	// 2 bytes ntp poll period (mins)	// 1290-1291

#define NVRAM_MODULE_NAME			0x510	// 16 bytes, modeule name			// 1296-1311
#define NVRAM_MODULE_NAME_LEN		16
#define NVRAM_ADMIN_PSWD			0x520	// 32 bytes admin password (len 32)	// 1312-1343
#define NVRAM_ADMIN_PSWD_LEN		32
#define NVRAM_NTP_SERVER			0x540	// 32 bytes, modeule name			// 1344-1367
#define NVRAM_NTP_SERVER_LEN		32

#define	NVRAM_SIZE					0x600

#define	PLAFO_NAME_1			"Warm white"
#define	PLAFO_NAME_2			"Natural white"
#define	PLAFO_NAME_3			"Cold white"
#define	PLAFO_NAME_4			"Red"
#define	PLAFO_NAME_5			"Blue"
#define	PLAFO_NAME_6			""
//
//	Action defines
//
#define	ACT_INC	+1		// increment
#define	ACT_DEC	-1		// decrement
//
//	DISPATCHER STATUS
//		The first nibble is the menu item. if 0, no menu displayed, normal run.
//		The second nibble (3 LSB) is the current menu item status indicator. MSB is the init flag
//			0x00	0000 0 000	running
//			0x19	0001 1 001	setup menu, voice 1, init function call
//			0x10	0001 0 000	setup menu, voice 1, already initialized
//			0x28	0010 1 000	setup menu, voice 2, init function call
//			0x21	0010 0 001	setup menu, voice 2, voice 1, already initialized
//
#define DS_INIT_BIT					0x03		// third bit is the init bit
#define DS_INIT_FLAG				0x08		// value of the init bit set (2^init-bit => 2^3=8)
#define	DS_INIT_MASK				0xF7		// clear Init bit mask
#define	DS_IDLE						0x00		//	Normal run mode
#define	DS_IDLE_INIT				0x08
#define	DS_SETUP					0x10		//	Setup mode (menu)
#define	DS_SETUP_DATETIME			0x11		//	Setup mode, datetime submenu
#define	DS_SETUP_LIGHTS				0x20					//	Setup mode, lights submenu 1 (base for Plafo array)
#define	DS_SETUP_LIGHTS_0			DS_SETUP_LIGHTS+1		//	Setup mode, lights Funz/LMax
#define	DS_SETUP_LIGHTS_1			DS_SETUP_LIGHTS+2		//	Setup mode, lights line 1
#define	DS_SETUP_LIGHTS_2			DS_SETUP_LIGHTS+3		//	Setup mode, lights line 2
#define	DS_SETUP_LIGHTS_3			DS_SETUP_LIGHTS+4		//	Setup mode, lights line 3
#define	DS_SETUP_LIGHTS_4			DS_SETUP_LIGHTS+5		//	Setup mode, lights line 4
#define	DS_SETUP_LIGHTS_5			DS_SETUP_LIGHTS+6		//	Setup mode, lights line 5
#define	DS_SETUP_LIGHTS_6			DS_SETUP_LIGHTS+7		//	Setup mode, lights line 6
#define	DS_SETUP_TEMP				0x31					//	Setup mode, temperature submenu
#define	DS_SETUP_INFOLIGHTS			0x41					//	Setup mode, info luci submenu
#define	DS_SETUP_CALIBRATION		0x51					//	Setup mode, sensors calibration
#define	DS_SETUP_CALIBRATION_PH		DS_SETUP_CALIBRATION+1	//	Setup mode, PH sensors calibration
#define	DS_SETUP_CALIBRATION_EC		DS_SETUP_CALIBRATION+2	//	Setup mode, EC sensors calibration
#define	DS_SETUP_TIMERUN			0x61					//	Setup mode, test time run
#define	DS_SETUP_SPARE_3			0x71					//	Setup mode, spare item
#define	DS_SETUP_SPARE_4			0x81					//	Setup mode, spare item
#define	DS_SETUP_SPARE_5			0x91					//	Setup mode, spare item
