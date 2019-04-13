/*
作者：闫文山

时间：2017/07/02

介绍：
本日志类功能强大：

1、小巧可爱，全程序（YLog.h+YLog.cpp）不过一百余行而已，比之网上其他的如log4cpp等堪称超轻量级；

2、可输出每条日志信息的日志级别、输出时间（可精确到毫秒级）、所在程序文件名、所在行数、日志信息（支持可变长、标准格式化 %s,%d,%f 等参数）；

3、兼容性强大，在vc++6.0、vs2010、vs2015上都测试通过；

4、构造函数为YLog(const int type, const int level,std::string logname="");

type：可定义每一个日志文件的输出方式，支持每次程序运行追加（YLOG_ADD)或覆盖(YLOG_OVER)之前的日志文件；

level：可定义下限级别有严格程度从低到高的TRACE < INFO < WARNING < ERROR < FATAL < RELEASE 六种日志级别，仅当调用日志输出函数的输出级别高于定义下限级别时才会往日志文件中写日志；下限设为最高release级别时不会产生日志文件；

logname：可定义每一个日志输出的文件路径，支持相对路径与绝对路径，文件夹不存在依然能自己创建；可以省略不写，则默认在当前文件夹下产生对应程序名称的日志文件。
*/

#include "YLog.h"

int main(){

	//创建日志对象log1，日志输出文件为绝对路径的"D:\\y1.log"，如果存在则追加；日志输出下限级别为INFO级别
	YLog log1(YLOG_ADD, YLOG_INFO,"D:\\y1.log");
	//穿件日志对象log2，日志输出文件为相对路径的"y2.log"，如果存在则覆盖；日志输出下限级别为ERROR级别
	YLog log2(YLOG_OVER, YLOG_ERROR,"y2.log");

	int a = 256;
	double b = 0.789;
	std::string c = "hello";

	log1.w(YFL,YLOG_INFO, "info级别的日志输出：%d",a);//INFO级别不低于log1的下限INFO级别，正常写入日志文件
	a++;
	log1.w(YFL,YLOG_WARNING, "warning级别的日志输出：%d",a);//正常写入
	a++;
	log1.w(YFL,YLOG_ERROR, "error级别的日志输出：%d",a);//正常写入
	a++;
	log1.w(YFL,YLOG_FATAL,"fatal级别的日志输出：%d,%f,%s",a,b,c.c_str());//正常写入


	log2.w(YFL,YLOG_INFO, "info级别的日志输出：%f",b);//INFO级别低于log2的下限ERROR级别，不写入日志
	b*=10;
	log2.w(YFL,YLOG_WARNING, "warning级别的日志输出：%f",b);//不写入日志
	b*=10;
	log2.w(YFL,YLOG_ERROR, "error级别的日志输出：%f",b);//正常写入
	b*=10;
	log2.w(YFL,YLOG_FATAL,"fatal级别的日志输出：%d,%f,%s",a,b,c.c_str());//正常写入

	return 0;
}