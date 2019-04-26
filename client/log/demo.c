#include "log.h"

int main(int argc, char* argv[])
{
	Log log(LOG_INFO);
	
	log.info(YFL, "info级别的日志输出：%s", "12345");
	log.error(YFL, "error级别的日志输出：%s", "123456");
	log.warn(YFL, "warn级别的日志输出：%s", "123456");
	return 0;
}
