#ifndef _TIME_UTIL_H
#define _TIME_UTIL_H
#include <string.h>
#include <time.h>
#include <base/common.h>
#include <sys/time.h>

struct timeval {
	long tv_sec;   //Seconds
	long tv_usec;  //Micro seconds
};

class CtTimeUtil
{
public:
	static string from_unix_time(time_t t)
	{
		char buf[32] = {0};
		tm *tm_time = localtime(&t);
		snprintf(buf,sizeof(buf),"%4d-%02d-%02d %02d:%02d:%02d",
				tm_time->tm_year + 1900,tm_time->tm_mon + 1,tm_time->tm_mday				,tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
		return buf;
	}

    //
	static time_t unix_time_stamp(const string &timeStr)
	{
		tm tm_time;
		memset(&tm_time,0,sizeof(tm_time));

		tm_time.tm_year = atoi(timeStr.substr(0,4).c_str()) - 1900;
		tm_time.tm_mon  = atoi(timeStr.substr(5,2).c_str()) - 1;
		tm_time.tm_mday = atoi(timeStr.substr(8,2).c_str());
		tm_time.tm_hour = atoi(timeStr.substr(11,2).c_str());
		tm_time.tm_min  = atoi(timeStr.substr(14,2).c_str());
		tm_time.tm_sec  = atoi(timeStr.substr(17,2).c_str());

		return mktime(&tm_time);
	}

	//The seconds from 1970.1.1
	static void timeval get_time_of_day()
	{
		struct timeval val;

        //in sys/time.h
		gettimeofday((struct timeval *)&val,0);

		return val;
	}
};


#endif
