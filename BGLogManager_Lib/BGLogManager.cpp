#include "BGLogManager.h"

#define VERSION "v1.0.1"

/**
 * !새로운 로그를 추가하면, 이곳에 추가해야 합니다.
 * 로그 시스템에 필요한 설정을 합니다.
 * logLevel 이 어떤 로그파일에 씌어야 하는지 설정합니다.
 * 필요한 파일 스트림을 생성합니다.
*/
bool BGLogManager::Init()
{
	m_pRunThread = nullptr;

	m_eLogLevel = ELogLevel::BG_DEBUG;

	// 마지막 로그파일 생성 시간
	m_lastCreateFileName.tm_hour = -1;
	
	/**
	 * 폴더 파일 설정

	 * 1. 로그레벨 별로 파일 앞에 붙는 prefix 설정
	 * 2. 로그레벨 별로 폴더 이름 설정
	*/

	// 1. 로그레벨 별로 파일 앞에 붙는 prefix 설정
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_NONE, "NONE"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_DEBUG, "DEBUG"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_INFO, "INFO"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_WARNING, "WARNING"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_ERROR, "ERROR"));
	m_logPrefixFilNameMap.insert(std::make_pair(ELogLevel::BG_FATAL, "FATAL"));
	
	// 2. 로그레벨 별로 폴더 이름 설정
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_NONE, "log"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_DEBUG, "log"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_INFO, "log"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_WARNING, "err"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_ERROR, "err"));
	m_logForderNameMap.insert(std::make_pair(ELogLevel::BG_FATAL, "err"));

	// 3. 폴더마다 파일 스트림 생성
	std::fstream* pNewFileStream{ nullptr };
	for (auto entry : m_logForderNameMap) {
		std::string strForderName = entry.second;

		if (m_forderNameFileStreamMap.end() == m_forderNameFileStreamMap.find(strForderName)) {
			pNewFileStream = new std::fstream;
			m_forderNameFileStreamMap.insert(std::make_pair(strForderName, pNewFileStream));
		}
	}


	/**
	 * 새로운 로그 타입 추가시
	 * 1번
	 * m_logLevelLogNameMap.insert(std::make_pair(ELogLevel::BG_EXTRACT_DATA_1, "EXTRACT_DATA_1"));
	 * 2번
	 * pFileStream = new std::fstream;
	 * m_logFileStreamVec.push_back(std::make_pair("extract_data_1", pFileStream));
	 * m_forderNameFileStreamMap.insert(std::make_pair("extract_data_1", pFileStream));
	 * 3번
	 * m_logLevelForderNameMap.insert(std::make_pair(ELogLevel::BG_EXTRACT_DATA_1, "extract_data_1"));
	*/
	return true;
}

/**
 * 이 함수를 호출하면 로그시스템이 시작합니다.
 * 로그를 찍는 전용 스레드를 만들어 동작
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
 * 이 함수를 호출하면 로그시스템이 종료합니다.
 * 로그 시스템 종료 요청 로그를 전용스레드에 보낸후에
 * 로그전용 스레드가 확인하면, 스레드가 종료된 것을 확인한 뒤에
 * true를 리턴합니다.
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
* 로그 전용 스레드 함수 입니다.
* queue에서 로그를 꺼내 기록합니다.
* 로그 종료 요청이 있을때가지 동작합니다.
* Stop함수를 호출하면 해당 함수는 종료됩니다.
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

		// 로그 쓰기
		pLogMgr->Write(log);
	}

	return;
}

/**
 * 로그 시스템이 동작종료를 요청하는 로그가 들어왔는지 확인합니다.
 * 종료 요청 로그는 [로그레벨 : INFO, 내용 : "STOP"] 입니다.
 * 로그 전용 스레드에서만 호출 합니다.
 * true를 리턴하면, 로그 전용 스레드는 종료됩니다.
*/
bool BGLogManager::IsStopRequest(BGLog& log)
{
	return ELogLevel::BG_LOG_SYSTEM_EXIT == log.GetLevel();
}

/**
* Queue에서 로그를 꺼냅니다.
* 꺼낸 로그는 Valid를 호출해서
* 유효한 로그인지 확인한 후에 사용합니다.
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
* 로그를 알맞은 파일에 기록합니다.
* 특정 로그파일이 없으면 디폴트 파일에 기록합니다.
* 디폴트 파일은 log폴더에 기록하고, m_logfileStreamVec 0번째 인덱스에 존재합니다.
*/
void BGLogManager::Write(BGLog &log)
{
	if (CheckLogFileNameAndRenew()) {
		RenewLogFileStream();
	}

	std::fstream* pDefaultFileStream{ nullptr };
	std::fstream* pSpecificFileStream{ nullptr };

	// 무조건 쓰는 기본 파일
	LogLevel_StringMap::iterator forderNameIter = m_logForderNameMap.find(ELogLevel::BG_NONE);
	if (forderNameIter != m_logForderNameMap.end()) {
		std::string defaultForderName = forderNameIter->second;
		String_FileStreamMap::iterator fstreamIter = m_forderNameFileStreamMap.find((defaultForderName));
		pDefaultFileStream = fstreamIter->second;
	}	

	// 특정 로그 레벨은 특정 폴더
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
* 파일이름은 시간단위로 갱신됩니다.
* 파일이 갱신되면 true를 리턴합니다.
* 기존 파일을 그대로 사용할 수 있으면 false를 리턴합니다.
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
* 기존 파일스트림을 닫고,
* 최신 파일 이름에 맞게 새로운 파일스트림을 생성합니다.
* CheckLogFileNameAndRenew()함수가 false를 리턴하면 해당함수를 호출합니다.
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
 * 기록해야 하는 로그인지 로그레벨로 검사합니다.
 * 현재 설정 된 로그레벨 이상일때 실제 로그를 기록합니다.
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
 * 로그를 시간, 레벨, 내용, 호출한 함수로 조합하여 Queue에 넣습니다.
 * 이후 전용스레드에서 하나씩 꺼내 처리합니다.
*/
void BGLogManager::PushLog(ELogLevel level, char* func_name, char* msg, ...)
{
	if (!CheckLogLevel(level))
		return;

	struct tm ltm;
	time_t t = time(NULL);
	localtime_s(&ltm, &t);


	// 로그 내용
	char tmp[4000] = "";	
	char result[4096] = "";
	va_list args;
	va_start(args, msg);
	vsprintf_s(tmp, _countof(tmp), msg, args);
	va_end(args);

	// 로그 이름
	std::string logName{ "[" };
	auto name = m_logPrefixFilNameMap.find(level);
	if (name == m_logPrefixFilNameMap.end()) {
		BG_LOG_ERROR("logLevelLogNameMap.find() Failed! [logLevel=%d]", static_cast<int>(level));
		return;
	}
	logName.append(name->second);
	logName.append("]");

	// 로그 시간
	char time_tmp[100] = "";
	sprintf_s(time_tmp, "[%02d:%02d:%02d]", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
	std::string logTime{ time_tmp };

	// 로그 발생한 함수
	std::string logFunc{ "(" };
	logFunc.append(func_name);
	logFunc.append(")");

	std::string logContents{ tmp };
	logContents.append(" - ");
	logContents.append(logFunc);

	// 조합
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


