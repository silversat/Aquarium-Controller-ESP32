# Aquarium-Controller-ESP32

<br>		This controller has been developed onto an ESP32 board.
<br>		The hardware includes:
<br>		- 4/8 relais serial/parallel board (hobbycomponents.com)
<br>		- I2C 20x4 LCD display or i2c OLED 32x128, 64x128, 128x128
<br>		- uses esp32 internal RTC/nvram and/or DS3231/DS1307 external RTC
<br>		- one or two DS18b20 temperature sensor
<br>		- IR receiver (TK19) associated with an apple tv remote control
<br>		- a buzzer
<br>		- parameters saved on internal flash or external 24c32 i2c nvram
<br>		- lights ar driven in PWM (default 5 channels, 4 for daylights and 1 for night lunar light)
<br>  		  by project Main Well pwm led driver modules are used (LDD-350H/LDD-700H/LDD-1000H) but any is ok.
<br>		- Web interface for config/view parameters with websockets dynamic updates (alternative to IR remote)
<br>
<br>		Most of the hardware is configurable (configuration.h)
<br>
<br>		Compile with 	Board = ESP32 Dev Module
<br>						Partition scheme = Default 4MB with spiffs (1.2MB App/1.5MB SPIFFS)
