// BGLibraryTestProject.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
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

