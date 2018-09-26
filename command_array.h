#pragma once
#include "pluginmain.h"
#include "strutils.h"

class CArrayHelper
{
public:
	typedef std::unordered_map<duint, duint> Items;
	typedef std::unordered_map<std::string, Items> Array;
public:
	CArrayHelper();
	~CArrayHelper();

	bool cmd_array_set(int argc, char* argv[]);
	bool cmd_array_get(int argc, char* argv[]);
	bool cmd_array_remove(int argc, char* argv[]);
	bool cmd_array_removeAll(int argc, char* argv[]);



private:
	Array m_array;
};

