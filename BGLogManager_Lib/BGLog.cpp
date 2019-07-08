#include "BGLog.h"



BGLog::~BGLog()
{
}


// 유효한 로그인지 확인합니다.
bool BGLog::Valid() 
{ 
	return m_logLevel != ELogLevel::BG_NONE;
}

// 종료 요청 로그인지 확인합니다.
bool BGLog::IsStopRequest() 
{
	if (m_logLevel == ELogLevel::BG_INFO && m_content == "STOP") 
		return true; 
	return false;
}

// 로그 레벨을 얻습니다.
ELogLevel BGLog::GetLevel() 
{
	return m_logLevel;
}

// 로그 내용을 얻습니다.
std::string BGLog::GetContents()
{
	return m_content;
}

// 로그를 파일에 씁니다.
void BGLog::Write(std::fstream* fs) {
	*fs << m_content;
}