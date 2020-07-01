# Aquarium-Controller-ESP32

//		This controller has been developed onto an ESP32 board. 
//		The hardware includes:
//		- 4/8 relais serial/parallel board (hobbycomponents.com)
//		- I2C 20x4 LCD display or i2c OLED 32x128, 64x128, 128x128
//		- uses esp32 internal RTC/nvram and/or DS3231/DS1307 external RTC
//		- one or two DS18b20 temperature sensor
//		- IR receiver (TK19) associated with an apple tv remote control
//		- a buzzer
//		- parameters saved on internal flash or external 24c32 i2c nvram
//		- lights ar driven in PWM (default 5 channels, 4 for daylights and 1 for night lunar light)
//		  by project Main Well pwm led driver modules are used (LDD-350H/LDD-700H/LDD-1000H) but anyy is ok.
//		- Web interface for config/view parameters with websockets dynamic updates (alternative to IR remote)
//
//		Most of the hardware is configurable (configuration.h)
//
//		Compile with 	Board = ESP32 Dev Module
//						Partition scheme = Default 4MB with spiffs (1.2MB App/1.5MB SPIFFS)
//
//
