//
//	BUS I2C ADDRESSING
//
#define	I2C_ADDR_RELAIS		0x20
#include "PCF8574.h"

PCF8574 schrele(I2C_ADDR_RELAIS);

void relais( byte relnum, boolean status ) {
	schrele.digitalWrite(relnum, status); 
}

boolean	relaisStatus( byte relnum ) {
	return schrele.digitalRead(relnum); 
}

void RelaisInit() {				// inizializzazione del PCF che comanda la scheda relais
//	schrele.begin();
	for(uint8_t i = 0; i < 8; i++) {
		schrele.pinMode(i, OUTPUT);
		schrele.digitalWrite(i, LOW);
	}
	DEBUG("I2C Relais OK\n");
}
