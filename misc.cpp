#include "misc.h"
#include "pluginmain.h"
#include "strutils.h"
#include <algorithm>

struct CmdHelp
{
	const char * name;
	const char * text;
};

static LL _log_level = LL::Message;

static const std::vector<CmdHelp> _cmd_helps =
{
	{
		CMD_YARAEX_LL,
		"Usage: %s <arg1>\n"
		"  <arg1>: Log level for outputing message(0 - Error, 1 - Warning, 2 - Normal, 3 - Debug)\n"
	},

	{
		CMD_YARAEX,
		"Usage: %s <arg1>, [arg2], [arg3]\n"
		"  <arg1>: Rules file to apply. This should be a full path.\n"
		"  [arg2]: Start address of the range to apply the rules to. If not specified, the disassembly selection will be used.\n"
		"  [arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n\n"
		"Remarks:\n"
		"  All results will be put into arrays using string identifier as name defined in \"strings\" section.\n"
		"  You can use it by \"Array.xxx\" command sets.\n"
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
		"  }\n"
	},

	{
		CMD_YARAFIND,
		"Usage: %s <arg1>, [arg2], [arg3]\n"
		"  <arg1>: The address to start searching from.\n"
		"  <arg2>: The byte pattern of yara to search for.\n"
		"  [arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n"
		"The $result is set to the number of occurrences.\n"
		"The $result1 variable is set to the virtual address of the first address that matches the byte pattern.\n"
	},

	{
		CMD_YARAFINDALL,
		"Usage: %s <arg1>, [arg2], [arg3]\n"
		"  <arg1>: The address to start searching from.\n"
		"  <arg2>: The byte pattern of yara to search for.\n"
		"  [arg3]: Size of the range to apply the rules to. When not specified, the whole page will be used.\n"
		"The $result is set to the number of occurrences.\n"
		"The $result1 variable is set to the virtual address of the first address that matches the byte pattern.\n"
		"All results will be shown in the gui reference view.\n"
	},

	{
		CMD_TYPE_STRUCT_ADD,
		"Usage: %s <arg1>\n"
		"  <arg1>: Name of struct to add.\n"
	},

	{
		CMD_TYPE_UNION_ADD,
		"Usage: %s <arg1>\n"
		"  <arg1>: Name of union to add.\n"
	},

	{
		CMD_TYPE_REMOVE,
		"Usage: %s <arg1>\n"
		"  <arg1>: Name of struct/union to remove.\n"
	},

	{
		CMD_TYPE_ADD_MEMBER,
		"Usage: %s <arg1>, <arg2>, <arg3>, [arg4], [arg5]\n"
		"  <arg1>: Name of struct/union to add member.\n"
		"  <arg2>: Type name of member.\n"
		"  <arg3>: Name of member.\n"
		"  [arg4]: Array size of member. Default is 0 means not an array.\n"
		"  [arg5]: Specify member offset. Will append it to the end if not supplied. Used for struct only.\n"
	},

	{
		CMD_TYPE_REMOVE_MEMBER,
		"Usage: %s <arg1>, <arg2>\n"
		"  <arg1>: Name of struct/union to remove member.\n"
		"  <arg2>: Name of member to remove.\n"
	},

	{
		CMD_TYPE_SET_COMMENT,
		"Usage: %s <arg1>, [arg2]\n"
		"  <arg1>: Name of struct/union.\n"
		"  [arg2]: Comment text. Leave it empty to clear comment. '\\n' can be used for multiple lines.\n"
		"  Comment text support string formatting. You can use '{...}' to format a value.\n"
	},

	{
		CMD_TYPE_SET_MEMBER_COMMENT,
		"Usage: %s <arg1>, <arg2>, [arg3]\n"
		"  <arg1>: Name of struct/union.\n"
		"  <arg2>: Name of member.\n"
		"  [arg3]: Comment text of member. Leave it empty to clear comment.'\\n' can be used for multiple lines.\n"
		"  Comment text support string formatting. You can use '{...}' to format a value.\n"
	},

	{
		CMD_TYPE_PRINT,
		"Usage: %s <arg1>, [arg2], [arg3], [arg4]\n"
		"  <arg1>: Name of struct/union to print.\n"
		"  [arg2]: Proposed length for formatting offset text. 0 indicates no offset.\n"
		"  [arg3]: Proposed length for formatting type name text.\n"
		"  [arg4]: Proposed length for formatting member name text.\n"
	},

	{
		CMD_TYPE_REMOVE_ALL,
		"Usage: %s \n"
		"  There is no argument for this command.\n"
	},

	{
		CMD_TYPE_SIZE,
		"Usage: %s <arg1>\n"
		"  <arg1>: Name of primitive type or struct/union type\n"
		"  Result value will be stored in $RESULT variable.\n"
	},

	{
		CMD_TYPE_ADD_ANCESTOR,
		"Usage: %s <arg1>, <arg2>\n"
		"  <arg1>: Name of struct to add ancestor.\n"
		"  <arg2>: Name of ancestor struct.\n"
	},

	{
		CMD_TYPE_INSERT_ANCESTOR,
		"Usage: %s <arg1>, <arg2>, [arg3]\n"
		"  <arg1>: Name of struct to insert ancestor.\n"
		"  <arg2>: Name of ancestor to add.\n"
		"  [arg3]: Name of ancestor struct to insert before.\n"
	},

	{
		CMD_TYPE_REMOVE_ANCESTOR,
		"Usage: %s <arg1>, <arg2>\n"
		"  <arg1>: Name of struct to remove ancestor from.\n"
		"  <arg2>: Name of ancestor to remove.\n"
	},

	{
		CMD_TYPE_REFERENCE,
		"Usage: %s <arg1>\n"
		"  <arg1>: Name of struct/union.\n"
	},

	{
		CMD_TYPE_ADD_DECLARATION,
		"Usage: %s <arg1>, <arg2>\n"
		"  <arg1>: Name of struct.\n"
		"  <arg2>: Declaration text.\n"
		"  They will be outputed as it is while using command Type.print.\n"
		"  You can put function declarations here.\n"
	},

	{
		CMD_TYPE_REMOVE_DECLARATION,
		"Usage: %s <arg1>, <arg2>\n"
		"  <arg1>: Name of struct.\n"
		"  <arg2>: Keyword to search in declaration text.\n"
	},

	{
		CMD_TYPE_OFFSET,
		"Usage: %s <arg1>, <arg2>\n"
		"  <arg1>: Name of struct.\n"
		"  <arg2>: Name of member.\n"
		"  Result value will be stored in $RESULT variable.\n"
	},

	{ 
		CMD_ARRAY_SET,
		"Usage: %s <arg1>, <arg2>, <arg3>\n"
		"  <arg1>: Name of array.\n"
		"  <arg2>: Index of array value. Only integer value is accepted.\n"
		"  <arg3>: Value of array to set. Only integer value is accepted.\n"
	},

	{
		CMD_ARRAY_GET,
		"Usage: %s <arg1>, [arg2]\n"
		"  <arg1>: Name of array.\n"
		"  [arg2]: Index of array value to get. Only integer value is accepted.\n"
		"  If [arg2] is supplied, the result value will be saved into $RESULT variable.\n"
	},

	{
		CMD_ARRAY_REMOVE,
		"Usage: %s <arg1>\n"
		"  <arg1>: Name of array to remove.\n"
	},

	{
		CMD_ARRAY_REMOVEALL,
		"Usage: %s\n"
		"  There is no argument for this command.\n"
	}
};

void print_usages(const char * cmdname)
{
	std::string name = cmdname;
	auto pos = name.find(' ');
	if (pos == std::string::npos)
		pos = name.find('\t');

	if (pos != std::string::npos)
		name = name.substr(0, pos);

	for (const CmdHelp& help : _cmd_helps)
	{
		std::vector<std::string> cmds = split_stringA(help.name, ",");
		if (cmds.cend() != std::find_if(cmds.cbegin(), cmds.cend(), [&name](const std::string& cmd)
		{ return !compare_stringA(name, cmd, true); }))
		{
			_plugin_logprintf(help.text, join_stringA(cmds, "/").c_str());
			break;
		}
	}
}

void set_log_level(LL level)
{
	_log_level = level;
}

int get_log_level()
{
	return (int)_log_level;
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


bool eval(const char * expression, int& result)
{
	bool success;
	result = (int)DbgEval(expression, &success);
	if (!success)
		logprintf(LL::Error, "Invalid expression: \"%s\"", expression);
	return success;
};

std::string eval_format(const char * text)
{
	const DBGFUNCTIONS * _dbg_functions = DbgFunctions();
	char result[10240];
	if (_dbg_functions->StringFormatInline(text, _countof(result), result))
		return std::string(result);

	return std::string(text);
}

bool eval(const char * expression, duint& result)
{
	bool success;
	result = DbgEval(expression, &success);
	if (!success)
		logprintf(LL::Error, "Invalid expression: \"%s\"", expression);
	return success;
};

//Fill 'text' with space char at right side til the 'text' length equals 'maxlength'
void str_fill(std::string& text, int maxlength, char ch)
{
	int fillCount = maxlength - (int)text.size();
	if (fillCount > 0)
		text.append((size_t)fillCount, ' ');
}

//Replace 'find' to 'replaceby' in 'str'
void str_replace(std::string& str, const std::string& find, const char * replacedby)
{
	auto pos = str.find(find.c_str());
	while (pos != std::string::npos)
	{
		str.replace(pos, find.length(), replacedby);
		pos = str.find(find.c_str());
	}
}
