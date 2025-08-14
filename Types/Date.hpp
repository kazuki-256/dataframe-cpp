#ifndef _DF_DATE_HPP_
#define _DF_DATE_HPP_

#include <time.h>
#include <string.h>
#include <stdio.h>




// parse month
size_t dfParseMonth(const char* strmonth, size_t n, int* month) {
    static const char* MONTHS[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

    for (int i = 0; i < 12; i++) {
        if (strncasecmp(strmonth, MONTHS[i], n) == 0) {
            *month = i + 1;
            return strlen(MONTHS[i]);
        }
    }
    return 0;
}

// parse weekday
size_t dfParseWeekday(const char* strweek, size_t n, int* week) {
    static const char* WEEKDAYS[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

    for (int i = 0; i < 7; i++) {
        if (strncasecmp(strweek, WEEKDAYS[i], n) == 0) {
            *week = i + 1;
            return strlen(WEEKDAYS[i]);
        }
    }
    return 0;
}

// self strptime for parsing time in win32
// ! be causeful since this method will not complete all values
size_t dfParseTime(const char* strdate, const char* fmt, struct tm* tm) {
    const char* original = strdate;
    int symbol, c;
    char buffer[10];
    int temp;

    // == reset tm values ==
    tm->tm_hour = 0;

    // == start parse ==

    while (symbol = *(fmt++)) {
        // == filter ==
        // symbol is normal char: clear anything not suit to fmt
        if (symbol != '%') {
    filter:
            while ((c = *(strdate++)) && c != symbol);

            if (c == 0) {
                break;
            }
            continue;
        }

        // == parse ==
        // if have known size, move strdate pointer and continue
        // if unknown size, continue to move by filter (!no savity)
        symbol = *(fmt++);

        switch (symbol) {
            // == date parsing ==
            case '%':
                goto filter;
            case 'Y':
                sscanf(strdate, "%04d", &tm->tm_year);
                tm->tm_year -= 1900;
                strdate += 4;
                continue;
            case 'y':
                sscanf(strdate, "%02d", &tm->tm_year);
                tm->tm_year += 100; // 2000 - 1900
                strdate += 2;
                continue;
            case 'm':
                sscanf(strdate, "%02d", &tm->tm_mon);
                tm->tm_mon -= 1;
                strdate += 2;
                continue;
            case 'B':
                strdate += dfParseMonth(strdate, 12, &tm->tm_mon);
                continue;
            case 'b':
                dfParseMonth(strdate, 3, &tm->tm_mon);
                strdate += 3;
                continue;
            case 'd':
                sscanf(strdate, "%02d", &tm->tm_mday);
                strdate += 2;
                continue;
            case 'e':
                strncpy(buffer, strdate, 2);
                sscanf(buffer, "%d", &tm->tm_mday);
                strdate += 2;
                continue;
            case 'j':
                sscanf(strdate, "%03d", &tm->tm_yday);
                strdate += 3;
                continue;
            case 'A':
                strdate += dfParseWeekday(strdate, 12, &tm->tm_wday);
                continue;
            case 'a':
                dfParseWeekday(strdate, 3, &tm->tm_wday);
                strdate += 3;
                continue;
            case 'u':
                sscanf(strdate, "%01d", &tm->tm_wday);
                strdate += 1;
                continue;
            case 'w':
                sscanf(strdate, "%01d", &tm->tm_wday);
                tm->tm_wday = 7 - tm->tm_wday;
                strdate += 1;
                continue;
            case 'U':
            case 'W':
                continue;   // week number (unusable)
            
            // == time parsing ==
            case 'H':
                sscanf(strdate, "%02d", &tm->tm_hour);
                strdate += 2;
                continue;
            case 'l':
                sscanf(strdate, "%02d", &temp);
                tm->tm_hour = (tm->tm_hour + temp) % 24;
                continue;
            case 'p':
                if (strncasecmp(strdate, "PM", 2) == 0) {
                    tm->tm_hour = (tm->tm_hour + 12) % 24;
                }
                continue;
            case 'M':
                sscanf(strdate, "%02d", &tm->tm_min);
                strdate += 2;
                continue;
            case 'S':
                sscanf(strdate, "%02d", &tm->tm_sec);
                strdate += 2;
                continue;
            case 'f':
                continue;   // micro seconds (unusable)
            case 'Z':
                // symbol of time zone
                if (strncasecmp(strdate, "GMT", 3) == 0) {
                    tm->tm_zone = "GMT";
                    strdate += 3;
                }
                else if (strncasecmp(strdate, "CST", 3) == 0) {
                    tm->tm_zone = "CST";
                    strdate += 3;
                }
                else if (strncasecmp(strdate, "CEST", 4) == 0) {
                    tm->tm_zone = "CEST";
                    strdate += 4;
                }
                continue;
            case 'z':
                sscanf(strdate, "%05ld", tm->tm_gmtoff);
                tm->tm_gmtoff = (tm->tm_gmtoff / 100 * 3600) + (tm->tm_gmtoff % 100 * 60);
                strdate += 5;
                continue;
            case 'c':
                strdate += dfParseTime(strdate, "%a %b %d %H:%M:%S %Y", tm);
                continue;
            case 'x':
                strdate += dfParseTime(strdate, "%d/%m/%y", tm);
                continue;
            case 'X':
                strdate += dfParseTime(strdate, "%H:%M:%S", tm);
                continue;
                
        }
    }
    return strdate - original;
}






class DfDate {
  time_t t;
  static char sharedBuffer[128];

public:
    static const char* defaultFormat;


    DfDate(time_t _t) { t = _t; }

    DfDate(const char* strdate, const char* fmt = defaultFormat) {
        struct tm tm{};

        dfParseTime(strdate, fmt, &tm);
        t = mktime(&tm);
    }


    operator time_t() const {
        return t;
    }

    const char* toString(char* buf = sharedBuffer, const char* fmt = defaultFormat) const {
        struct tm* tm = localtime(&t);
        strftime(buf, 63, fmt, tm);
        return buf;
    }
};


char DfDate::sharedBuffer[128];
const char* DfDate::defaultFormat = "%Y-%m-%d %H:%M:%S";



#endif // _DF_DATE_HPP_