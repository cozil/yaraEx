#include "command_yaraEx.h"
#include "strutils.h"
#include "misc.h"
#include "FileHelper.h"
#include <algorithm>
#include "Handle.h"

#define ARGUMENT_PATTERN	"([@#!])([a-zA-Z_][\\w]+)(?:\\[([\\d]+)\\])?"
#define LOG_ERROR		1
#define LOG_WARNING		2
#define LOG_INFO		3
#define LOG_DEBUG		4

yaraEx::yaraEx()
	:m_yrCompiler(nullptr)
	, m_yrRules(nullptr)
	, m_base(0)
	, m_debug(false)
	, m_re(nullptr)
	, m_match(nullptr)
	, m_matchCount(0)
	, m_logLevel(LOG_INFO)
	, m_dbgPuased(false)

{
	duint value;
	if (BridgeSettingGetUint("Engine", "YaraDebug", &value))
		m_debug = !!value;

	if (DbgIsRunning())
	{
		m_dbgPuased = true;
		Script::Debug::Pause();
	}
}



yaraEx::~yaraEx()
{
	cleanup();
	if (m_dbgPuased)
		Script::Debug::Run();
}

//void yaraEx::logprintf(int level, const char* format, ...)
//{
//	if (!m_debug && m_logLevel < level)
//		return;
//
//	va_list ap;
//	va_start(ap, format);
//	std::string msg = string_formatA("[%s] ",m_cmdname.c_str()) + string_vformatA(format, ap);
//	va_end(ap);
//	::logputs(msg.c_str());
//}
//
//void yaraEx::logputs(int level, const char * text)
//{
//	if (!m_debug && m_logLevel < level)
//		return;
//
//	std::string msg = string_formatA("[%s] ", m_cmdname.c_str()) + std::string(text);
//	::logputs(msg.c_str());
//}

void yaraEx::cleanup()
{
	if (m_yrRules)
	{
		yr_rules_destroy(m_yrRules);
		m_yrRules = nullptr;
	}

	if (m_yrCompiler)
	{
		yr_compiler_destroy(m_yrCompiler);
		m_yrCompiler = nullptr;
	}

	if (m_match)
	{
		pcre2_match_data_free(m_match);
		m_match = nullptr;
	}

	if (m_re)
	{
		pcre2_code_free(m_re);
		m_re = nullptr;
	}

	m_data.free();
	m_results.clear();
	m_rulesIdentifiers.clear();
	m_matchCount = 0;
	m_base = 0;
}


bool yaraEx::initModule(const char * mod, const char * size)
{
	bool rawFile = false;
	duint modsize = 0;
	SELECTIONDATA sel;
	GuiSelectionGet(GUI_DISASSEMBLY, &sel);
	m_base = sel.start;	

	if (m_base = Script::Module::BaseFromName(mod))
	{
		modsize = Script::Module::SizeFromAddr(m_base);
		rawFile = size && *size == '1';
	}
	else
	{
		if (!Script::Misc::ParseExpression(mod, &m_base))
		{
			logprintf(LL::Error, "Invalid expression \"%s\"!", mod);
			return false;
		}

		if (size)
		{
			if (!Script::Misc::ParseExpression(size, &modsize))
				modsize = 0;
		}

		if (!modsize)
			m_base = DbgMemFindBaseAddr(m_base, &modsize);
	}

	std::vector<unsigned char> rawFileData;
	if (rawFile) //read the file from disk
	{
		char modPath[MAX_PATH] = "";
		if (!Script::Module::PathFromAddr(m_base, modPath))
		{
			logprintf(LL::Error, "Failed to get module path for %p!", m_base);
			return false;
		}

		if (!FileHelper::ReadAllData(modPath, rawFileData))
		{
			logprintf(LL::Error, "Failed to read file \"%s\"!", modPath);
			return false;
		}
		modsize = rawFileData.size();
	}

	m_data.realloc(modsize);
	if (rawFile)
	{
		memcpy(m_data(), rawFileData.data(), modsize);
	}
	else
	{
		memset(m_data(), 0xCC, m_data.size());
		DbgMemRead(m_base, m_data(), modsize);
	}

	return true;
}

bool yaraEx::initPattern(const char * pattern)
{
	//compile pcre2 pattern
	int error_code = 0;
	PCRE2_SIZE error_offset = 0;

	m_re = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(pattern),
		PCRE2_ZERO_TERMINATED, 0, &error_code, &error_offset, NULL);
	if (!m_re)
	{
		char text[1024];
		pcre2_get_error_message(error_code, reinterpret_cast<PCRE2_UCHAR*>(text), sizeof(text));
		logprintf(LL::Error, "Failed to compile pcre pattern: %s", text);
		return false;
	}

	m_match = pcre2_match_data_create_from_pattern(m_re, NULL);
	if (!m_match)
	{
		logputs(LL::Error, "Failed to create pcre match data");
		return false;
	}

	return true;
}

bool yaraEx::initYara(const std::string& content, bool isfile)
{
	if (yr_compiler_create(&m_yrCompiler) != ERROR_SUCCESS)
	{
		logputs(LL::Error, "yr_compiler_create failed!");
		return false;
	}

	yr_compiler_set_callback(m_yrCompiler, &yaraEx::compilerCallback, this);
	
	std::string rulesContent;
	if (isfile)
	{		
		rulesContent = string_formatA("include \"%s\"", content.c_str());

		//Fetch path
		auto pos = content.find_last_of('/');
		if (pos == std::string::npos)
			pos = content.find_last_of('\\');

		if (pos != std::string::npos)
		{
			m_filePath = content.substr(0, pos + 1);
		}
	}
	else
	{
		rulesContent = content;
	}

	if (yr_compiler_add_string(m_yrCompiler, rulesContent.c_str(), nullptr) != 0)
	{
		logputs(LL::Error, "Errors in the rules text!");
		return false;
	}

	if (yr_compiler_get_rules(m_yrCompiler, &m_yrRules) != ERROR_SUCCESS)
	{
		logputs(LL::Error, "Error while getting the rules!");
		return false;
	}

	//yara库版本过低，不支持下面的函数
	//yr_compiler_set_include_callback()

	return true;
}

void yaraEx::parseCommand(Cmd& cmd)
{
	int rc = 0;
	size_t start_offset = 0;
	while ((rc = pcre2_match(m_re, reinterpret_cast<PCRE2_SPTR>(cmd.fulltext.c_str()), cmd.fulltext.length(), start_offset, 0, m_match, NULL)) > 0)
	{
		PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_match);
		Var var;
		if (rc >= 3)
		{
			var.fulltext = cmd.fulltext.substr(ovector[0], ovector[1] - ovector[0]);
			var.prefix = cmd.fulltext.substr(ovector[2], ovector[3] - ovector[2]);
			var.identifier = cmd.fulltext.substr(ovector[4], ovector[5] - ovector[4]);
			var.slot = 0;
			if (rc >= 4)
				var.slot = atoi(cmd.fulltext.substr(ovector[6], ovector[7] - ovector[6]).c_str());

			//如果是#前缀，表示取匹配数量，不应是一个数组。需要剔除[]内容
			if (var.prefix == "#")
			{
				std::string::size_type pos = var.fulltext.find_first_of('[');
				if (pos != std::string::npos)
				{
					std::string s = var.fulltext.substr(0, pos);
					var.fulltext = s;
				}
			}
		}

		if (var.identifier.length() != 0)
			cmd.varlist.push_back(var);

		start_offset = ovector[2 * (rc - 1) + 1];
	}
}

bool yaraEx::initRules(bool scanMeta)
{
	YR_RULE * yrRule;
	m_results.clear();

	
	//yr_rules_foreach(m_yrRules, yrRule)
	for (yrRule = m_yrRules->rules_list_head; !RULE_IS_NULL(yrRule); yrRule++)
	{
		if (!yrRule->identifier || !*yrRule->identifier)
			continue;

		YaraRule rule;
		if (scanMeta)
		{
			YR_META * yrMeta;
			//yr_rule_metas_foreach(yrRule, yrMeta)
			for (yrMeta = yrRule->metas; !META_IS_NULL(yrMeta); yrMeta++)
			{
				if (yrMeta->type != META_TYPE_STRING || !yrMeta->string)
					continue;

				if (!_stricmp(yrMeta->identifier, "script"))
				{
					Cmd cmd;
					cmd.fulltext = yrMeta->string;
					parseCommand(cmd);
					rule.cmds.push_back(std::move(cmd));
				}
				else if (!_stricmp(yrMeta->identifier, "load"))
				{
					std::vector<std::string> lines;
					std::string file = trim_leftA(std::string(yrMeta->string));
					if (file.length() == 0)
						continue;

					//Not a full path
					if (file.find(':') == std::string::npos)
					{
						if (file[0] == '\\' || file[0] == '/')
							file.erase(0, 1);
						file = m_filePath + file;
					}

					if (!FileHelper::ReadAllLines(file, lines))
					{
						logprintf(LL::Error, "Rule \"%s\": Failed to read the script file \"%s\"",
							yrRule->identifier, yrMeta->string);
						return false;
					}

					for (const std::string& line : lines)
					{
						Cmd cmd;
						cmd.fulltext = line;
						trim_stringA(cmd.fulltext);
						if (cmd.fulltext.length())
						{
							parseCommand(cmd);
							rule.cmds.push_back(std::move(cmd));
						}
					}
				}
			}
		}

		rule.matched = false;
		m_results[std::string(yrRule->identifier)] = std::move(rule);
		m_rulesIdentifiers.push_back(std::string(yrRule->identifier));
	}

	return true;
}

bool yaraEx::doScan()
{
	bool bSuccess = false;
	duint ticks = GetTickCount();
	int err = yr_rules_scan_mem(m_yrRules, m_data(), m_data.size(), 0, &yaraEx::scanCallback, this, 0);
	switch (err)
	{
	case ERROR_SUCCESS:
		logprintf(LL::Message, "%u scan results in %ums...", m_matchCount, GetTickCount() - DWORD(ticks));
		bSuccess = true;
		break;
	case ERROR_TOO_MANY_MATCHES:
		logputs(LL::Error, "Too many matches!");
		break;
	default:
		logputs(LL::Error, "Error while scanning memory!");
		break;
	}

	return bSuccess;
}

void yaraEx::execScript()
{
	//按m_rulesIdentifiers中的顺序执行rule中的脚本命令
	for (const std::string& identifier : m_rulesIdentifiers)
	{
		auto itResult = m_results.find(identifier);
		if (itResult == m_results.cend())
			continue;
		
		const YaraRule& rule = itResult->second;
		if (!rule.matched)
			continue;

		std::vector<std::string> script;
		for (auto& cmd : rule.cmds)
		{
			std::string cmdtext = cmd.fulltext;
			for (const auto& var : cmd.varlist)
			{
				auto itMatches = rule.matches.find(var.identifier);
				if (itMatches == rule.matches.end())
					continue;

				duint value = 0;
				bool replace = true;
				const std::vector<YaraMatch>& matches = itMatches->second;
				std::string::size_type pos = cmdtext.find(var.fulltext.c_str(), 0);
				if (pos == std::string::npos)
					continue;

				//当一个rule有多个strings存在时，没有匹配成功的string，其对应的
				//matches数组为空。

				int slot = var.slot;
				if (slot >= 1) slot -= 1;
				if (slot >= (int)matches.size()) slot = -1;

				if (var.prefix == "#")
				{		
					value = matches.size();
				}
				else if (var.prefix == "@")
				{
					if (slot >= 0)
						value = m_base + matches[slot].offset;
				}
				else if (var.prefix == "!")
				{
					if (slot >= 0)
						value = matches[slot].match_length;
				}
				else
				{
					replace = false;
				}

				if (replace)
				{
					cmdtext.replace(pos, var.fulltext.length(), string_formatA("0x%x", value));
				}
			}

			script.push_back(std::move(cmdtext));
		}

		std::string scriptfile = string_formatA("%s_%s.compile.scr", m_filePath.c_str(), identifier.c_str());			
		std::string content;

		auto str_replace = [](std::string& str, const std::string& find, const char * replacedby)
		{
			auto pos = str.find(find.c_str());
			while (pos != std::string::npos)
			{
				str.replace(pos, find.length(), replacedby);
				pos = str.find(find.c_str());
			}
		};

		for (const std::string& cmd : script)
		{
			std::string _cmd = cmd;
			str_replace(_cmd, "\r", "\\r");
			str_replace(_cmd, "\n", "\\n");
			content += _cmd + "\n";
		}

		//meta中的 log "\n\n" 是由yara读取文件时进行转义的
		//写入脚本文件时会输出两行，而反转义之后，log只会输出\n\n而不是两个空行

		if (!FileHelper::WriteAllText(scriptfile, content))
		{
			logprintf(LL::Error, "Failed to write script file \"%s\".", scriptfile.c_str());
		}
		else
		{ 
			DbgScriptLoad(scriptfile.c_str());
			remove(scriptfile.c_str());
			DbgScriptRun(0);
		}
	}

	DbgScriptUnload();
}

void yaraEx::compilerCallback(int error_level, const char* file_name, int line_number, const char* message, void* user_data)
{
	yaraEx * lpThis = (yaraEx *)user_data;
	std::string text;
	
	if (file_name)
		text = string_formatA("File: \"%s\", Line: %d, Message: \"%s\"", file_name, line_number, message);
	else
		text = string_formatA("Line: %d, Message: \"%s\"", line_number, message);

	switch (error_level)
	{
	case YARA_ERROR_LEVEL_ERROR:
		logputs(LL::Error, text.c_str());
		break;

	case YARA_ERROR_LEVEL_WARNING:
		logputs(LL::Warning, text.c_str());
		break;
	}	
}

duint yaraEx::saveMatches(YR_RULE * yrRule)
{
	YR_STRING * string;
	auto itRule = m_results.find(std::string(yrRule->identifier));
	if (itRule == m_results.end())
	{
		logprintf(LL::Error, "Rule \"%s\" doesn't initialized!", yrRule->identifier);
		return 0;
	}

	duint matchCount = 0;
	itRule->second.matched = true;
	if (STRING_IS_NULL(yrRule->strings))
	{
	}
	else
	{
		yr_rule_strings_foreach(yrRule, string)
		{
			YR_MATCH * yrMatch;
			std::vector<YaraMatch> matches;
			yr_string_matches_foreach(string, yrMatch)
			{
				YaraMatch match;
				match.offset = (duint)yrMatch->offset;
				match.data_length = yrMatch->data_length;
				match.match_length = yrMatch->match_length;
				matches.push_back(std::move(match));
				++matchCount;
			}

			std::string identifier = string->identifier;
			identifier.erase(0, 1);	//删除yara变量前缀'$'
			itRule->second.matches[identifier] = std::move(matches);

		}
	}

	m_matchCount += matchCount;
	return matchCount;
}


int yaraEx::scanCallback(int message, void* message_data, void* user_data)
{
	yaraEx * lpThis = (yaraEx*)user_data;
	YR_RULE * yrRule = (YR_RULE *)message_data;

	switch (message)
	{
	case CALLBACK_MSG_RULE_MATCHING:
	{
		duint matchCount = lpThis->saveMatches(yrRule);
		logprintf(LL::Debug, "Rule \"%s\" found %d matches!", yrRule->identifier, matchCount);
	}
	break;

	case CALLBACK_MSG_RULE_NOT_MATCHING:
	{
		logprintf(LL::Warning, "Rule \"%s\" didn't match!", yrRule->identifier);
	}
	break;

	//case CALLBACK_MSG_SCAN_FINISHED:
	//{
	//	lpThis->logdebugmsg("Scan finished!");
	//}
	//break;

	//case CALLBACK_MSG_IMPORT_MODULE:
	//{
	//	YR_MODULE_IMPORT* yrModuleImport = (YR_MODULE_IMPORT*)message_data;
	//	lpThis->logdebugmsg(string_formatA("Imported module \"%s\"!", yrModuleImport->module_name));
	//}
	break;
	}
	
	return CALLBACK_CONTINUE;
}

bool yaraEx::cmd_yaraEx(int argc, char *argv[])
{
	cleanup();
	m_cmdname = CMD_NAME_YARAEX;
	m_logLevel = LOG_INFO;

	if (!initModule(argv[2], argc > 3 ? argv[3] : nullptr))
		return false;

	if (!initPattern(ARGUMENT_PATTERN))
		return false;

	if (!initYara(argv[1], true))
		return false;

	if (!initRules(true))
		return false;

	bool bSuccess = doScan();
	if (bSuccess)
		execScript();

	return bSuccess;
}

bool yaraEx::_yarafind(int argc, char * argv[])
{
	cleanup();

	if (!initModule(argv[1], argc > 3 ? argv[3] : nullptr))
		return false;

	std::string yaraRule = string_formatA(
		"rule %s {\n"
		"	strings:\n"
		"		$pattern = { %s }\n"
		"	condition:\n"
		"		all of them\n"
		"}", m_cmdname.c_str(), argv[2]);

	if (!initYara(yaraRule.c_str(), false))
		return false;

	if (!initRules(false))
		return false;

	bool bSuccess = doScan();
	duint address = 0;
	size_t count = 0;

	if (bSuccess)
	{
		//logputs(LOG_INFO, "Preparing gui reference view!");

		auto itMatches = m_results.find(m_cmdname);
		if (itMatches != m_results.end())
		{
			//logputs(LOG_INFO, "Found result!");

			const YaraRule& rule = itMatches->second;
			if (rule.matched)
			{
				//logputs(LOG_INFO, "Found match!");
				if (rule.matches.size() != 0)
				{
					const std::vector<YaraMatch>& matches = rule.matches.cbegin()->second;
					count = matches.size();
					if (count)
						address = (duint)matches[0].offset + m_base;

					//logprintf(LOG_INFO, "match result: address = %p, count = %d", address, count);
				}
			}
		}

		DbgScriptCmdExec(string_formatA("$result=0x%x", count).c_str());
		DbgScriptCmdExec(string_formatA("$result1=0x%x", address).c_str());
	}

	return bSuccess;

}

bool yaraEx::cmd_yarafind(int argc, char * argv[])
{
	m_logLevel = LOG_ERROR;
	m_cmdname = CMD_NAME_YARAFIND;
	return _yarafind(argc, argv);
}

bool yaraEx::cmd_yarafindall(int argc, char *argv[])
{
	m_logLevel = LOG_INFO;
	m_cmdname = CMD_NAME_YARAFINDALL;
	bool bSuccess = _yarafind(argc, argv);
	if (bSuccess)
	{
		std::string pattern = argv[2];
		if (pattern.length() > 16)
		{
			pattern.erase(16, pattern.length() - 16);
			pattern += "...";
		}

		bool findData = false;
		if (argc > 3)
		{
			if (!_stricmp(argv[3], "&data&"))
				findData = true;
		}

		GuiReferenceInitialize(string_formatA(GuiTranslateText("Pattern: %s"), pattern.c_str()).c_str());
		GuiReferenceAddColumn(2 * sizeof(duint), GuiTranslateText("Address"));
		if (findData)
			GuiReferenceAddColumn(10, GuiTranslateText("Data"));
		else
			GuiReferenceAddColumn(10, GuiTranslateText("Disassembly"));
		GuiReferenceSetRowCount(0);
		GuiReferenceReloadData();

		int index = 0;
		for (auto& pairRule : m_results)
		{
			const YaraRule& rule = pairRule.second;
			if (!rule.matched)
				continue;

			for (auto& pairMatch : rule.matches)
			{
				const std::vector<YaraMatch>& matches = pairMatch.second;
				for (auto match : matches)
				{
		
					std::string msg;
					GuiReferenceSetRowCount(index + 1);
					GuiReferenceSetCellContent(index, 0, string_formatA("%p", m_base + match.offset).c_str());

					if (findData)
					{
						Memory<unsigned char *> data(min(80, match.match_length));
						DbgMemRead(m_base + (duint)match.offset, data(), data.size());
						msg = to_hexstringA(data(), data.size(), 0, true, ' ');
						if (match.match_length > 80)
							msg += "...";
					}
					else
					{
						char text[1024];
						if (!GuiGetDisassembly(m_base + (duint)match.offset, text))
							msg = GuiTranslateText("[Error disassembling]");
						else
							msg = text;						
					}

					GuiReferenceSetCellContent(index, 1, msg.c_str());
					++index;
				}
			}
		}
	}

	return bSuccess;
}