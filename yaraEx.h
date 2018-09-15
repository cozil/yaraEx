#pragma once
#include "pluginmain.h"
#include <string>
#include <map>
#include <unordered_map>
#include "Memory.h"

class yaraEx
{
public:

	struct Var
	{
		std::string fulltext;		//变量完整内容
		std::string prefix;			//变量前缀 #:匹配数量，@:匹配地址，!:匹配地址
		std::string identifier;		//变量名称		
		int slot;					//数组索引 如果为0则默认为第一个匹配

		Var& operator=(const Var& v)
		{
			fulltext = v.fulltext;
			identifier = v.identifier;
			prefix = v.prefix;
			slot = v.slot;
			return *this;
		}
	};

	struct Cmd
	{
		std::string fulltext;		//命令完整内容
		std::vector<Var> varlist;	//需要替换的变量列表

		Cmd& operator=(const Cmd& c)
		{
			fulltext = c.fulltext;
			varlist = c.varlist;
			return *this;
		}
	};

	struct YaraMatch
	{
		duint offset;
		int data_length;
		int match_length;
	};

	struct YaraRule
	{
		bool matched;

		//Key : string identifier
		//Value : matched results
		std::unordered_map<std::string, std::vector<YaraMatch> > matches;

		//cmd list Extracted from meta data, whose identifier is "script" 
		//or prefixed with "script_"
		std::vector<Cmd> cmds;

		YaraRule& operator=(const YaraRule& yr)
		{
			matched = yr.matched;
			matches = yr.matches;
			cmds = yr.cmds;
			return *this;
		}

		YaraRule& operator=(YaraRule&& yr)
		{
			matched = yr.matched;
			matches = yr.matches;
			cmds = yr.cmds;
			return *this;
		}
	};


	yaraEx();
	~yaraEx();

	void logprintf(int level, const char* format, ...);
	void logputs(int level, const char * text);
	//void logdebugmsg(const std::string& text);

	//yaraEx
	bool cmd_yaraEx(int argc, char* argv[]);

	//yarafind
	bool cmd_yarafind(int argc, char * argv[]);

	//yarafindall
	bool cmd_yarafindall(int argc, char *argv[]);

private:
	static void compilerCallback(int error_level, const char* file_name, int line_number, const char* message, void* user_data);
	static int scanCallback(int message, void* message_data, void* user_data);

	void cleanup();
	bool initYara(const std::string& content, bool isfile);
	bool initRules(bool scanMeta);
	bool initModule(const char * mod, const char * size);
	bool initPattern(const char * pattern);
	bool doScan();
	duint saveMatches(YR_RULE * yrRule);
	void execScript();
	void parseCommand(Cmd& cmd);

	bool _yarafind(int argc, char * argv[]);

private:
	std::string m_cmdname;
	int m_logLevel;
	bool m_dbgPuased;

	//pcre2 objects
	pcre2_code * m_re;
	pcre2_match_data * m_match;

	//yara objects
	YR_COMPILER * m_yrCompiler;
	YR_RULES * m_yrRules;

	//arguments
	duint m_base;
	bool m_debug;
	std::string m_filePath;

	//intermediate runtime datas
	duint m_matchCount;
	Memory<uint8_t*> m_data;
	std::unordered_map<std::string, YaraRule> m_results;
	std::vector<std::string> m_rulesIdentifiers;
};

