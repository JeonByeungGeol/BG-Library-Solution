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
	// ��ȿ�� �α����� Ȯ���մϴ�.
	bool Valid();

	// ���� ��û �α����� Ȯ���մϴ�.
	bool IsStopRequest();

	// �α� ������ ����ϴ�.
	ELogLevel GetLevel();

	// �α� ������ ����ϴ�.
	std::string GetContents();

	// �α׸� ���Ͽ� ���ϴ�.
	void Write(std::fstream* fs);

private:
	ELogLevel m_logLevel;
	std::string m_content;
};

