#include "YLog.h"
//可变长string模板
char yinfo[YLOG_MAXBUF];

YLog::YLog(const int type, const int level,std::string logname/* ="" */){
	this->level=level;
	if(YLOG_RELEASE <= level || YLOG_MIN > level){
		this->file = NULL;
		return;
	}
	if(""==logname){
		// 取进程执行文件名称
		char szExeName[MAX_PATH];
		if ( ::GetModuleFileNameA(NULL, szExeName, MAX_PATH) > 0)
		{
			char* pBegin = szExeName;
			char* pTemp  = szExeName;
			while ( *pTemp != 0 )
			{
				if ( *pTemp == '\\' )
				{
					pBegin = pTemp + 1;
				}
				pTemp++;
			}
			memcpy(szExeName, pBegin, strlen(pBegin)+1);
		}
		sprintf(this->path, "YLog(%s).log", szExeName);
	}
	else{
		char *findEnd = NULL;
		if (logname[1] != ':'){
			char nowPath[MAX_PATH];
			if (!GetModuleFileNameA(NULL, nowPath, MAX_PATH)){
				assert(false);
			}
			findEnd = nowPath + strlen(nowPath) - 1;
			while (*findEnd != '\\'){
				--findEnd;
			}
			*findEnd = '\0';
			if (logname[0] == '\\'){
				sprintf(this->path, "%s%s", nowPath, logname.c_str());
			}
			else{
				sprintf(this->path, "%s\\%s", nowPath, logname.c_str());
			}
		}
		else{
			sprintf(this->path, "%s", logname.c_str());
		}
		logname = "mkdir \"";
		logname = logname.append(this->path);
		logname = logname.substr(0, logname.find_last_of('\\'));
		//if folder not exist
		struct _stat fileStat;
		if (	!(	(_stat(logname.substr(7).c_str(), &fileStat) == 0) 
					&& 
					(fileStat.st_mode & _S_IFDIR)
				)	
				&&
				(logname.size() != 9)
			){
			logname.append("\"");
			system(logname.c_str());
		}
	}
	this->file = NULL;
	if (NULL != this->file)	{
		fclose(this->file);
	}
	if (type == YLOG_ADD){
		this->file = fopen(this->path, "at+");
		if(!this->file){
			assert(false);
		}
	}
	else{
		this->file=fopen(this->path, "w");
		if(!this->file){
			assert(false);
		}
	}
	assert(NULL != this->file);
}

YLog::~YLog(){
	if (NULL != this->file){
		fclose(this->file);
		this->file = NULL;
	}
}

void YLog::w(const char* logfilename, const int logfileline, const int level, const char* format, ...){
	assert(YLOG_RELEASE > level && YLOG_TRACE <= level);
	if (level >= this->level){
		std::string ymess;
		switch(level){
		case YLOG_TRACE:
			ymess="[TRACE]";
			break;
		case YLOG_INFO:
			ymess="[INFO]";
			break;
		case YLOG_WARNING:
			ymess="[WARNING]";
			break;
		case YLOG_ERROR:
			ymess="[ERROR]";
			break;
		default:
			ymess="[FATAL]";
			break;
		}
		SYSTEMTIME yst;
        ::GetLocalTime(&yst);
		
		va_list args;
		va_start(args, format);    
		vsprintf(yinfo,format,args);
		va_end(args); 
		if(strlen(yinfo)>YLOG_MAXBUF){
			strcpy(yinfo,"TOO LONG,CAN NOT LOG!");
		}
		assert(NULL != this->file);
		fprintf(this->file, "%-9.9s[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s(%d):\n%s\n", 
			ymess.c_str(),
			yst.wYear, yst.wMonth, yst.wDay,
			yst.wHour, yst.wMinute, yst.wSecond, yst.wMilliseconds, 
			logfilename, logfileline,
			yinfo);
		fflush(this->file);
#ifdef YLOG_SLEEP
		Sleep(YLOG_SLEEP);
#endif
	}
	else{
		return;
	}
}