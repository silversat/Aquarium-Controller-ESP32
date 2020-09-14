//
//	NVRam.h
//

//#define USE_EXT_EEPROM_LIBRARY

#if defined(USE_RTC_NVRAM)
	#define BUFFER_LENGTH 		128						// eeprom/nvram buffer length
	#define NVRAM_CAPACITY		0x7FFF					// 32k type
	#if defined(USE_EXT_EEPROM_LIBRARY)
		#include <extEEPROM.h>
		#define EEPROM	extEEPROM
	#else
		#include <Eeprom24C32_64.h>
		#define EEPROM	Eeprom24C32_64
	#endif
	EEPROM *nvram;
	uint16_t nvramI2Caddr;
#else
	#include <EEPROM.h>
#endif

uint8_t readStaticMemory( uint16_t address ) {
	uint8_t ret = false;
	#ifdef USE_RTC_NVRAM
		ret = nvram->read(address);
	#else
		ret = EEPROM.read(address);
	#endif	
	return ret;
}	

void writeStaticMemory( uint16_t address, uint8_t data ) {
	#ifdef USE_RTC_NVRAM
		nvram->write(address, data);
		delay(10);
	#else
		EEPROM.write(address, data);
	#endif
}

void updateStaticMemory( uint16_t address, uint8_t data ) {
	if(data != readStaticMemory(address)) {
		writeStaticMemory(address, data);
	}
}

int readStaticMemoryInt( uint16_t address ) {
	uint8_t high = readStaticMemory(address);
	uint8_t low = readStaticMemory(address+1);
	return word(high,low);
}

bool writeStaticMemoryInt( uint16_t address, int value ) {
	bool ret = false;
	if(value != readStaticMemoryInt(address)) {
		writeStaticMemory(address, highByte(value));
		writeStaticMemory(address+1, lowByte(value));
		ret = true;
	}
	return ret;
}

void updateStaticMemoryInt( uint16_t address, int value ) {
	if(value != readStaticMemoryInt(address)) {
		updateStaticMemory(address, highByte(value));
		updateStaticMemory(address+1, lowByte(value));
	}
}

char* readStaticMemoryString( char* buffer, int readPos, int maxChars ) {	
	int i = 0;
	char c;
	buffer[0] = 0x00;
	while(i < maxChars) {
		c = (char)readStaticMemory(readPos + i);
		if(c < 0xFF) {
			buffer[i] = c;
			if(c == '\0') {
				break;
			}
		} else if(c == 0xFF) {
			buffer[i] = '\0';
			break;
		}
		i++;
	}
	return buffer;
}	

bool writeStaticMemoryString( char *str, int writePos, int maxChars ) {
	int i;
	for(i = 0; i < strlen(str); i++) {
		if(i < maxChars - 1) {
			writeStaticMemory(writePos + i, str[i]);
		}
	}
	writeStaticMemory(writePos + i, '\0');
	return true;
}

bool writeStaticMemoryString( String str, int writePos, int maxChars ) {
	int i;
	for(i = 0; i < str.length(); i++) {
		if(i < maxChars - 1) {
			writeStaticMemory(writePos + i, str.charAt(i));
		}
	}
	writeStaticMemory(writePos + i, '\0');
	return true;
}

bool updateStaticMemoryString( char *str, int writePos, int maxChars ) {
	char buff[strlen(str)+1];
	bool ret = false;
	
	readStaticMemoryString(buff, writePos, maxChars);
	if(strcmp(buff, str) != 0) {
		writeStaticMemoryString(str, writePos, maxChars);
		ret = true;
	}
	return ret;
}

template <class T> int NvramWriteAnything(int ee, const T& value) {
	const byte* p = (const byte*)(const void*)&value;
	int i;
	for(i = 0; i < sizeof(value); i++) {
		writeStaticMemory(ee++, *p++);
	}
	return i;
}

template <class T> int NvramReadAnything(int ee, T& value) {
	byte* p = (byte*)(void*)&value;
	int i;
	for(i = 0; i < sizeof(value); i++) {
		*p++ = readStaticMemory(ee++);
	}
	return i;
}

void StaticMemoryCommit() {
	#ifdef USE_RTC_NVRAM
//		delay(10);
	#else
		EEPROM.commit();
	#endif
}
	
void clearNvram() {
	for(int i = 0; i < NVRAM_SIZE; i++) {
		writeStaticMemory(i, 0xFF);
		yield();
	}
	StaticMemoryCommit();
}

void NvRamInit() {
	#ifdef USE_RTC_NVRAM
		int addr = checkI2CdeviceAddress(0x50, 0x57);
		if(addr > 0) {
			nvramI2Caddr = (uint16_t)addr;
			
			nvram = new EEPROM(nvramI2Caddr);
			nvram->initialize(false, true, false);				// initialize( begin, wiretimeout, log )
			
			randomSeed(analogRead(0));
			uint8_t randNumber = (uint8_t)random(0x100);		// random number from 0 to 255
			DEBUG("24c32 NV Ram chip @ 0x%02X ", nvramI2Caddr);
			writeStaticMemory(NVRAM_CAPACITY, randNumber);
			delay(20);
			if(readStaticMemory(NVRAM_CAPACITY) == randNumber) {
				DEBUG("OK\n");
			} else {
				DEBUG("FAILED\n");
			}
		} else {
			DEBUG("NVram device not present on I2C bus\n");
		}
	#else
		EEPROM.begin(NVRAM_SIZE);
		DEBUG("onboard EEPROM OK\n");
	#endif	
}
