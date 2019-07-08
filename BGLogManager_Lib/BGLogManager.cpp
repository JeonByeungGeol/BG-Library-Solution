#include "BGLogManager.h"

#define VERSION "v1.0.1"

/**
 * !���ο� �α׸� �߰��ϸ�, �̰��� �߰��ؾ� �մϴ�.
 * �α� �ý��ۿ� �ʿ��� ������ �մϴ�.
 * logLevel �� � �α����Ͽ� ����� �ϴ��� �����մϴ�.
 * �ʿ��� ���� ��Ʈ���� �����մϴ�.
*/
bool BGLogManager::Init()
{
	m_pRunThread = nullptr;

	m_eLogLevel = ELogLevel::BG_DEBUG;

	// ������ �α����� ���� �ð�
	m_lastCreateFileName.tm_hour = -1;
	
	/**
	 * ���� ���� ����

	 * 1. �α׷��� ���� ���� �տ� �ٴ� prefix ����
	 * 2. �α׷��� ���� ���� �̸� ����
	*/

	// 1. �α׷��� ���� ���� �տ� �ٴ� prefix ����
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_NONE, "NONE"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_DEBUG, "DEBUG"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_INFO, "INFO"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_WARNING, "WARNING"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_ERROR, "ERROR"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_FATAL, "FATAL"));
	
	// 2. �α׷��� ���� ���� �̸� ����
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_NONE, "log"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_DEBUG, "log"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_INFO, "log"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_WARNING, "err"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_ERROR, "err"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_FATAL, "err"));

	// 3. �������� ���� ��Ʈ�� ����
	std::fstream* pNewFileStream{ nullptr };
	for (auto entry : m_logForderNameMap) {
		std::string strForderName = entry.second;

		if (m_forderNameFileStreamMap.end() == m_forderNameFileStreamMap.find(strForderName)) {
			pNewFileStream = new std::fstream;
			m_forderNameFileStreamMap.insert(std::make_pair(strForderName, pNewFileStream));
		}
	}


	/**
	 * ���ο� �α� Ÿ�� �߰���
	 * 1��
	 * m_logLevelLogNameMap.insert(std::make_pair(ELogLevel::BG_EXTRACT_DATA_1, "EXTRACT_DATA_1"));
	 * 2��
	 * pFileStream = new std::fstream;
	 * m_logFileStreamVec.push_back(std::make_pair("extract_data_1", pFileStream));
	 * m_forderNameFileStreamMap.insert(std::make_pair("extract_data_1", pFileStream));
	 * 3��
	 * m_logLevelForderNameMap.insert(std::make_pair(ELogLevel::BG_EXTRACT_DATA_1, "extract_data_1"));
	*/
	return true;
}

/**
 * �� �Լ��� ȣ���ϸ� �α׽ý����� �����մϴ�.
 * �α׸� ��� ���� �����带 ����� ����
*/
bool BGLogManager::Start()
{
	if (!Init())
		return false;
	
	if (m_pRunThread != nullptr)
		return false;

	m_pRunThread = new std::thread{BGLogManager::Run, this};
	
	BG_LOG_INFO("BGLogManager Version Check [VERSION = %s]", VERSION);

	return true;
}

/**
 * �� �Լ��� ȣ���ϸ� �α׽ý����� �����մϴ�.
 * �α� �ý��� ���� ��û �α׸� ���뽺���忡 �����Ŀ�
 * �α����� �����尡 Ȯ���ϸ�, �����尡 ����� ���� Ȯ���� �ڿ�
 * true�� �����մϴ�.
*/
bool BGLogManager::Stop()
{
	m_queueLock.lock();
	m_queue.push(BGLog{ELogLevel::BG_LOG_SYSTEM_EXIT, "STOP"});
	m_queueLock.unlock();

	m_pRunThread->join();

	delete m_pRunThread;
	m_pRunThread = nullptr;
	
	for (auto fStream : m_forderNameFileStreamMap) {
		if (fStream.second->is_open())
			fStream.second->close();
		delete fStream.second;
		fStream.second = nullptr;
	}

	return true;
}

/**
* �α� ���� ������ �Լ� �Դϴ�.
* queue���� �α׸� ���� ����մϴ�.
* �α� ���� ��û�� ���������� �����մϴ�.
* Stop�Լ��� ȣ���ϸ� �ش� �Լ��� ����˴ϴ�.
*/
void BGLogManager::Run(BGLogManager* pLogMgr)
{
	while (true)
	{
		BGLog log = pLogMgr->Pick();
		if (!log.Valid())
			continue;

		if (pLogMgr->IsStopRequest(log)) {
			return;
		}

		// �α� ����
		pLogMgr->Write(log);
	}

	return;
}

/**
 * �α� �ý����� �������Ḧ ��û�ϴ� �αװ� ���Դ��� Ȯ���մϴ�.
 * ���� ��û �α״� [�α׷��� : INFO, ���� : "STOP"] �Դϴ�.
 * �α� ���� �����忡���� ȣ�� �մϴ�.
 * true�� �����ϸ�, �α� ���� ������� ����˴ϴ�.
*/
bool BGLogManager::IsStopRequest(BGLog& log)
{
	return ELogLevel::BG_LOG_SYSTEM_EXIT == log.GetLevel();
}

/**
* Queue���� �α׸� �����ϴ�.
* ���� �α״� Valid�� ȣ���ؼ�
* ��ȿ�� �α����� Ȯ���� �Ŀ� ����մϴ�.
*/
BGLog BGLogManager::Pick()
{
	m_queueLock.lock();		//--*--*--*--*--*--*--

	if (m_queue.empty()) {
		m_queueLock.unlock();	//**-**-**-**-**-**
		return BGLog{};
	}

	BGLog log = m_queue.front();
	m_queue.pop();

	m_queueLock.unlock();	//**-**-**-**-**-**

	return log;
}

/**
* �α׸� �˸��� ���Ͽ� ����մϴ�.
* Ư�� �α������� ������ ����Ʈ ���Ͽ� ����մϴ�.
* ����Ʈ ������ log������ ����ϰ�, m_logfileStreamVec 0��° �ε����� �����մϴ�.
*/
void BGLogManager::Write(BGLog &log)
{
	if (CheckLogFileNameAndRenew()) {
		RenewLogFileStream();
	}

	std::fstream* pDefaultFileStream{ nullptr };
	std::fstream* pSpecificFileStream{ nullptr };

	// ������ ���� �⺻ ����
	LogLevel_StringMap::iterator forderNameIter = m_logForderNameMap.find(ELogLevel::BG_NONE);
	if (forderNameIter != m_logForderNameMap.end()) {
		std::string defaultForderName = forderNameIter->second;
		String_FileStreamMap::iterator fstreamIter = m_forderNameFileStreamMap.find((defaultForderName));
		pDefaultFileStream = fstreamIter->second;
	}	

	// Ư�� �α� ������ Ư�� ����
	forderNameIter = m_logForderNameMap.find(log.GetLevel());
	if (forderNameIter != m_logForderNameMap.end()) {
		String_FileStreamMap::iterator fstreamIter = m_forderNameFileStreamMap.find(forderNameIter->second);
		if (fstreamIter != m_forderNameFileStreamMap.end())
			pSpecificFileStream = fstreamIter->second;
	}

	if (pDefaultFileStream) {
		log.Write(pDefaultFileStream);

		printf(log.GetContents().c_str());
		fflush(stdout);
	}

	if (pSpecificFileStream)
		log.Write(pSpecificFileStream);
}

/**
* �����̸��� �ð������� ���ŵ˴ϴ�.
* ������ ���ŵǸ� true�� �����մϴ�.
* ���� ������ �״�� ����� �� ������ false�� �����մϴ�.
*/
bool BGLogManager::CheckLogFileNameAndRenew()
{
	struct tm ltm;
	time_t t = time(NULL);
	localtime_s(&ltm, &t);

	if (ltm.tm_hour != m_lastCreateFileName.tm_hour) {
		m_lastCreateFileName = ltm;
		return true;
	}

	return false;
}

/**
* ���� ���Ͻ�Ʈ���� �ݰ�,
* �ֽ� ���� �̸��� �°� ���ο� ���Ͻ�Ʈ���� �����մϴ�.
* CheckLogFileNameAndRenew()�Լ��� false�� �����ϸ� �ش��Լ��� ȣ���մϴ�.
*/
void BGLogManager::RenewLogFileStream()
{
	char timestr[32];
	sprintf_s(timestr, "%04d-%02d-%02d_%02d.LOG", m_lastCreateFileName.tm_year + 1900,
		m_lastCreateFileName.tm_mon + 1,
		m_lastCreateFileName.tm_mday,
		m_lastCreateFileName.tm_hour);

	for (auto entry : m_forderNameFileStreamMap) {
		if (entry.second->is_open())
			entry.second->close();

		std::string fileName{ entry.first };
		fileName.append(timestr);
		entry.second->open(fileName, std::ios::app);
	}
}

/**
 * ����ؾ� �ϴ� �α����� �α׷����� �˻��մϴ�.
 * ���� ���� �� �α׷��� �̻��϶� ���� �α׸� ����մϴ�.
*/
bool BGLogManager::CheckLogLevel(BGLog & log)
{
	return (static_cast<int>(m_eLogLevel) <= static_cast<int>(log.GetLevel()));
}

bool BGLogManager::CheckLogLevel(ELogLevel level)
{
	return (static_cast<int>(m_eLogLevel) <= static_cast<int>(level));
}

/**
 * �α׸� �ð�, ����, ����, ȣ���� �Լ��� �����Ͽ� Queue�� �ֽ��ϴ�.
 * ���� ���뽺���忡�� �ϳ��� ���� ó���մϴ�.
*/
void BGLogManager::PushLog(ELogLevel level, char* func_name, char* msg, ...)
{
	if (!CheckLogLevel(level))
		return;

	struct tm ltm;
	time_t t = time(NULL);
	localtime_s(&ltm, &t);


	// �α� ����
	char tmp[4000] = "";	
	char result[4096] = "";
	va_list args;
	va_start(args, msg);
	vsprintf_s(tmp, _countof(tmp), msg, args);
	va_end(args);

	// �α� �̸�
	std::string logName{ "[" };
	auto name = m_logPrefixFilNameMap.find(level);
	if (name == m_logPrefixFilNameMap.end()) {
		BG_LOG_ERROR("logLevelLogNameMap.find() Failed! [logLevel=%d]", static_cast<int>(level));
		return;
	}
	logName.append(name->second);
	logName.append("]");

	// �α� �ð�
	char time_tmp[100] = "";
	sprintf_s(time_tmp, "[%02d:%02d:%02d]", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
	std::string logTime{ time_tmp };

	// �α� �߻��� �Լ�
	std::string logFunc{ "(" };
	logFunc.append(func_name);
	logFunc.append(")");

	std::string logContents{ tmp };
	logContents.append(" - ");
	logContents.append(logFunc);

	// ����
	sprintf_s(result, "%-10s %-7s %-s\n"
		, logTime.c_str()
		, logName.c_str()
		, logContents.c_str());

	BGLog log{ level, result };
	
	m_queueLock.lock();
	m_queue.push(log);
	m_queueLock.unlock();
}

void BGLogManager::SetLogLevel(ELogLevel logLevel)
{
	m_eLogLevel = logLevel;
}

ELogLevel BGLogManager::GetLogLevel()
{
	return m_eLogLevel;
}

std::once_flag BGLogManager::onceflag_;
std::unique_ptr<BGLogManager> BGLogManager::instance_{ nullptr };

BGLogManager* BGLogManager::Instance() {
	std::call_once(onceflag_, []() {
		instance_.reset(new BGLogManager);
	});
	return instance_.get();
}


