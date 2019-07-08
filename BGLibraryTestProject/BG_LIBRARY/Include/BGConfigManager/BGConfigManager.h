#pragma once

#include <string>
#include <fstream>
#include <map>
#include <mutex>

/**
 * <pre>
 * ���� ������ ����
 * </pre>
*/

class BGConfigManager
{
	using Key = std::string;
	using Value = std::string;
	using ConfigMap = std::map<Key, Value>;
	
public:
	// config.ini ������ �н��ϴ�.
	bool Load(std::string fileName);
	
	// config ���� �о� �ɴϴ�.
	std::string GetString(Key);
	int GetInt(Key);
	double GetDouble(Key);

private:
	// config ������ �����մϴ�.
	ConfigMap m_ConfigMap;


	//////////////////////////////////////////////////
	/** �̱��� ���� ��*/
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
