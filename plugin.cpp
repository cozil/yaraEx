#include "plugin.h"
#include "strutils.h"
#include "Handle.h"
#include "misc.h"
#include "command_yaraEx.h"
#include "command_struct.h"

static CTypeHelper _typeHelper;
static yaraEx _yaraInst;

static int print_exception(EXCEPTION_POINTERS * _excpt)
{
	if (!_excpt)
	{
		logputs(LL::Error, "Empty exception pointer!\n");
		return 1;
	}

	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(_excpt->ExceptionRecord->ExceptionAddress, &mbi, sizeof(mbi)))
	{
		char ModName[MAX_PATH];
		GetModuleFileNameA((HMODULE)mbi.AllocationBase, ModName, MAX_PATH);
		logprintf(LL::Error, "An exception occurred at address %p resided in module %p.",
			_excpt->ExceptionRecord->ExceptionAddress, mbi.AllocationBase);
			
		logprintf(LL::Error, "Module file is %s.", ascii_to_utf8(ModName).c_str());
	}
	else
	{
		logprintf(LL::Error, "An exception occurred at address %p.",
			_excpt->ExceptionRecord->ExceptionAddress);
	}
	
	if (_excpt->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		if (_excpt->ExceptionRecord->ExceptionInformation[0])
			logprintf(LL::Error, "Access violation on reading from address %p.",
			_excpt->ExceptionRecord->ExceptionInformation[1]);
		else
			logprintf(LL::Error, "Access violation on writing into address %p.",
				_excpt->ExceptionRecord->ExceptionInformation[1]);
	}
	else
	{
		logprintf(LL::Error, "Exception code is %p.",
			_excpt->ExceptionRecord->ExceptionCode);
	}

	return 1;
}

#define TRY_CALL(_func, _argN) \
	if (argc <= _argN) \
		{ \
		logprintf(LL::Error, "Not enough arguments!"); \
		print_usages(argv[0]); \
		return false; \
	} \
	bool success = false; \
	__try { \
		success = _func(argc, argv); \
	}__except(print_exception(GetExceptionInformation())) {} \
	return success


bool _plugin_registercommand_ex(int pluginHandle, const char* command, CBPLUGINCOMMAND cbCommand, bool debugonly)
{
	auto cmds = split_stringA(command, ",");
	for (auto& cmd : cmds)
	{
		_plugin_registercommand(pluginHandle, cmd.c_str(), cbCommand, debugonly);
	}

	return true;
}

//Initialize your plugin data here.

bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
	_plugin_registercommand_ex(pluginHandle, CMD_YARAEX_LL, [](int argc, char* argv[]) -> bool
	{
		if (argc <= 1)
		{
			logputs(LL::Error, "Not enough arguments!");
			print_usages(argv[0]);
			return false;
		}

		const char * lltexts[] =
		{ "Error", "Warning", "Normal", "Debug" };

		int l = atoi(argv[1]);
		if (l < 0) l = 0;
		if (l > (int)LL::Max) l = (int)LL::Max;
		set_log_level((LL)l);
		if (l < _countof(lltexts))
			logprintf(LL::Error, "Log level has been changed to \"%s\".", lltexts[l]);

		return true;
	}, false);

	//yara commands
	_plugin_registercommand_ex(pluginHandle, CMD_YARAEX, [](int argc, char* argv[]) -> bool
	{		
		TRY_CALL(_yaraInst.cmd_yaraEx, 1);
	}, true);

	_plugin_registercommand_ex(pluginHandle, CMD_YARAFIND, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_yaraInst.cmd_yarafind, 2);
	}, true);

	_plugin_registercommand_ex(pluginHandle, CMD_YARAFINDALL, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_yaraInst.cmd_yarafindall, 2);
	}, true);


	//type commands
	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_STRUCT_ADD, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_AddStruct, 1);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_UNION_ADD, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_AddUnion, 1);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_REMOVE, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_Remove, 1);
	}, false);
	
	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_ADD_MEMBER, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_AddMember, 3);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_REMOVE_MEMBER, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_RemoveMember, 2);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_SET_COMMENT, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_SetComment, 1);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_SET_MEMBER_COMMENT, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_SetMemberComment, 2);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_REMOVE_ALL, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_RemoveAll, 0);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_SIZE, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_Size, 1);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_PRINT, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_Print, 1);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_ADD_ANCESTOR, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_AddAncestor, 2);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_INSERT_ANCESTOR, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_InsertAncestor, 2);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_REMOVE_ANCESTOR, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_RemoveAncestor, 2);
	}, false);

	_plugin_registercommand_ex(pluginHandle, CMD_TYPE_REFERENCE, [](int argc, char* argv[]) -> bool
	{
		TRY_CALL(_typeHelper.cmd_type_Reference, 1);
	}, false);

    return true; //Return false to cancel loading the plugin.
}

//Deinitialize your plugin data here.
void pluginStop()
{
}

//Do GUI/Menu related things here.
void pluginSetup()
{
}
