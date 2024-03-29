#include "time.h"

int century_register;

uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
uint32_t year;

char* format;
uint16_t formatLen;

int get_update_in_progress_flag() {
	port_byte_out(cmos_address, 0x0A);
	return (port_byte_in(cmos_data) & 0x80);
}

uint8_t get_RTC_register(int reg) {
	port_byte_out(cmos_address, reg);
	return port_byte_in(cmos_data);
}

void read_rtc() {
	uint8_t century;
	uint8_t last_second;
	uint8_t last_minute;
	uint8_t last_hour;
	uint8_t last_day;
	uint8_t last_month;
	uint8_t last_year;
	uint8_t last_century;
	uint8_t registerB;
	while (get_update_in_progress_flag());
	second = get_RTC_register(0x00);
	minute = get_RTC_register(0x02);
	hour = get_RTC_register(0x04);
	day = get_RTC_register(0x07);
	month = get_RTC_register(0x08);
	year = get_RTC_register(0x09);
	century = 0;
	if (century_register != 0) {
		century = get_RTC_register(century_register);
	}
	do {
		last_second = second;
		last_minute = minute;
		last_hour = hour;
		last_day = day;
		last_month = month;
		last_year = year;
		last_century = century;
		while (get_update_in_progress_flag());
		second = get_RTC_register(0x00);
		minute = get_RTC_register(0x02);
		hour = get_RTC_register(0x04);
		day = get_RTC_register(0x07);
		month = get_RTC_register(0x08);
		year = get_RTC_register(0x09);
		if (century_register != 0) {
			century = get_RTC_register(century_register);
		}
	} while ((last_second != second) || (last_minute != minute) ||
		(last_hour != hour) || (last_day != day) ||
		(last_month != month) || (last_year != year) ||
		(last_century != century_register));
	registerB = get_RTC_register(0x0B);
	if (!(registerB & 0x04)) {
		second = (second & 0x0F) + ((second / 16) * 10);
		minute = (minute & 0x0F) + ((minute / 16) * 10);
		hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
		day = (day & 0x0F) + ((day / 16) * 10);
		month = (month & 0x0F) + ((month / 16) * 10);
		year = (year & 0x0F) + ((year / 16) * 10);

		if (century_register != 0) {
			century = (century & 0x0F) + ((century / 16) * 10);
		}
	}
	if (!(registerB & 0x02) && (hour & 0x80)) {
		hour = ((hour & 0x7F) + 12) % 24;
	}
	if (century_register != 0) {
		year += century * 100;
	}
	else {
		year += (CURRENT_YEAR / 100) * 100;
		if (year < CURRENT_YEAR) year += 100;
	}
}

void formatTime() {
	read_rtc();
	char months[12][10] = { "January ", "February ", "March ", "April ", "May ", "June ", "July ", "August ", "September ", "October ", "November ", "December" };

	format = (char*)malloc(sizeof(char));
	int timeOfDay = 0; // 0: AM, 1: PM
	int formatMinute = 0; // 0: No format, 1: format
	char* timebuf = (char*)malloc(sizeof(char));

	if (hour > 12) {
		hour -= 12;
		timeOfDay = 1;
	}
	else {
		timeOfDay = 0;
	}

	if (minute < 10) {
		formatMinute = 1;
	}
	else {
		formatMinute = 0;
	}

	itoa(hour, timebuf);
	strcat(format, timebuf);
	strcat(format, ":");

	itoa(minute, timebuf);

	if (formatMinute == 1) {
		strcat(format, "0");
		strcat(format, timebuf);
	}
	else {
		strcat(format, timebuf);
	}
	
	if (timeOfDay == 0) {
		strcat(format, "AM");
	}
	else {
		strcat(format, "PM");
	}

	strcat(format, " on ");

	strcat(format, months[month - 1]);
	itoa(day, timebuf);
	strcat(format, timebuf);

	strcat(format, ", ");
	itoa(year, timebuf);
	strcat(format, timebuf);
}