//
//	LCD_I2C.h
//
#define	DISPLAY_I2C_ADDR	0x27
#define	DISPLAY_MAX_ROWS	4
#define	DISPLAY_MAX_COLS	20

//#include <LiquidCrystal_I2C.h>
#include <PCF8574_HD44780_I2C.h>

//LiquidCrystal_I2C display(DISPLAY_I2C_ADDR, DISPLAY_MAX_COLS, DISPLAY_MAX_ROWS);
PCF8574_HD44780_I2C display(DISPLAY_I2C_ADDR, DISPLAY_MAX_COLS, DISPLAY_MAX_ROWS);		// Address 0x27, 16 chars, 4 line display

byte 	linea, colonna;

void setCursor( byte col=0xFF, byte row=0xFF ) {
	if(col < 0xFF and row < 0xFF) {
		display.setCursor(col, row);
	}
}
	
void displayClear() {
	display.clear();
}

void displayClearRow( uint8_t row ) {
	display.setCursor(0, row);						// clear last display row
	display.print(spaces(20));
}

void printCommit() {}
	
void printString( const char* msg, byte col=0xFF, byte row=0xFF, byte commit=true ) {
	setCursor(col, row);
	display.print(msg);
}
	
void printStringCenter( const char* msg, byte row=0xFF, byte commit=true ) {
	if(row == 0xFF) row = DISPLAY_MAX_ROWS-1;		// if omitted ROW, print at last available row
	displayClearRow(row);
	setCursor((DISPLAY_MAX_COLS-strlen(msg))/2, row);
	display.print(msg);
}
	
void printBlinkingString( const char* msg, byte row=0xFF ) {
	static float blinkTimer;
	static bool status;
	if(blinkTimer + BLINK_TIMER < millis()) {
		if(status) {
			printStringCenter(msg, row);
		} else {
			displayClearRow(row);
		}
		status = !status;
		blinkTimer = millis();
	}
//	if(blinkTimer > millis()) blinkTimer = millis();
}

void printChar( char chr, byte col=0xFF, byte row=0xFF ) {
	setCursor(col, row);
	display.print(chr);
}
	
void printSpaces( int val, byte col=0xFF, byte row=0xFF, byte commit=true ) {
	setCursor(col, row);
	display.print(spaces(val));
}
	
void printNumber( float number, byte col=0xFF, byte row=0xFF ) {
	setCursor(col, row);
	display.print(number);
}
	
void stampafrecce( byte col=0xFF, byte row=0xFF, byte before=0, byte after=0, byte num=2 ) {	// Creata per semplificare il codice dell'impostazione di data e ora stampa semplicemente due frecce consecutive
	setCursor(col, row);
	if(before > 0) {								// if any spaces to print before arrows
		printSpaces(before);						// ptint them
	}
	for(int i = 0; i < num; i++) {	
		printChar('^');								// print arrows
	}
	if(after > 0) {									// if any spaces to print after arrows
		printSpaces(after);							// print them
	}
}

void DisplayInit() {
//	display.begin();							// inizializzazione del display  (LiquidCrystal_I2C library)
	display.init(false);						// inizializzazione del display  (PCF8574_HD44780_I2C library) without wire.begin()
	display.clear();
	display.backlight();
	DEBUG("Lcd display OK\n");
}