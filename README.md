# Aquarium-Controller-ESP32 v.3.9.14

<br>		This controller has been developed onto an ESP32 board.
<br>		The hardware includes:
<br>		- 4/8 relais serial/parallel board (hobbycomponents.com)
<br>		- I2C 20x4 LCD display or i2c OLED 32x128, 64x128, 128x128
<br>		- uses esp32 internal RTC/nvram and/or DS3231/DS1307 external RTC
<br>		- one or two DS18b20 water temperature sensor
<br>    - water PH, EC, LEVEL, TURBIDITY sensors handled
<br>		- IR receiver (TK19) associated with an apple tv remote control
<br>		- a buzzer
<br>		- parameters saved on internal flash or external 24c32 i2c nvram
<br>		- lights ar driven in PWM (default 5 channels, 4 for daylights and 1 for night lunar light)
<br>  		by project Main Well pwm led driver modules are used (LDD-350H/LDD-700H/LDD-1000H) but any is ok.
<br>		- Web interface for config/view parameters with websockets dynamic updates (alternative to IR remote)
<br>
<br>		Most of the hardware is configurable (configuration.h)
<br>
<br>		Compile with 	Board = ESP32 Dev Module
<br>						Partition scheme = Default 4MB with spiffs (1.2MB App/1.5MB SPIFFS)
<br>						ESP32-16Mb => Board = SparkFun ESP32 Thing Plus
<br>						Partition scheme = Default 6.5MB APP/OTA /  with 3.43 SPIFFS)
<br>
<br>		CHECKLIST:	1 - Boards.txt
<br>      					2 - variants/esp32thing_plus (or whatever) check pin 21 as SDA and 22 as SCL
<br>
<br><br> v.3.9.14
<br> - corrected issue that made mDNS and OTA not working together
<br>
