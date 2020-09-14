
// declare a static string
#ifdef __AVR__
	#define P(name)   static const unsigned char name[] __attribute__(( section(".progmem." #name) ))
#else
	#define P(name)   static const unsigned char name[]
#endif

// static strings handling functions
char* getP( char *buffer, const unsigned char *str ) {
	int i = 0;
	while(uint8_t value = pgm_read_byte(str++)) {
		buffer[i++] = value;
	}
	buffer[i] = 0;
	return buffer;
}

String getP(const unsigned char *str) {
	String ret = "";
	while(uint8_t value = pgm_read_byte(str++)) {
		ret += char(value);
	}
	return ret;
}

//================================== STRINGS STORAGE ===================================

P(ntp_default)	=	"pool.ntp.org";

P(brk)			=	"<br>";

P(html_init)	=	"<!doctype html><body><h1>";
P(html_exit)	=	"</body></html>";

P(menu_home)	=	"<h2>Current values:</h2>";
P(value_date)	=	"<br>Date: ";
P(value_time)	=	"<br>Time: ";
P(value_temp)	=	"<br>Temperature average: ";
P(value_dlight)	=	"<br>Daylight average: ";
P(value_nlight)	=	"<br>Lunar light: ";
P(value_ph)		=	"<br>Water PH: ";
P(value_ec)		=	"<br>Water EC: ";
P(value_level)	=	"<br>Water Level: ";
P(value_turb)	=	"<br>Water Turbidity: ";
P(item_config)	=	"Configuration";

P(menu_config)	=	"<h2>Current parameters:</h2>Firmware version: ";
P(menu_ssid)	=	"SSID / Password";
P(menu_name)	=	"Module name";
P(menu_timer)	=	"System Timers";
P(menu_ota)		=	"OTA switch";
P(menu_psw)		=	"AP Admin Password";
P(menu_lights)	=	"Lights Parameters";
P(menu_time)	=	"Date & Time";
P(menu_temp)	=	"Temperature";
P(menu_reset)	=	"Module Restart";
P(menu_runmode)	=	"Run Mode";
P(menu_ldval)	=	"Current Lights Values";

P(btn_radio)	=	" <input type='radio' name='type";
P(btn_chk)		=	" checked";
P(btn_disabled)	=	" disabled";

P(curr_link)	=	"<br><br><a href=''>Refresh</a>";
P(refresh_link)	=	"<br><br><a href='/'>Refresh</a>";
P(home_link)	=	"<br><br><a href='/'>Home</a>";
P(config_link)	=	"<br><a href='/config.html'>Back</a>";

P(form_init1)	=	"<form method='post' action='/";
P(form_init2)	=	"'>";
P(form_end)		=	"</form>";
P(form_end_ok)	=	"<br><br><input type='submit' value='OK'></form>";

P(label_init)	=	"<label>";
P(label_end)	=	"</label>";

P(req_ok)		=	">> OK when ready <<";
P(req_refresh)	=	">> RESTART when ready <<";

P(color_red)	=	"<font color=red><b>";
P(color_blue)	=	"<font color=blue><b>";
P(color_rest)	=	"</b></font>";
P(color_full)	=	"<font color=red>Full</font>";
P(color_dec)	=	"<font color=orange>Dec</font>";
P(color_inc)	=	"<font color=orange>Inc</font>";
P(color_dark)	=	"Dark";

P(cal_ph_ph1)	=	"Put PH probe into the 4.0 or 7.0 buffer solution";
P(cal_ph_ph2)	=	"Buffer solution";
P(cal_ph_ph3)	=	"PH calibration ended... data saved";
P(cal_ec_ph1)	=	"Put EC probe into 1413us or 12.88ms/cm buffer solution";
P(cal_ec_ph2)	=	"";
P(cal_ec_ph3)	=	"EC calibration ended... data saved";
