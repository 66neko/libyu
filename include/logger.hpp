/*
*	logger-libyu
*	copyright - libyu
*	date - 2017-1-10
*	github - https://github.com/yuyugenius/libyu
*/
#pragma once

#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <cstdlib>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif // _WIN32

#define _LIBYU namespace libyu {
#define _END }

#ifdef ENABLE_LOGGER
#define LOGGER_INIT(fileName) libyu::Logger::GetInstance()->Load(fileName);
#define LOGGER_SAMPLE(lvl, format, ...) {std::lock_guard<std::mutex> lock{libyu::Logger::GetInstance()->mtx}; \
	if(libyu::Logger::GetInstance()->WriteHead(__FILE__,__LINE__,lvl)){ \
	libyu::Logger::GetInstance()->WriteBody(format,## __VA_ARGS__); \
	libyu::Logger::GetInstance()->WriteBody("\n"); \
	libyu::Logger::GetInstance()->Flush(); \
	}}

#define LOGGER_ERROR(format, ...) LOGGER_SAMPLE(libyu::Logger::LEVEL::LOG_LV_ERROR,format,##__VA_ARGS__)
#define LOGGER_WARNING(format, ...) LOGGER_SAMPLE(libyu::Logger::LEVEL::LOG_LV_WARNING,format,##__VA_ARGS__)
#define LOGGER_INFO(format, ...) LOGGER_SAMPLE(libyu::Logger::LEVEL::LOG_LV_INFO,format,##__VA_ARGS__)
#define LOGGER_DEBUG(format, ...) LOGGER_SAMPLE(libyu::Logger::LEVEL::LOG_LV_DEBUG,format,##__VA_ARGS__)
#else
#define LOGGER_INIT(fileName)
#define LOGGER_SAMPLE(lvl, format, ...)

#define LOGGER_ERROR(format, ...)
#define LOGGER_WARNING(format, ...)
#define LOGGER_INFO(format, ...) 
#define LOGGER_DEBUG(format, ...)
#endif // ENABLE_LOGGER



#pragma warning(disable:4996)

_LIBYU

class Logger
{
public:
	enum LEVEL : int
	{
		LOG_LV_ERROR,
		LOG_LV_WARNING,
		LOG_LV_INFO,
		LOG_LV_DEBUG
	};
public:
	static Logger* GetInstance() {
		static Logger instance{};
		return &(instance);
	}

	bool Load(const char* fileName) {
		if (m_out != nullptr)
		{
			return false;
		}
		m_out = fopen(fileName, "a+");
		if (m_out != nullptr)
		{
			return true;
		}
		return false;
	}

	bool WriteHead(const char* file, int line, LEVEL log_lv) {
		if (m_out == nullptr)
		{
			return false;
		}
		std::chrono::time_point<std::chrono::system_clock> raw_time = std::chrono::system_clock::now();
		auto now_time = std::chrono::system_clock::to_time_t(raw_time);
		auto ptm = std::localtime(&now_time);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
			(raw_time - std::chrono::system_clock::from_time_t(std::mktime(ptm))).count();
		char str[60]{ 0 };
		sprintf(str, "[%d-%02d-%02d %02d:%02d:%02d.%03d]",
			(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
			(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec, (int)(ms));
		//std::cerr << str << log << std::endl;
		//m_out << log << std::endl;
		WriteBody(str);
		WriteBody("[%s:%d]", file, line);
		WriteBody("[pid=%d]", _getpid());
		WriteBody("[thread=%d]", std::this_thread::get_id());
		WriteBody("[%s]", lv_str[log_lv].c_str());
		return true;
	}

	template <typename  T, typename  ... Types>
	void WriteBody(const T &head, const Types ... _value) {
		fprintf(m_out, head, _value...);
		//fprintf(stderr, head, _value...);
		return;
	}

	void Flush() {
		fflush(m_out);
	}

	std::mutex mtx;
private:
	Logger() :
		m_out{ nullptr }
	{
		//m_out.open(fileName, std::ios::out | std::ios::app);
		lv_str[LOG_LV_DEBUG] = "DEBUG";
		lv_str[LOG_LV_INFO] = "INFO";
		lv_str[LOG_LV_WARNING] = "WARNING";
		lv_str[LOG_LV_ERROR] = "ERROR";
	}

	~Logger()
	{
		if (m_out != nullptr)
		{
			fclose(m_out);
			m_out = nullptr;
		}
	}
private:
	FILE  *m_out;
	std::string lv_str[4];
};

_END
