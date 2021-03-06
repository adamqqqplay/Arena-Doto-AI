#ifndef _LOG_H_
#define _LOG_H_

#include <string>
#include <sys/stat.h>
#include <assert.h>

#ifdef _WIN32
	#include <direct.h>
#elif __linux__
	#include <unistd.h>
#endif

//定义函数宏
#define _info(format, ...) info(YFL,format,__VA_ARGS__)
#define _warn(format, ...) warn(YFL,format,__VA_ARGS__)
#define _error(format, ...) error(YFL,format,__VA_ARGS__)

#include <stdarg.h>
#include <time.h>
#include <string.h>

#define YFL __FILE__,__LINE__

#define LOG_MAXBUF 2048
#define LOG_SLEEP 4

typedef enum LOG_LEVELenum{
	//any info
	LOG_TRACE,
		//useful info
		LOG_INFO,
		LOG_WARNING,
		LOG_ERROR,
		LOG_FATAL,
		//none info
		LOG_RELEASE,//only for #define LOGLEVELn,can not log out
		LOG_MIN = 0,
}LOG_LEVEL;

class Log{
public:
	Log(const int level);
	~Log();
	
	void write(const char* logfilename, const int logfileline, const int level, const char* format, ...);
	void info(const char* logfilename, const int logfileline, const char* format, ...);
	void warn(const char* logfilename, const int logfileline, const char* format, ...);
	void error(const char* logfilename, const int logfileline, const char* format, ...);
private:

	char path[LOG_MAXBUF];
	FILE* file;
	int level;

	time_t t;
    struct tm * lt;
};

#endif /* _LOG_H_ */
