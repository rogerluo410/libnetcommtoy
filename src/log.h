#ifndef _CT_LOG_H
#define _CT_LOG_H
#pragma once

#include <stdio.h>
#include <errno.h>

#include "nocopyable.h"

enum Level
{
  TRACE =0,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL,
};
   
class Log : public nocopyable
{
public:
        static void rootLog(const char *fileName=NULL) 
        {
            //static Log log;
            if(fileName !=NULL)
				       log_instance.redirect(fileName);
        }

		    static Log& getLog()
		    {
			     return log_instance;
		    }
private:
        Log();
        Log(const char* fileName);
        ~Log();
        void redirect(const char* fileName);

        void setLevel(Level level) { m_level = level; }
        Level getLevel() { return m_level; }

        void trace(const char* file, const char* function, int line, const char* fmt, ...);
        void debug(const char* file, const char* function, int line, const char* fmt, ...);
        void info(const char* file, const char* function, int line, const char* fmt, ...);
        void warn(const char* file, const char* function, int line, const char* fmt, ...);
        void error(const char* file, const char* function, int line, const char* fmt, ...);
        void fatal(const char* file, const char* function, int line, const char* fmt, ...);
    
private:
        void log(const char* level, const char* file, const char* function, int line, const char* msg);

private:
        FILE* m_fd;
        Level m_level;
		    static Log log_instance;
};   
    
#define LOG_TRACE(fmt, args...) Log::getLog().trace(__FILE__, __FUNCTION__, __LINE__, fmt, ##args)
#define LOG_DEBUG(fmt, args...) Log::getLog().debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##args)
#define LOG_INFO(fmt, args...) Log::getLog().info(__FILE__, __FUNCTION__, __LINE__, fmt, ##args)
#define LOG_WARN(fmt, args...) Log::getLog().warn(__FILE__, __FUNCTION__, __LINE__, fmt, ##args)
#define LOG_ERROR(fmt, args...) Log::getLog().error(__FILE__, __FUNCTION__, __LINE__, fmt, ##args)
#define LOG_FATAL(fmt, args...) Log::getLog().fatal(__FILE__, __FUNCTION__, __LINE__, fmt, ##args)

#define LogAppend(level,fmt,args...)\
	switch(level) \
	{\
		case 0:\
			LOG_TRACE(fmt,##args);\
		case 1:\
			LOG_DEBUG(fmt,##args);\
		case 2:\
			LOG_INFO(fmt,##args);\
		case 3:\
			LOG_WARN(fmt,##args);\
		case 4:\
			LOG_ERROR(fmt,##args);\
		case 5:\
			LOG_FATAL(fmt,##args);\
		default:\
			break;\
	}

  const char* errorMsg(int code);
#endif