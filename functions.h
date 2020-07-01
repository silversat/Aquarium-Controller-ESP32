
int checkI2CdeviceAddress( uint8_t beg_addr, uint8_t end_addr=NULL ) {
	uint8_t ret = -1;
	// This function uses the return value of
	// the Write.endTransmisstion to see if
	// a device did acknowledge to the address.
	if(end_addr == NULL) end_addr = beg_addr;
	for(uint8_t addr = beg_addr; addr <= end_addr; addr++) {
		Wire.beginTransmission(addr);
		if(Wire.endTransmission() == 0) {
			ret = addr;
			break;
		}
		delay(10);
	}
	return ret;
}

float calcRingBufAverage( float ringBuf[], int samples ) {
	float ret = 0;
	int samp_count = 0;
	for(int idx = 0; idx < samples; idx++) {
		if(ringBuf[idx] > 0) {
			samp_count++;
			ret += ringBuf[idx];
		}
	}
	if(samp_count > 0) {
		return (ret / samp_count);
	} else {
		return NAN;
	}
}

char* ftoa(char *a, double f) {
	long unit = (long)f;
	long decimal = abs((long)((f - unit) * 100));
	sprintf(a, "%2lu.%02lu", unit, decimal);
	return a;
}

boolean isNumeric( char value ) {
	if(value >= 0x30 and value <= 0x39)
		return true;
	else
		return false;
}

String spaces( byte num ) {
	String ret = "";
	for(int i = 0; i < num; i++) {
		ret += " ";
	}
	return ret;
}

void SetInitBit() {
	bitSet(dstatus, DS_INIT_BIT);
}
	
void SetInitBit( byte status ) {
	dstatus = status;
	bitSet(dstatus, DS_INIT_BIT);
}
	
void ClearInitBit() {
	bitClear(dstatus, DS_INIT_BIT);
}
	
boolean CheckInitBit( boolean clearflag ) {		// chech init bit. if clearflag set, clear it too.
	boolean ret = bitRead(dstatus, DS_INIT_BIT);
	if(clearflag and ret)
	bitClear(dstatus, DS_INIT_BIT);
		
	return ret;
}

void printDeviceFound( int device, DeviceAddress addr ) {
	DEBUG("Temperature device %d found at address: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n", device, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]); 
}

void printDeviceNotFound( int device ) {
	DEBUG("Temperature device %d not found\n", device); 
}

byte decToBcd(byte val) { 	// Da decimale a binario
	return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val) {	// Da binario a decimale
	return ( (val/16*10) + (val%16) );
}

void ScrollHandler( uint16_t &value, uint16_t Min, uint16_t Max, int increment ) {  // controllo tutte le variabili che devono ciclare in incremento o decremento
	if(increment > 0) { 
		if(value < Max) {
			value += increment;
		} else {
			value = Min; 
		}
	} else if(increment < 0) {
		if(value > Min) {
			value += increment;
		} else {
			value = Max;
		}
	}
}

void ScrollHandler( uint8_t &value, uint8_t Min, uint8_t Max, int increment ) {	// 8 bit data version
	if(increment > 0) { 
		if(value < Max) {
			value += increment;
		} else {
			value = Min; 
		}
	} else if(increment < 0) {
		if(value > Min) {
			value += increment;
		} else {
			value = Max;
		}
	}
}

char* toUppercase( char* input ) {
	int i = 0;
	while(input[i]) {
		input[i] = toupper(input[i]);
		i++;
	}
	return input;
}

char* decodeStatus(int status) {
	switch(status) {
		case WL_IDLE_STATUS:
			return "IDLE";
			break;
		case WL_NO_SSID_AVAIL:
			return "NO SSID AVAIL";
			break;
		case WL_SCAN_COMPLETED:
			return "SCAN COMPLETED";
			break;
		case WL_CONNECTED:
			return "CONNECTED";
			break;
		case WL_CONNECT_FAILED:
			return "CONNECT FAILED";
			break;
		case WL_CONNECTION_LOST:
			return "CONNECTION LOST";
			break;
		case WL_DISCONNECTED:
			return "DISCONNECTED";
			break;
		case WL_NO_SHIELD:
			return "[no shield detected]";
			break;
		default:
			return "[unknown]";
	}
}

char* decodeEncryption(int enc) {
	switch(enc) {
		case 0:
			return "open";
			break;
		case 1:
			return "WEP";
			break;
		case 2:
			return "WPA_PSK";
			break;
		case 3:
			return "WPA2_PSK";
			break;
		case 4:
			return "WPA_WPA2_PSK";
			break;
		case 5:
			return "WPA2_ENTERPRISE";
			break;
		case 7:
			return "[none]";
			break;
		case 8:
			return "[auto]";
			break;
		default:
			return "[unknown]";
	}
}

String IPAddress2char( IPAddress ip ) {
	return String(ip[0])+"."+String(ip[1])+"."+String(ip[2])+"."+String(ip[3]);
}

char* IPAddress2char( char* buffer, IPAddress ip ) {
	char iBuf[8];
	strcpy(buffer, itoa(ip[0], iBuf, 10));
	strcat(buffer, ".");
	strcat(buffer, itoa(ip[1], iBuf, 10));
	strcat(buffer, ".");
	strcat(buffer, itoa(ip[2], iBuf, 10));
	strcat(buffer, ".");
	strcat(buffer, itoa(ip[3], iBuf, 10));

	return buffer;
}

