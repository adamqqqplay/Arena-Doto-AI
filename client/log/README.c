
int main(int argc, char* argv[])
{
	Log log1(LOG_ADD, LOG_INFO);
	log1.write(YFL, LOG_INFO, "info级别的日志输出：%s", "12345");//INFO级别不低于log1的下限INFO级别，正常写入日志文件
	return 0;
}

/*类初始化参数
1. LOG_ADD, LOG_OVER 第一个不覆盖上次生产的文件, 第二个会覆盖(注意, 因为现在每次输出的文件名不一样, 最好直接用LOG_OVER)
2. 最低输出LOG级别, 可取LOG_INFO,
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

