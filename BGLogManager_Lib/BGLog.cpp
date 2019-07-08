#include "BGLog.h"



BGLog::~BGLog()
{
}


// ��ȿ�� �α����� Ȯ���մϴ�.
bool BGLog::Valid() 
{ 
	return m_logLevel != ELogLevel::BG_NONE;
}

// ���� ��û �α����� Ȯ���մϴ�.
bool BGLog::IsStopRequest() 
{
	if (m_logLevel == ELogLevel::BG_INFO && m_content == "STOP") 
		return true; 
	return false;
}

// �α� ������ ����ϴ�.
ELogLevel BGLog::GetLevel() 
{
	return m_logLevel;
}

// �α� ������ ����ϴ�.
std::string BGLog::GetContents()
{
	return m_content;
}

// �α׸� ���Ͽ� ���ϴ�.
void BGLog::Write(std::fstream* fs) {
	*fs << m_content;
}