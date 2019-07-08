#pragma once

#include <string>
#include <fstream>
#include <map>
#include <mutex>

/**
 * <pre>
 * 설정 값들을 관리
 * </pre>
*/

class BGConfigManager
{
	using Key = std::string;
	using Value = std::string;
	using ConfigMap = std::map<Key, Value>;
	
public:
	// config.ini 파일을 읽습니다.
	bool Load(std::string fileName);
	
	// config 값을 읽어 옵니다.
	std::string GetString(Key);
	int GetInt(Key);
	double GetDouble(Key);

private:
	// config 값들을 저장합니다.
	ConfigMap m_ConfigMap;


	//////////////////////////////////////////////////
	/** 싱글톤 구현 부*/
private:
	static std::once_flag onceflag_;
	static std::unique_ptr<BGConfigManager> instance_;

protected:
	BGConfigManager() = default;
	BGConfigManager(const BGConfigManager&) = delete;
	BGConfigManager& operator=(const BGConfigManager&) = delete;

public:
	static BGConfigManager* Instance();
	//////////////////////////////////////////////////
};

#define g_ConfigManager (*BGConfigManager::Instance())
