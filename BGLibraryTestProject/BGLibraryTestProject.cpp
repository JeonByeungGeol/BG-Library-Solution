// BGLibraryTestProject.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include "BG_LIBRARY\Include\BGLogManager.h"


int main()
{
	g_LogManager.Start();

	BG_LOG_DEBUG("test");
	
	Sleep(100000);

	g_LogManager.Stop();

    return 0;
}

