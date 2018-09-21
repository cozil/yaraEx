#include "misc.h"
#include "pluginmain.h"
#include "strutils.h"

static LL _log_level = LL::Debug;

void set_log_level(LL level)
{
	_log_level = level;
}

void logprintf(LL level, const char * format, ...)
{
	if (level <= _log_level)
	{
		va_list ap;
		va_start(ap, format);
		std::string msg = string_formatA("[" PLUGIN_NAME "] %s\n", string_vformatA(format, ap).c_str());
		va_end(ap);
		_plugin_logputs(msg.c_str());
	}
}

void logputs(LL level, const char * text)
{
	if (level <= _log_level)
	{
		std::string msg = string_formatA("[" PLUGIN_NAME "] %s\n", text);
		_plugin_logputs(msg.c_str());
	}
}

void print_usages(const char * cmdname)
{
	if (!cmdname || !*cmdname)
		return;

	if (!compare_stringA(cmdname, CMD_NAME_YARAEX_LL, true))
	{
		_plugin_logputs(
			"Usage:\n" CMD_NAME_YARAEX_LL "<arg1>\n"
			"  <arg1>: Log level for outputing message(0 - Error, 1 - Warning, 2 - Normal message, 3 - Debug message");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_YARAEX, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_YARAEX " <arg1>, [arg2], [arg3]\n"
			"  <arg1>: Rules file to apply. This should be a full path.\n"
			"  [arg2]: Start address of the range to apply the rules to. If not specified, the disassembly selection will be used.\n"
			"  [arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n\n"
			"Remarks:\n"
			"  To run a single command, define it in metadata section whose identifer must be \"script\".\n"
			"  To run script file, define it in metadata section whose identifer must be \"load\".\n"
			"  Support multiple \"script\" or \"load\" definitions. Running sequences rely on their defined orders.\n"
			"  Yara variables like #pattern, @pattern[n], !pattern[n] will be automatically replaced to their real values\n"
			"  in the command text or the script file before running.\n\n"
			"  The script file path can be relative to the directory where the rules file resides.\n"
			"  All commands including the script file will be put together to run as one script file through DbgScriptLoad function.\n"
			"  \\r and \\n will be unescaped in the command text defined by \"script\" before writing into script file.\n\n"
			"Sample:\n"
			"  rule test {\n"
			"   meta:\n"
			"    script=\"log \\\"Hello yaraEx!\\\"\"\n"
			"    load=\"d:\\yaraEx.yar\"\n"
			"   ...\n"
			"  }\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_YARAFIND, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_YARAFIND " <arg1>, [arg2], [arg3]\n"
			"  <arg1>: The address to start searching from.\n"
			"  <arg2>: The byte pattern of yara to search for.\n"
			"  [arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n"
			"The $result is set to the number of occurrences.\n"
			"The $result1 variable is set to the virtual address of the first address that matches the byte pattern.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_YARAFINDALL, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_YARAFINDALL " <arg1>, [arg2], [arg3]\n"
			"  <arg1>: The address to start searching from.\n"
			"  <arg2>: The byte pattern of yara to search for.\n"
			"  [arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n"
			"The $result is set to the number of occurrences.\n"
			"The $result1 variable is set to the virtual address of the first address that matches the byte pattern.\n"
			"All results will be shown in the gui reference view.\n");
	}

	else if (!compare_stringA(cmdname, CMD_NAME_ADD_STRUCT, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_ADD_STRUCT " <arg1>\n"
			"  <arg1>: Name of struct to add.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_REMOVE_STRUCT, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_REMOVE_STRUCT " <arg1>\n"
			"  <arg1>: Name of struct to remove.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_ADD_ANCESTOR, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_ADD_ANCESTOR " <arg1>, <arg2>\n"
			"  <arg1>: Name of struct to add ancestor.\n"
			"  <arg2>: Name of ancestor struct.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_INSERT_ANCESTOR, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_INSERT_ANCESTOR " <arg1>, <arg2>, [arg3]\n"
			"  <arg1>: Name of struct to insert ancestor.\n"
			"  <arg2>: Name of ancestor to add.\n"
			"  [arg3]: Name of ancestor struct to insert before.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_REMOVE_ANCESTOR, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_REMOVE_ANCESTOR " <arg1>, <arg2>\n"
			"  <arg1>: Name of struct to remove ancestor from.\n"
			"  <arg2>: Name of ancestor to remove.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_ADD_MEMBER, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_ADD_MEMBER " <arg1>, <arg2>, <arg3>, [arg4], [arg5]\n"
			"  <arg1>: Name of struct to add member.\n"
			"  <arg2>: Type name of member.\n"
			"  <arg3>: Name of member.\n"
			"  [arg4]: Array size of member. Default is 0 means not an array.\n"
			"  [arg5]: Explicitly set the offset of member. Default action is appending at the end.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_REMOVE_MEMBER, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_REMOVE_MEMBER " <arg1>, <arg2>\n"
			"  <arg1>: Name of struct to remove member.\n"
			"  <arg2>: Name of member to remove.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_SET_MEMBER_COMMENT, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_SET_MEMBER_COMMENT " <arg1>, <arg2>, [arg3]\n"
			"  <arg1>: Name of struct.\n"
			"  <arg2>: Name of member.\n"
			"  [arg3]: Comment text of member. Leave it empty to clear comment.\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_PRINT_STRUCT, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_PRINT_STRUCT " <arg1>, [arg2], [arg3], [arg4]\n"
			"  <arg1>: Name of struct to print.\n"
			"  [arg2]: Proposed length for formatting offset text. 0 indicates no offset\n"
			"  [arg3]: Proposed length for formatting type name text\n"
			"  [arg4]: Proposed length for formatting member name text\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_REMOVE_ALL, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_REMOVE_ALL "\n"
			"  There is no argument for this command\n");
	}
	else if (!compare_stringA(cmdname, CMD_NAME_SIZEOFTYPE, true))
	{
		_plugin_logputs(
			"Usage: " CMD_NAME_SIZEOFTYPE "<arg1>\n"
			"  <arg1>: Name of primitive type or struct type\n");
	}
}