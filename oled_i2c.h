//
//	OLED_I2C.h
//
#include "configuration.h"

#if defined(OLED_32)				// OLED display height, in pixels
	#define SCREEN_HEIGHT 	32 
	#define OLED_ADDR		OLED_32
#elif defined(OLED_64)
	#define SCREEN_HEIGHT	64
	#define OLED_ADDR		OLED_64
#elif defined(OLED_128)
	#define SCREEN_HEIGHT	128
	#define OLED_ADDR		OLED_128 
#else	
	#message DISPLAY RESOLUTION ERROR
#endif

#define SCREEN_WIDTH 		128 	// OLED display width, in pixels
#define	OLED_PIX_CHAR_ROW	8		// pixels per row (8)
#define	OLED_PIX_CHAR_COL	6.4		// pixels per column (6)
#define	DISPLAY_MAX_ROWS	SCREEN_HEIGHT / OLED_PIX_CHAR_ROW
#define	DISPLAY_MAX_COLS	SCREEN_WIDTH / OLED_PIX_CHAR_COL

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);
byte 	linea, colonna;

void setCursor( byte col=0xFF, byte row=0xFF ) {
	if(col < 0xFF and row < 0xFF) {
		display.setCursor(col*OLED_PIX_CHAR_COL, row*OLED_PIX_CHAR_ROW);
	}
}
	
void stampafrecce( byte col=0xFF, byte row=0xFF, byte before=0, byte after=0, byte num=2 ) {	// Creata per semplificare il codice dell'impostazione di data e ora stampa semplicemente due frecce consecutive
	setCursor(col, row);
	if(before > 0) {								// if any spaces to print before arrows
		display.print(spaces(before));				// ptint them
	}
	for(int i = 0; i < num; i++) {
		display.print('^');							// ptint them
	}
	if(after > 0) {									// if any spaces to print after arrows
		display.print(spaces(after));				// print them
	}
	display.display();
}

void printSpaces( int value, byte col=0xFF, byte row=0xFF, byte commit=true ) {
	setCursor(col, row);
	display.print(spaces(value));
	if(commit) display.display();
}
	
void displayClear() {
	display.clearDisplay();
	display.display();
}

void displayClearRow( uint8_t row ) {
	if(row == 0xFF) row = DISPLAY_MAX_ROWS-1;		// if omitted ROW, print at last available row
	printSpaces(DISPLAY_MAX_COLS, 0, row);
}

void printCommit() {
	display.display();
}
	
void printString( const char* msg, byte col=0xFF, byte row=0xFF, byte commit=true ) {
	setCursor(col, row);
	display.print(msg);
	if(commit) display.display();
}
	
void printStringCenter( const char* msg, byte row=0xFF, byte commit=true ) {
	if(row == 0xFF) row = DISPLAY_MAX_ROWS-1;		// if omitted ROW, print at last available row
	displayClearRow(row);
	setCursor((DISPLAY_MAX_COLS-strlen(msg))/2, row);
	display.print(msg);
	if(commit) display.display();
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
}

void printChar( const char chr, byte col=0xFF, byte row=0xFF ) {
	setCursor(col, row);
	display.print(chr);
	display.display();
}
	
void printNumber( float number, byte col=0xFF, byte row=0xFF ) {
	setCursor(col, row);
	display.print(number);
	display.display();
}
	
void DisplayInit() {
	display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);		// inizializzazione del display  (0x3C, 0x3D, 0x78, 0x7A)
	display.display();
	display.setTextSize(1);     						// Normal 1:1 pixel scale
	display.setTextColor(WHITE, BLACK);					// Draw white text
	display.setCursor(0, 0);    						// Start at top-left corner
	display.cp437(true);								// Use full 256 char 'Code Page 437' font
	DEBUG("Oled display (%dx%d) OK\n", SCREEN_WIDTH, SCREEN_HEIGHT);
}