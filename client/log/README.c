
int main(int argc, char* argv[])
{
	Log log1(LOG_INFO);
	log1.write(YFL, LOG_INFO, "info级别的日志输出：%s", "12345");//INFO级别不低于log1的下限INFO级别，正常写入日志文件
	return 0;
}

int main(int argc, char* argv[])
{
	Log log(LOG_INFO);
	
	log.info(YFL, "info级别的日志输出：%s", "12345");
	log.error(YFL, "error级别的日志输出：%s", "123456");
	log.warn(YFL, "warn级别的日志输出：%s", "123456");

	log._info("info级别的日志输出：%s", "12345_");
	log._error("error级别的日志输出：%s", "123456_");
	log._warn("warn级别的日志输出：%s", "123456_");
	return 0;
}

/*类初始化参数
1. 最低输出LOG级别, 可取LOG_INFO,
		LOG_WARNING,
		LOG_ERROR,
		LOG_FATAL,
		LOG_RELEASE
*/

/*
write方法说明
*
1.第一个参数是个宏，输出要log的文件的名字和行数, 就取YFL(这个定义在log.h里面, 不方便在初始化时设置, 因为要输出的行数只有在write时获取)
2.LOG级别 可取 LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL, LOG_RELEASE
3.后面的和printf差不多
*/

/* 输出样例：[2019-3-13 12:30:31] [INFO]     D:\SOURCE\Log\Log.cpp(99): info级别的日志输出：12345*/


/**
 * info, warn, error方法说明
 1.第一个参数是个宏，输出要log的文件的名字和行数, 就取YFL(这个定义在log.h里面, 不方便在初始化时设置, 因为要输出的行数只有在write时获取)
 2.后面和printf一样
 * 
 */


/**
 * _info, _warn, _error方法说明
 * 和上面的一致，只是用宏处理了自动添加YFL
 1.现在和printf用法完全一样
 * 
 */

