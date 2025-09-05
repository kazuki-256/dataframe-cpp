#pragma once

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif




// parse month
inline size_t df_parse_month(const char* strmonth, size_t n, int* month) {
    static const char* MONTHS[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

    for (int i = 0; i < 12; i++) {
        if (strncasecmp(strmonth, MONTHS[i], n) == 0) {
            *month = i;
            return strlen(MONTHS[i]);
        }
    }
    return 0;
}

// parse weekday
inline size_t df_parse_weekday(const char* strweek, size_t n, int* week) {
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
int df_parse_time(const char* strdate, const char* fmt, struct tm* tm) {
    const char* fmt_start = fmt;
    int symbol, c;
    char buffer[10];
    int temp;

    // == reset tm values ==
    tm->tm_hour = 0;
    tm->tm_mon = 0;
    tm->tm_mday = 1;

    // == start parse ==

    while ((symbol = *fmt++) != 0) {
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
                if (sscanf(strdate, "%04d", &tm->tm_year) == 0) {
                    return -1;
                }

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
                if (sscanf(strdate, "%02d", &tm->tm_hour) == 0) {
                    return -1;
                }
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
                sscanf(strdate, "%05ld", &tm->tm_gmtoff);
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
            return 0;
          }
          if (!isdigit(c)) {
            strdate--;
            break;
          }
        }
        while (--temp > 0);
    }

    return 0;
}






class df_interval_t {
public:
    int years = 0, months = 0, days = 0;
    int hours = 0, minutes = 0, seconds = 0;

    short on_wday = -1;
    short on_mday = -1;
    short on_yday = -1;



    inline df_interval_t(const char* fmt) {
        const char* p = fmt;
        int value = 0;
        int c = *p;

        days = 0;

        while ((c = *p)) {
            while (c != '\0' && isspace(c)) c = *(p++);
            if (c == '\0') {
                return;
            }

            if (isdigit(c)) {
                sscanf(p, "%d", &value);

                while ((c = *p++) && isdigit(c));
                continue;
            }

            if (!isalpha(c)) {
                c = *(p++);
                continue;
            }

            if (strncasecmp(p, "year", 4) == 0) {
                years = value;
                p += 4;
                continue;
            }
            if (strncasecmp(p, "month", 5) == 0) {
                months = value;
                p += 5;
                continue;
            }
            if (strncasecmp(p, "week", 4) == 0) {
                days += value * 7;
                p += 4;
                continue;
            }
            if (strncasecmp(p, "day", 3) == 0) {
                days += value;
                p += 3;
                continue;
            }
            if (strncasecmp(p, "hour", 4) == 0) {
                hours += value * 7;
                p += 4;
                continue;
            }
            if (strncasecmp(p, "min", 3) == 0) {
                minutes += value * 7;
                p += 3;
                goto label_fix_lessing;
            }
            if (strncasecmp(p, "sec", 3) == 0) {
                seconds = value;
                p += 3;
                goto label_fix_lessing;
            }

            // couldn't parse
            p++;
            continue;
        label_fix_lessing:
            while ((c = *p++) != 0 && !isspace(c)) {
                p++;
            }
        }
    }



    // sum(years, months, ..., seconds), if total < 0, return -1. total == 0, return 0. total > 0, return 1
    inline int get_direction() const {
        int total = years + months + days + hours + minutes + seconds;

        return total < 0 ? -1
            : total == 0 ? 0
            : 1;
    }

    inline bool is_constant() const {
        return !(years || months);
    }

    inline time_t calculate_constant() const {
        return ((((years * 365 + months) * 30 + days) * 24 + hours) * 60 + minutes) * 60 + seconds;
    }


    // == formatting ==
    
    inline const char* c_str(char* buffer = DF_STATIC_BUFFER, size_t buffer_size = DF_STATIC_BUFFER_LENGTH) const {
        snprintf(buffer, buffer_size, "df_interval_t(%d years, %d months, %d days, %d hours, %d mintues, %d seconds)",
            years, months, days, hours, minutes, seconds);
        return buffer;
    }

    inline operator std::string() const {
            return std::string(c_str());
    }
};




class df_date_t {
  time_t t = DF_NULL_DATETIME;

public:
    inline df_date_t(time_t t) : t(t) {}

    inline df_date_t(const char* strdate, const char* fmt = DF_DATETIME_FORMAT) {
        parse_date(strdate, fmt);
    }

    inline df_date_t(std::string& strdate, const char* fmt = DF_DATETIME_FORMAT) {
        parse_date(strdate.c_str(), fmt);
    }


    inline df_date_t& parse_date(const char* strdate, const char* fmt = DF_DATETIME_FORMAT) {
        struct tm tm{};
        if (df_parse_time(strdate, fmt, &tm) == 0) {
            t = mktime(&tm);
        }
        return *this;
    }


    inline df_date_t& operator=(const char* strdate) {
        struct tm tm{};
        df_parse_time(strdate, DF_DATETIME_FORMAT, &tm);
        t = mktime(&tm);
        return *this;
    }

    // == interval ==

    inline df_date_t& operator+=(time_t interval) {
        t += interval;
        return *this;
    }

    inline df_date_t& operator+(time_t interval) {
        t += interval;
        return *this;
    }

    
    


    inline df_date_t operator+(const df_interval_t& interval) const {
        struct tm* tm = localtime(&t);

        struct tm dest{};
        dest.tm_year = tm->tm_year + interval.years;
        dest.tm_mon = tm->tm_mon + interval.months;
        dest.tm_mday = tm->tm_mday + interval.days;
        dest.tm_hour = tm->tm_hour + interval.hours;
        dest.tm_min = tm->tm_min + interval.minutes;
        dest.tm_sec = tm->tm_sec + interval.seconds;

        return df_date_t(mktime(&dest));
    }

    inline df_date_t& operator+=(const df_interval_t& interval) {
        *this = *this + interval;
        return *this;
    }



    inline df_date_t operator-(const df_interval_t& interval) const {
        struct tm* tm = localtime(&t);

        struct tm dest{};
        dest.tm_year = tm->tm_year - interval.years;
        dest.tm_mon = tm->tm_mon - interval.months;
        dest.tm_mday = tm->tm_mday - interval.days;
        dest.tm_hour = tm->tm_hour - interval.hours;
        dest.tm_min = tm->tm_min - interval.minutes;
        dest.tm_sec = tm->tm_sec - interval.seconds;

        return df_date_t(mktime(&dest));
    }

    inline df_date_t& operator-=(const df_interval_t& interval) {
        *this = *this - interval;
        return *this;
    }


    inline operator time_t() const {
        return t;
    }


    // == formatting ==

    inline operator std::string() const {
        return std::string(c_str());
    }

    inline const char* c_str(const char* fmt = DF_DATETIME_FORMAT, char* buffer = DF_STATIC_BUFFER, size_t buffer_size = DF_STATIC_BUFFER_LENGTH) const {
        struct tm* tm = localtime(&t);
        strftime(buffer, buffer_size, fmt, tm);
        return buffer;
    }


    // == std::cout ==

    friend inline std::ostream& operator<<(std::ostream& stream, const df_date_t date) {
        return stream << date.c_str();
    }
};


