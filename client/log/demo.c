#include "log.h"

int main(int argc, char* argv[])
{
	Log log(LOG_OVER, LOG_INFO);
	log.write(YFL, LOG_INFO, "info级别的日志输出：%s", "12345");
	return 0;
}
