//
//	KEYBOARD.h
//
//	based on the Ken Shrriff IR library
//
//	APPLE remote uses NEC ir codes
//	for pairing: MENU+right for 6secs (0x1502) 
//	for unpairing MENU+left for 6secs (0x1504)

#define IRMP_PROTOCOL_NAMES 		1	// Enable protocol number mapping to protocol strings - requires some FLASH.
#define IRMP_SUPPORT_NEC_PROTOCOL	1	// includes APPLE and ONKYO protocols
#define IRMP_INPUT_PIN   			32	// D32
#define IRSND_OUTPUT_PIN  			0	// none
#define tone(a,b) void()				// no tone() available on ESP32
#define noTone(a) void()

//#define IR_REMOTE_CODE	0x77E1FF7F	// remote mask code (my apple)
//#define IR_REMOTE_CODE	0x00007000	// remote mask code (all apple)

#define IR_REMOTE_CODE		0x0000FF00	// remote mask code (all apple)
#define IR_PAIRING			0x1502		// apple remote pairing code
#define IR_NONE				0x00
#define IR_REPEAT			0xFF
#define IR_UP				0x0A		//	up
#define IR_DOWN				0x0C 		//	dn
#define IR_LEFT				0x09 		//	left
#define IR_RIGHT			0x06 		//	right
#define IR_MENU				0x03 		//	menu
#define IR_OK				0x5C 		//	ok + 0x77E1A07F dub code
#define IR_PLAY				0x5F 		//	play/pause + 0x77E1A07F dub code
#define IR_DUMMY			0x05 		//	dummy code (in link with OK and PLAY)

#ifdef IR_REMOTE_KEYBOARD
#include <irmp.hpp>

IRMP_DATA keyread;

void PairingHandler() {
	DEBUG(", Pairing remote...\n");
}

uint16_t ReadKeyboard( bool autorepeat = false ) {
	static uint16_t 	kp_old;			// last keypressed container
	uint16_t retkey = IR_NONE;

    if(irmp_get_data(&keyread)) {
//		DEBUG("===> IR addr: 0x%02X, cmd: 0x%02X, flags: 0x%02X\n", keyread.address, keyread.command, keyread.flags);
		if(!autorepeat and !(keyread.flags & IRMP_FLAG_REPETITION)) {
			retkey = keyread.command;
			switch(retkey) {
				case IR_PAIRING:
					PairingHandler();
					break;
						
				case IR_OK:
					break;
					
				case IR_UP:
					break;
					
				case IR_DOWN:
					break;
					
				case IR_LEFT:
					break;
					
				case IR_RIGHT:
					break;
					
				case IR_MENU:
					break;
					
				case IR_PLAY:
					break;
					
				case IR_DUMMY:				// do nothing (discard)
					retkey = IR_NONE;
					break;
					
				default:					// do nothing (discard)
					retkey = IR_NONE;
					break;
			}
		}
	}
	return retkey;
}

void KeyboardInit() {
	irmp_init();
	DEBUG("Keyboard/IR OK\n");
}	

#endif