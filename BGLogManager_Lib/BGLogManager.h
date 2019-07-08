#pragma once

#include "BGLog.h"

// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
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
 * �α׸� �����Ѵ�.
 *
 * ���
 * 1. �α׸� ���� ���� �����Ѵ�. (trace, debug, info, err, fatal...)
 * 2. �α������� Ŀ���� �ʵ��� �Ⱓ���� ���ο� ����/������ ���⵵�� �Ѵ�.
 * 3. ���� ����� �α׸� ���� ���� �Ѵ�. (extract1, extract2, ...)
 *
 * ! ���ο� �α� Ÿ�� �߰� �� (������ �����)
 * BGLogEnum�� �α׷��� �߰� �Ŀ� Init���� �������ָ� �ȴ�.
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
	// �α� �ý����� �����մϴ�.
	bool Start();

	// �α� �ý����� �����մϴ�.
	bool Stop();
	
	// queue�� �α׸� �ֽ��ϴ�.
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


	// �α׷���-�������λ� ��Ī
	LogLevel_StringMap			m_logPrefixFilNameMap;
	
	// �α׷���-�����̸� ��Ī
	LogLevel_StringMap			m_logForderNameMap;

	// �����̸�-���Ͻ�Ʈ��
	String_FileStreamMap		m_forderNameFileStreamMap;


private:
	// �α� �ý��ۿ� �ʿ��� ������ �մϴ�.
	bool Init();

	// �α� �ý��� �����û�α����� Ȯ���մϴ�.
	bool IsStopRequest(BGLog&);

	// �α� ������ �˻��մϴ�.
	bool CheckLogLevel(BGLog&);
	bool CheckLogLevel(ELogLevel);
	
	// queue���� �α׸� �ϳ� �����ϴ�.
	BGLog Pick();
		

	// log�� ����մϴ�.
	void Write(BGLog&);

	// �����̸��� �ֽ��� �ƴ϶�� ��ü�մϴ�.
	bool CheckLogFileNameAndRenew();

	// �ֽ� �����̸����� ���Ͻ�Ʈ���� �����մϴ�.
	void RenewLogFileStream();

	///////////////////////////////
	// �α� ���� ������ �Լ� �Դϴ�
	///////////////////////////////
	static void Run(BGLogManager*);



	//////////////////////////////////////////////////
	/** �̱��� ���� ��*/
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