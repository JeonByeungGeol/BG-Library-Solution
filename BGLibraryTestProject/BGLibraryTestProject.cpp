// BGLibraryTestProject.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"



int main()
{
	g_LogManager.Start();

	if (!g_ConfigManager.Load(CONFIG_FILE_NAME)) {
		BG_LOG_ERROR("load fail");
		return -1;
	}

	ELogLevel logLevel = static_cast<ELogLevel>(g_ConfigManager.GetInt("LogLevel"));
	g_LogManager.SetLogLevel(logLevel);


	g_LogManager.Stop();

    return 0;
}

