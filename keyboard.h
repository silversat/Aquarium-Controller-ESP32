//
//	KEYBOARD.h
//
//	based on the Ken Shrriff IR library

#define IR_TIMER_USE_ESP32
#include <IRremote.h>		// have to be ESP32-IRremote library

#define IR_DEBOUNCE_TIME	350			// mS

//#define IR_REMOTE_CODE	0x77E1FF7F	// remote mask code (my apple)
//#define IR_REMOTE_CODE	0x00007000	// remote mask code (all apple)

#define IR_REMOTE_CODE		0x0000FF00	// remote mask code (all apple)
#define IR_PAIRING			0x40		// apple remote pairing code
#define IR_NONE				0x00
#define IR_REPEAT			0xFF
#define IR_UP				0x50 	//	up
#define IR_DOWN				0x30 	//	dn
#define IR_LEFT				0x90 	//	left
#define IR_RIGHT			0x60 	//	right
#define IR_MENU				0xC0 	//	menu
#define IR_OK				0x3A 	//	ok + 0x77E1A07F dub code
#define IR_PLAY				0xFA 	//	play/pause + 0x77E1A07F dub code
#define IR_DUMMY			0xA0 	//	dummy code (in link with OK and PLAY)

IRrecv irrecv(PIN_IR_RECEIVER);
decode_results results;
float debounce_timer;
bool irKeyboardEnabled = false;


void PairingHandler() {
	DEBUG(", Pairing remote...\n");
}

uint16_t ReadKeyboard() {
	static uint16_t 	kp_old;			// last keypressed container
	uint16_t retkey = IR_NONE;
	
	if(irrecv.decode(&results)) {
		if((debounce_timer + IR_DEBOUNCE_TIME) < millis()) {
			retkey = results.value;
//			DEBUG("Remote code %02X", retkey);		
			
			retkey = retkey & IR_REMOTE_CODE;
			retkey = retkey >> 8;

//			DEBUG(", decoded to %02X, ", retkey);
			
			switch(retkey) {
				case IR_PAIRING:
					PairingHandler();
					break;
						
				case IR_REPEAT:
//					DEBUG(" *\n");
					retkey = kp_old;
					break;
					
				case IR_OK:
//					DEBUG("OK\n");
					break;
					
				case IR_UP:
//					DEBUG("UP\n");
					break;
					
				case IR_DOWN:
//					DEBUG("DOWN\n");
					break;
					
				case IR_LEFT:
//					DEBUG("LEFT\n");
					break;
					
				case IR_RIGHT:
//					DEBUG("RIGHT\n");
					break;
					
				case IR_MENU:
//					DEBUG("MENU/ESC\n");
					break;
					
				case IR_PLAY:
//					DEBUG("PLAY\n");
					break;
					
				case IR_DUMMY:				// do nothing (discard)
//					DEBUG("dummy\n");
					retkey = IR_NONE;
					break;
					
				default:					// do nothing (discard)
//					DEBUG("unknown %02X\n", retkey);
					retkey = IR_NONE;
					break;
			}
			kp_old = retkey;
			debounce_timer = millis();
		}
		irrecv.resume(); 				// Receive the next value
	}
	return retkey;
}

void KeyboardEnable() {
	irrecv.enableIRIn();
}

void KeyboardDisable() {
	irrecv.disableIRIn();
}

void KeyboardInit() {
	KeyboardEnable(); 				// Start the IR receiver;
	irKeyboardEnabled = true;
	debounce_timer = millis();
	DEBUG("Keyboard/IR OK\n");
}	
