#pragma once

#include "BGLogEnum.h"

#include <fstream>
#include <string>

class BGLog
{
public:
	BGLog() : m_logLevel(ELogLevel::BG_NONE) {};
	BGLog(ELogLevel level, std::string content)
		: m_logLevel(level), m_content(content)
	{}
	~BGLog();


public:
	// 유효한 로그인지 확인합니다.
	bool Valid();

	// 종료 요청 로그인지 확인합니다.
	bool IsStopRequest();

	// 로그 레벨을 얻습니다.
	ELogLevel GetLevel();

	// 로그 내용을 얻습니다.
	std::string GetContents();

	// 로그를 파일에 씁니다.
	void Write(std::fstream* fs);

private:
	ELogLevel m_logLevel;
	std::string m_content;
};

