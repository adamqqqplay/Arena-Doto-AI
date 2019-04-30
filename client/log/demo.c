#include "log.h"

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
