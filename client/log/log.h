#ifndef _LOG_H_
#define _LOG_H_

#include <string>
#include <sys/stat.h>
#include <assert.h>

#ifdef _WIN32
#include <direct.h>
#elif _LINUX
#include <unistd.h>
#endif

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

typedef enum LOG_Modeenum{
	LOG_ADD,
	LOG_OVER
}LOG_MODE;

class Log{
public:
	Log(const int type, const int level);
	~Log();
	
	void write(const char* logfilename, const int logfileline, const int level, const char* format, ...);
private:

	char path[LOG_MAXBUF];
	FILE* file;
	int level;

	time_t t;
    struct tm * lt;
};

#endif /* _LOG_H_ */
