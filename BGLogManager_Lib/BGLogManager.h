#pragma once

#include "BGLog.h"

// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>

#include <queue>
#include <map>
#include <vector>

#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>

/**
 * <pre>
 * 로그를 관리한다.
 *
 * 기능
 * 1. 로그를 레벨 별로 관리한다. (trace, debug, info, err, fatal...)
 * 2. 로그파일이 커지지 않도록 기간마다 새로운 폴더/파일이 생기도록 한다.
 * 3. 정보 추출용 로그를 따로 관리 한다. (extract1, extract2, ...)
 *
 * ! 새로운 로그 타입 추가 시 (데이터 추출용)
 * BGLogEnum에 로그레벨 추가 후에 Init에서 셋팅해주면 된다.
 *
 * </pre>
*/



class BGLogManager
{
	using LogQueue = std::queue<BGLog>;

	// key:loglevel, value:string
	using LogLevel_StringMap = std::map<ELogLevel, std::string>;
	// key:string, value:string
	using String_FileStreamMap = std::map<std::string, std::fstream*>;
	
public:
	// 로그 시스템을 시작합니다.
	bool Start();

	// 로그 시스템을 종료합니다.
	bool Stop();
	
	// queue에 로그를 넣습니다.
	void PushLog(ELogLevel level, char* func_name, char* msg, ...);

	void SetLogLevel(ELogLevel logLevel);
	ELogLevel GetLogLevel();
	
private:
	std::mutex					m_queueLock;
	LogQueue					m_queue;

	std::thread*				m_pRunThread;
	
	struct tm					m_lastCreateFileName;

	//std::mutex					m_rwLogLevelLock;
	ELogLevel					m_eLogLevel;


	// 로그레벨-파일접두사 매칭
	LogLevel_StringMap			m_logPrefixFilNameMap;
	
	// 로그레벨-폴더이름 매칭
	LogLevel_StringMap			m_logForderNameMap;

	// 폴더이름-파일스트림
	String_FileStreamMap		m_forderNameFileStreamMap;


private:
	// 로그 시스템에 필요한 설정을 합니다.
	bool Init();

	// 로그 시스템 종료요청로그인지 확인합니다.
	bool IsStopRequest(BGLog&);

	// 로그 레벨을 검사합니다.
	bool CheckLogLevel(BGLog&);
	bool CheckLogLevel(ELogLevel);
	
	// queue에서 로그를 하나 꺼냅니다.
	BGLog Pick();
		

	// log를 기록합니다.
	void Write(BGLog&);

	// 파일이름이 최신이 아니라면 교체합니다.
	bool CheckLogFileNameAndRenew();

	// 최신 파일이름으로 파일스트림을 생성합니다.
	void RenewLogFileStream();

	///////////////////////////////
	// 로그 전용 스레드 함수 입니다
	///////////////////////////////
	static void Run(BGLogManager*);



	//////////////////////////////////////////////////
	/** 싱글톤 구현 부*/
private:
	static std::once_flag onceflag_;
	static std::unique_ptr<BGLogManager> instance_;

protected:
	BGLogManager() = default;
	BGLogManager(const BGLogManager&) = delete;
	BGLogManager& operator=(const BGLogManager&) = delete;

public:
	static BGLogManager* Instance();
	//////////////////////////////////////////////////
};

#define g_LogManager (*BGLogManager::Instance())
#define BG_LOG_DEBUG(...)		g_LogManager.PushLog(ELogLevel::BG_DEBUG,	__FUNCTION__, ##__VA_ARGS__)
#define BG_LOG_INFO(...)		g_LogManager.PushLog(ELogLevel::BG_INFO,	__FUNCTION__, ##__VA_ARGS__)
#define BG_LOG_WARNING(...)		g_LogManager.PushLog(ELogLevel::BG_WARNING,	__FUNCTION__, ##__VA_ARGS__)
#define BG_LOG_ERROR(...)		g_LogManager.PushLog(ELogLevel::BG_ERROR,	__FUNCTION__, ##__VA_ARGS__)
#define BG_LOG_FATAL(...)		g_LogManager.PushLog(ELogLevel::BG_FATAL,	__FUNCTION__, ##__VA_ARGS__)