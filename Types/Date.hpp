#ifndef _DF_DATE_HPP_
#define _DF_DATE_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif




// parse month
size_t df_parse_month(const char* strmonth, size_t n, int* month) {
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
size_t df_parse_weekday(const char* strweek, size_t n, int* week) {
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
size_t df_parse_time(const char* strdate, const char* fmt, struct tm* tm) {
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
                temp = 4;
                goto label_pass;
            case 'y':
                sscanf(strdate, "%02d", &tm->tm_year);
                tm->tm_year += 100; // 2000 - 1900
                temp = 2;
                goto label_pass;
            case 'm':
                sscanf(strdate, "%02d", &tm->tm_mon);
                tm->tm_mon -= 1;
                temp = 2;
                goto label_pass;
            case 'B':
                strdate += df_parse_month(strdate, 12, &tm->tm_mon);
                continue;
            case 'b':
                df_parse_month(strdate, 3, &tm->tm_mon);
                temp = 3;
                goto label_pass;
            case 'd':
                sscanf(strdate, "%02d", &tm->tm_mday);
                temp = 2;
                goto label_pass;
            case 'e':
                strncpy(buffer, strdate, 2);
                sscanf(buffer, "%d", &tm->tm_mday);
                temp = 2;
                goto label_pass;
            case 'j':
                sscanf(strdate, "%03d", &tm->tm_yday);
                temp = 3;
                goto label_pass;
            case 'A':
                strdate += df_parse_weekday(strdate, 12, &tm->tm_wday);
                continue;
            case 'a':
                df_parse_weekday(strdate, 3, &tm->tm_wday);
                temp = 3;
                goto label_pass;
            case 'u':
                sscanf(strdate, "%01d", &tm->tm_wday);
                temp = 1;
                goto label_pass;
            case 'w':
                sscanf(strdate, "%01d", &tm->tm_wday);
                tm->tm_wday = 7 - tm->tm_wday;
                temp = 1;
                goto label_pass;
            case 'U':
            case 'W':
                continue;   // week number (unusable)
            
            // == time parsing ==
            case 'H':
                sscanf(strdate, "%02d", &tm->tm_hour);
                temp = 2;
                goto label_pass;
            case 'l':
                sscanf(strdate, "%02d", &temp);
                tm->tm_hour = (tm->tm_hour + temp) % 24;
                temp = 2;
                goto label_pass;
            case 'p':
                if (strncasecmp(strdate, "PM", 2) == 0) {
                    tm->tm_hour = (tm->tm_hour + 12) % 24;
                    strdate += 2;
                }
                continue;
            case 'M':
                sscanf(strdate, "%02d", &tm->tm_min);
                temp = 2;
                goto label_pass;
            case 'S':
                sscanf(strdate, "%02d", &tm->tm_sec);
                temp = 2;
                goto label_pass;
            case 'f':
                continue;   // micro seconds (unusable)
            case 'Z':
                // symbol of time zone
                if (strncasecmp(strdate, "GMT", 3) == 0) {
                    tm->tm_zone = "GMT";
                    temp = 3;
                    goto label_pass;
                }
                else if (strncasecmp(strdate, "CST", 3) == 0) {
                    tm->tm_zone = "CST";
                    temp = 3;
                    goto label_pass;
                }
                else if (strncasecmp(strdate, "CEST", 4) == 0) {
                    tm->tm_zone = "CEST";
                    temp = 4;
                    goto label_pass;
                }
                continue;
            case 'z':
                sscanf(strdate, "%05ld", tm->tm_gmtoff);
                tm->tm_gmtoff = (tm->tm_gmtoff / 100 * 3600) + (tm->tm_gmtoff % 100 * 60);
                temp = 5;
                goto label_pass;
            case 'c':
                strdate += df_parse_time(strdate, "%a %b %d %H:%M:%S %Y", tm);
                continue;
            case 'x':
                strdate += df_parse_time(strdate, "%d/%m/%y", tm);
                continue;
            case 'X':
                strdate += df_parse_time(strdate, "%H:%M:%S", tm);
                continue;
                
        }

        continue;
      label_pass:
        do {
          if ((c = *(strdate++)) == 0) {
            return strdate - original;
          }
          if (!isdigit(c)) {
            strdate--;
            break;
          }
        }
        while (--temp > 0);
      }
    return strdate - original;
}








class df_date_t {
  time_t t;

public:
    df_date_t(time_t _t = DF_NULL_DATE) { t = _t; }

    df_date_t(const char* strdate, const char* fmt = DF_DATETIME_FORMAT) {
        struct tm tm{};
        df_parse_time(strdate, fmt, &tm);

        t = mktime(&tm);
    }

    df_date_t& operator=(const char* strdate) {
      struct tm tm{};
      df_parse_time(strdate, DF_DATETIME_FORMAT, &tm);
      t = mktime(&tm);
      return *this;
    }

    df_date_t& operator+(time_t offset) {
      t += offset;
      return *this;
    }


    operator time_t() const {
        return t;
    }

    const char* c_str(const char* fmt = DF_DATETIME_FORMAT, char* buffer = DF_STATIC_BUFFER, size_t buffer_size = DF_STATIC_BUFFER_LENGTH) const {
        struct tm* tm = localtime(&t);
        strftime(buffer, buffer_size, fmt, tm);
        return buffer;
    }
};



class df_interval_t {
public:
  int years, months, days;
  int hours, minutes, seconds;

  df_interval_t(const char* fmt, ...) {
    
  }

  
  const char* c_str(char* buffer = DF_STATIC_BUFFER, size_t buffer_size = DF_STATIC_BUFFER_LENGTH) const {
    snprintf(buffer, buffer_size, "df_interval_t(%d years, %d months, %d days, %d hours, %d mintues, %d seconds)",
        years, months, days, hours, minutes, seconds);
    return buffer;
  }
};



#endif // _DF_DATE_HPP_