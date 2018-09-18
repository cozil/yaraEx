#include "misc.h"
#include "pluginmain.h"
#include "strutils.h"

void logprintf(const char * cmd_name, const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	std::string msg = string_formatA("[" PLUGIN_NAME "] [%s] ", cmd_name) + string_vformatA(format, ap);
	va_end(ap);
	_plugin_logputs(msg.c_str());
}

void logputs(const char * text)
{
	std::string msg = "[" PLUGIN_NAME "] " + std::string(text);
	_plugin_logputs(msg.c_str());
}

void print_usages(const char * cmdname)
{
	if (!cmdname || !*cmdname)
		return;

	if (!compare_stringA(std::string(cmdname), std::string(CMD_NAME_YARAEX), true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_YARAEX " <arg1>, [arg2], [arg3]\n"
			"\t<arg1>: Rules file to apply. This should be a full path.\n"
			"\t[arg2]: Start address of the range to apply the rules to. If not specified, the disassembly selection will be used.\n"
			"\t[arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n\n"
			"Remarks:\n"
			"\tTo run a single command, define it in metadata section whose identifer must be \"script\".\n"
			"\tTo run script file, define it in metadata section whose identifer must be \"load\".\n"
			"\tSupport multiple \"script\" or \"load\" definitions. Running sequences rely on their defined orders.\n"
			"\tYara variables like #pattern, @pattern[n], !pattern[n] will be automatically replaced to their real values\n"
			"\tin the command text or the script file before running.\n\n"
			"\tThe script file path can be relative to the directory where the rules file resides.\n"
			"\tAll commands including the script file will be put together to run as one script file through DbgScriptLoad function.\n"
			"\t\\r and \\n will be unescaped in the command text defined by \"script\" before writing into script file.\n\n"
			"Sample:\n"
			"\trule test {\n"
			"\t\tmeta:\n"
			"\t\t\tscript=\"log \\\"Hello yaraEx!\\\"\"\n"
			"\t\t\tload=\"d:\\yaraEx.yar\"\n"
			"\t\t...\n"
			"\t}\n");
	}
	else if (!compare_stringA(std::string(cmdname), std::string(CMD_NAME_YARAFIND), true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_YARAFIND " <arg1>, [arg2], [arg3]\n"
			"\t<arg1>: The address to start searching from.\n"
			"\t<arg2>: The byte pattern of yara to search for.\n"
			"\t[arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n"
			"The $result is set to the number of occurrences.\n"
			"The $result1 variable is set to the virtual address of the first address that matches the byte pattern.\n");
	}
	else if (!compare_stringA(std::string(cmdname), std::string(CMD_NAME_YARAFINDALL), true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_YARAFINDALL " <arg1>, [arg2], [arg3]\n"
			"\t<arg1>: The address to start searching from.\n"
			"\t<arg2>: The byte pattern of yara to search for.\n"
			"\t[arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n"
			"The $result is set to the number of occurrences.\n"
			"The $result1 variable is set to the virtual address of the first address that matches the byte pattern.\n"
			"All results will be shown in the gui reference view.\n");
	}
}