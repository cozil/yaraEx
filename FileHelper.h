#pragma once
#include <string>
#include <vector>

class FileHelper
{
public:
	static bool ReadAllData(const std::string & fileName, std::vector<unsigned char> & content);
	static bool WriteAllData(const std::string & fileName, const void* data, size_t size);
	static bool ReadAllText(const std::string & fileName, std::string & content);
	static bool WriteAllText(const std::string & fileName, const std::string & content);
	static bool ReadAllLines(const std::string & fileName, std::vector<std::string> & lines, bool keepEmpty = false);
	static std::string GetFileName(const std::string & fileName);
};

