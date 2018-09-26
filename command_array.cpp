#include "command_array.h"
#include "misc.h"
#include <algorithm>

CArrayHelper::CArrayHelper()
{
}


CArrayHelper::~CArrayHelper()
{
}

bool CArrayHelper::cmd_array_set(int argc, char* argv[])
{
	std::string name = trim_string(argv[1]);
	duint key, value;

	if (name.size() == 0)
	{
		logprintf(LL::Error, "Invalid argument name/key!");
		return false;
	}

	if (!eval(argv[2], key) || !eval(argv[3], value))
		return false;

	auto itrArray = m_array.find(name);
	if (itrArray != m_array.end())
	{
		itrArray->second.insert({ key, value });
	}
	else
	{
		Items i;
		i.insert({ key, value });
		m_array.insert({ name, i });
	}
	logprintf(LL::Message, "%s[.%d] = 0x%x", name.c_str(), key, value);
	return true;	
}

bool CArrayHelper::cmd_array_get(int argc, char* argv[])
{
	std::string name = trim_string(argv[1]);
	duint key;

	if (argc > 2 && !eval(argv[2], key))
		return false;

	auto itrArray = m_array.find(name);
	if (itrArray != m_array.cend())
	{
		const Items& items = itrArray->second;
		if (argc > 2)
		{
			auto itrItem = items.find(key);
			if (itrItem == items.cend())
			{
				logprintf(LL::Warning, "%s[.%d] doesn't exists!", name.c_str(), key);
			}
			else
			{
				logprintf(LL::Message, "%s[.%d] = 0x%x", name.c_str(), key, itrItem->second);
			}

			return DbgCmdExecDirect(string_formatA("$RESULT=0x%x", itrItem->second).c_str());
		}
		else
		{
			logprintf(LL::Message, "Array \"%s\":", name.c_str());
			for (const auto& pair : items)
			{
				logprintf(LL::Message, "  \".%d\" => \"0x%x\"", pair.first, pair.second);
			}
		}
	}
	else
	{
		logprintf(LL::Warning, "Array \"%s\" doesn't exists!", name.c_str());
	}
	return true;
}

bool CArrayHelper::cmd_array_remove(int argc, char* argv[])
{
	std::string name = trim_string(argv[1]);
	auto itrArray = m_array.find(name);
	if (itrArray != m_array.cend())
	{
		m_array.erase(itrArray);
		logprintf(LL::Message, "Array \"%s\" has been removed!", name.c_str());
	}
	else
	{
		logprintf(LL::Warning, "Array \"%s\" doesn't exists!", name.c_str());
	}
	return true;
}

bool CArrayHelper::cmd_array_removeAll(int argc, char* argv[])
{
	size_t count = m_array.size();
	m_array.clear();
	logprintf(LL::Warning, "All %d arrays have been removed!", count);
	return true;
}

