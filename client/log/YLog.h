#ifndef _YLOG_H_
#define _YLOG_H_
//忽略printf等非安全函数警告
#pragma warning(disable:4996)

#include <windows.h>
#include <string>
#include <sys/stat.h>
#include <assert.h>

#define YFL __FILE__,__LINE__
#define YLOG_MAXBUF 1024
#define YLOG_SLEEP 4

typedef enum YLOG_LEVELenum{
	//any info
	YLOG_TRACE,
	//useful info
	YLOG_INFO,
	YLOG_WARNING,
	YLOG_ERROR,
	YLOG_FATAL,
	//none info
	YLOG_RELEASE,//only for #define YLOGLEVELn,can not log out
	YLOG_MIN = 0,
}YLOG_LEVEL;

typedef enum YLOG_TYPEenum{
	YLOG_ADD,
	YLOG_OVER
}YLOG_TYPE;

class YLog{
public:
	YLog(const int type, const int level,std::string logname="");
	~YLog();
	void w(const char* logfilename, const int logfileline, const int level, const char* format, ...);
private:
	char path[MAX_PATH];
	FILE*file;
	int level;
};

#endif /* _YLOG_H_ */