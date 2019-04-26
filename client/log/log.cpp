#include "log.h"

char logMsg[LOG_MAXBUF];

Log::Log(const int level){
	time (&(this->t));
	this->lt = localtime (&(this->t));
	
	this->level=level;
	if(LOG_RELEASE <= level || LOG_MIN > level){
		this->file = NULL;
		return;
	}
	
#ifdef __linux__
	if(access("./log",0)==-1)
	{
		mkdir("./log", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		umask(S_IWOTH);	//权限
	}
#elif _WIN32
	if(_access("./log",0)==-1)
	{
		_mkdir("./log");
	}
#endif	
	
	sprintf(this->path, "%s_%02d_%02d_%02d.log", "./log/playerAI_",lt->tm_hour,lt->tm_min,lt->tm_sec);
	
	if(fopen(this->path,"r")!=NULL)	//判断这个文件名对应的文件是否存在
	{
		sprintf(this->path, "%s_%02d_%02d_%02d_1.log", "./log/playerAI_",lt->tm_hour,lt->tm_min,lt->tm_sec);	//存在的话改一下后缀
	}
	
	this->file = NULL;
	if (NULL != this->file)	{
		fclose(this->file);
	}
	
	this->file=fopen(this->path, "w");
	if(!this->file){
		assert(false);
	}
	
	assert(NULL != this->file);
}

Log::~Log(){
	if (NULL != this->file){
		fclose(this->file);
		this->file = NULL;
	}
}

void Log::write(const char* logfilename, const int logfileline, const int level, const char* format, ...){
	assert(LOG_RELEASE > level && LOG_TRACE <= level);
	if (level >= this->level){
		std::string mTag;
		switch(level){
		case LOG_TRACE:
			mTag="[TRACE]";
			break;
		case LOG_INFO:
			mTag="[INFO]";
			break;
		case LOG_WARNING:
			mTag="[WARNING]";
			break;
		case LOG_ERROR:
			mTag="[ERROR]";
			break;
		default:
			mTag="[FATAL]";
			break;
		}
		
		va_list args;
		va_start(args, format);    
		vsprintf(logMsg,format,args);
		va_end(args);
		
		if(strlen(logMsg)>LOG_MAXBUF){
			strcpy(logMsg,"TOO LONG,CAN NOT LOG!");
		}
		
		assert(NULL != this->file);

		time (&(this->t));
		this->lt = localtime (&(this->t));	//tm_mon里面0表示1月

		fprintf(this->file, "[%04d-%02d-%02d %02d:%02d:%02d] %-9.9s  %s(%d): %s\n", 
			this->lt->tm_year+1900,
			this->lt->tm_mon+1,	
			this->lt->tm_mday,
			this->lt->tm_hour,
			this->lt->tm_min,
			this->lt->tm_sec,
			mTag.c_str(),
			logfilename, logfileline,
			logMsg);
		fflush(this->file);
	}
	else{
		return;
	}
}

void Log::info(const char* logfilename, const int logfileline, const char* format, ...)
{
		va_list args;
		va_start(args, format);    
		vsprintf(logMsg,format,args);
		va_end(args);
		
		if(strlen(logMsg)>LOG_MAXBUF){
			strcpy(logMsg,"TOO LONG,CAN NOT LOG!");
		}
		
		assert(NULL != this->file);

		time (&(this->t));
		this->lt = localtime (&(this->t));	//tm_mon里面0表示1月

		fprintf(this->file, "[%04d-%02d-%02d %02d:%02d:%02d] %-9.9s  %s(%d): %s\n", 
			this->lt->tm_year+1900,
			this->lt->tm_mon+1,	
			this->lt->tm_mday,
			this->lt->tm_hour,
			this->lt->tm_min,
			this->lt->tm_sec,
			"[INFO]",
			logfilename, logfileline,
			logMsg);
		fflush(this->file);
}

void Log::error(const char* logfilename, const int logfileline, const char* format, ...)
{
		va_list args;
		va_start(args, format);    
		vsprintf(logMsg,format,args);
		va_end(args);
		
		if(strlen(logMsg)>LOG_MAXBUF){
			strcpy(logMsg,"TOO LONG,CAN NOT LOG!");
		}
		
		assert(NULL != this->file);

		time (&(this->t));
		this->lt = localtime (&(this->t));	//tm_mon里面0表示1月

		fprintf(this->file, "[%04d-%02d-%02d %02d:%02d:%02d] %-9.9s  %s(%d): %s\n", 
			this->lt->tm_year+1900,
			this->lt->tm_mon+1,	
			this->lt->tm_mday,
			this->lt->tm_hour,
			this->lt->tm_min,
			this->lt->tm_sec,
			"[ERROR]",
			logfilename, logfileline,
			logMsg);
		fflush(this->file);
}

void Log::warn(const char* logfilename, const int logfileline, const char* format, ...)
{
		va_list args;
		va_start(args, format);    
		vsprintf(logMsg,format,args);
		va_end(args);
		
		if(strlen(logMsg)>LOG_MAXBUF){
			strcpy(logMsg,"TOO LONG,CAN NOT LOG!");
		}
		
		assert(NULL != this->file);

		time (&(this->t));
		this->lt = localtime (&(this->t));	//tm_mon里面0表示1月

		fprintf(this->file, "[%04d-%02d-%02d %02d:%02d:%02d] %-9.9s  %s(%d): %s\n", 
			this->lt->tm_year+1900,
			this->lt->tm_mon+1,	
			this->lt->tm_mday,
			this->lt->tm_hour,
			this->lt->tm_min,
			this->lt->tm_sec,
			"[WARN]",
			logfilename, logfileline,
			logMsg);
		fflush(this->file);
}

