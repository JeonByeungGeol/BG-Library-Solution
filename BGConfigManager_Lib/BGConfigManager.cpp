
#include "BGConfigManager.h"


bool BGConfigManager::Load(std::string fileName)
{
	std::fstream fs;
	fs.open(fileName, std::ios::in);

	if (!fs.is_open())
		return false;

	Key k;
	Value v;

	while (fs >> k) {
		if (k == "//") {
			std::getline(fs, v);
			continue;
		}
		
		fs >> v;

		m_ConfigMap.insert(std::make_pair(k, v));
	}

	return true;
}

std::string BGConfigManager::GetString(Key k)
{
	auto iter = m_ConfigMap.find(k);
	if (iter == m_ConfigMap.end()) {
		return std::string();
	}
		
	return iter->second;
}

int BGConfigManager::GetInt(Key k)
{
	Value v = GetString(k);
	if (v.empty())
		return 0;
	
	return std::stoi(v);	
}

double BGConfigManager::GetDouble(Key k)
{
	Value v = GetString(k);
	if (v.empty())
		return 0.0;
		
	return std::stod(v);
}

std::once_flag BGConfigManager::onceflag_;
std::unique_ptr<BGConfigManager> BGConfigManager::instance_{ nullptr };

BGConfigManager* BGConfigManager::Instance() {
	std::call_once(onceflag_, []() {
		instance_.reset(new BGConfigManager);
	});
	return instance_.get();
}

