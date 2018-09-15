#include "plugin.h"
#include "strutils.h"
#include "Handle.h"
#include "yaraEx.h"
#include "misc.h"

static int print_exception(EXCEPTION_POINTERS * _excpt)
{
	if (!_excpt)
	{
		logputs("Empty exception pointer!\n");
		return 1;
	}

	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(_excpt->ExceptionRecord->ExceptionAddress, &mbi, sizeof(mbi)))
	{
		//logputs(string_formatA("module handle: %p\n", mbi.AllocationBase).c_str());
		char ModName[MAX_PATH];
		GetModuleFileNameA((HMODULE)mbi.AllocationBase, ModName, MAX_PATH);
		logputs(string_formatA("An exception occurred at address %p resided in module %p.\n",
			_excpt->ExceptionRecord->ExceptionAddress, mbi.AllocationBase).c_str());
			
		logputs(string_formatA("Module file is %s.\n", ascii_to_utf8(ModName).c_str()).c_str());
	}
	else
	{
		logputs(string_formatA("An exception occurred at address %p.\n",
			_excpt->ExceptionRecord->ExceptionAddress).c_str());
	}
	
	if (_excpt->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		if (_excpt->ExceptionRecord->ExceptionInformation[0])
			logputs(string_formatA("Access violation on reading from address %p.\n",
				_excpt->ExceptionRecord->ExceptionInformation[1]).c_str());
		else
			logputs(string_formatA("Access violation on writing into address %p.\n",
				_excpt->ExceptionRecord->ExceptionInformation[1]).c_str());
	}
	else
	{
		logputs(string_formatA("Exception code is %p.\n",
			_excpt->ExceptionRecord->ExceptionCode).c_str());
	}

	return 1;
}


//static bool _cbYaraEx(int argc, char* argv[])
//{
//	yaraEx yarInst;
//	return yarInst.cmd_yaraEx(argc, argv);
//}



#define TRY_CALL(func) \
	bool success = false; \
	__try { \
		success =  func(argc,argv); \
	}__except(print_exception(GetExceptionInformation())) {} \
	return success

static yaraEx _yaraInst;

static bool cb_cmd_yaraEx(int argc, char* argv[])
{
	TRY_CALL(_yaraInst.cmd_yaraEx);
}

static bool cb_cmd_yarafind(int argc, char* argv[])
{
	TRY_CALL(_yaraInst.cmd_yarafind);
}

static bool cb_cmd_yarafindall(int argc, char* argv[])
{
	TRY_CALL(_yaraInst.cmd_yarafindall);
}

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
	_plugin_registercommand(pluginHandle, CMD_NAME_YARAEX, cb_cmd_yaraEx, true);
	_plugin_registercommand(pluginHandle, CMD_NAME_YARAFIND, cb_cmd_yarafind, true);
	_plugin_registercommand(pluginHandle, CMD_NAME_YARAFINDALL, cb_cmd_yarafindall, true);

	//_plugin_unregistercommand(pluginHandle, "strget");
	//_plugin_unregistercommand(pluginHandle, "getstr");
    return true; //Return false to cancel loading the plugin.
}

//Deinitialize your plugin data here.
void pluginStop()
{
	_plugin_unregistercommand(pluginHandle, CMD_NAME_YARAEX);
	_plugin_unregistercommand(pluginHandle, CMD_NAME_YARAFIND);
	_plugin_unregistercommand(pluginHandle, CMD_NAME_YARAFINDALL);
}

//Do GUI/Menu related things here.
void pluginSetup()
{
}
