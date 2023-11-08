//
//	BUS I2C ADDRESSING
//
#define	I2C_ADDR_RELAIS		0x38
#include "PCF8574.h"

PCF8574 schrele(I2C_ADDR_RELAIS);

void relais( uint8_t relnum, bool status ) {
	uint8_t idx = (relnum-1);
	if(schrele.digitalRead(idx) != status) {
		schrele.digitalWrite(idx, status); 
//		DEBUG("rele %d set to %s\n", relnum, status?"On":"Off"); 
	}
}

bool relaisStatus( uint8_t relnum ) {
	uint8_t idx = (relnum-1);
	return schrele.digitalRead(idx); 
}

void RelaisInit() {				// inizializzazione del PCF che comanda la scheda relais
	for(uint8_t i = 0; i < SR_RELAIS_NUM; i++) {
		schrele.pinMode(i, OUTPUT);
		schrele.digitalWrite(i, LOW);
	}
//	schrele.begin();				// best NOT to begin, WIRE init included!!!
	DEBUG("I2C Relais OK\n");
}
