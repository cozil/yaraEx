#pragma once

#include "pluginsdk/bridgemain.h"
#include "pluginsdk/_plugins.h"

#include "pluginsdk/_scriptapi_argument.h"
#include "pluginsdk/_scriptapi_assembler.h"
#include "pluginsdk/_scriptapi_bookmark.h"
#include "pluginsdk/_scriptapi_comment.h"
#include "pluginsdk/_scriptapi_debug.h"
#include "pluginsdk/_scriptapi_flag.h"
#include "pluginsdk/_scriptapi_function.h"
#include "pluginsdk/_scriptapi_gui.h"
#include "pluginsdk/_scriptapi_label.h"
#include "pluginsdk/_scriptapi_memory.h"
#include "pluginsdk/_scriptapi_misc.h"
#include "pluginsdk/_scriptapi_module.h"
#include "pluginsdk/_scriptapi_pattern.h"
#include "pluginsdk/_scriptapi_register.h"
#include "pluginsdk/_scriptapi_stack.h"
#include "pluginsdk/_scriptapi_symbol.h"

#include "pluginsdk/capstone/capstone.h"
#include "pluginsdk/DeviceNameResolver/DeviceNameResolver.h"
#include "pluginsdk/jansson/jansson.h"
#include "pluginsdk/lz4/lz4file.h"
#include "pluginsdk/TitanEngine/TitanEngine.h"
#include "pluginsdk/XEDParse/XEDParse.h"
#include "pluginsdk/yara/yara.h"


extern "C" {
#define PCRE2_STATIC	1
#define PCRE2_CODE_UNIT_WIDTH	8
#include "pluginsdk/pcre2/pcre2.h" 
}

#ifdef _WIN64
#pragma comment(lib, "pluginsdk/x64dbg.lib")
#pragma comment(lib, "pluginsdk/x64bridge.lib")
#pragma comment(lib, "pluginsdk/capstone/capstone_x64.lib")
#pragma comment(lib, "pluginsdk/DeviceNameResolver/DeviceNameResolver_x64.lib")
#pragma comment(lib, "pluginsdk/jansson/jansson_x64.lib")
#pragma comment(lib, "pluginsdk/lz4/lz4_x64.lib")
#pragma comment(lib, "pluginsdk/TitanEngine/TitanEngine_x64.lib")
#pragma comment(lib, "pluginsdk/XEDParse/XEDParse_x64.lib")
#pragma comment(lib, "pluginsdk/yara/yara_x64.lib")
#pragma comment(lib, "pluginsdk/pcre2/pcre2-8_x64.lib")
#else
#pragma comment(lib, "pluginsdk/x32dbg.lib")
#pragma comment(lib, "pluginsdk/x32bridge.lib")
#pragma comment(lib, "pluginsdk/capstone/capstone_x86.lib")
#pragma comment(lib, "pluginsdk/DeviceNameResolver/DeviceNameResolver_x86.lib")
#pragma comment(lib, "pluginsdk/jansson/jansson_x86.lib")
#pragma comment(lib, "pluginsdk/lz4/lz4_x86.lib")
#pragma comment(lib, "pluginsdk/TitanEngine/TitanEngine_x86.lib")
#pragma comment(lib, "pluginsdk/XEDParse/XEDParse_x86.lib")
#pragma comment(lib, "pluginsdk/yara/yara_x86.lib")
extern "C" {

#pragma comment(lib, "pluginsdk/pcre2/pcre2-8_x86.lib")    
}
#endif //_WIN64

#define Cmd(x) DbgCmdExecDirect(x)
#define Eval(x) DbgValFromString(x)
#define dprintf(x, ...) _plugin_logprintf("[" PLUGIN_NAME "] " x, __VA_ARGS__)
#define dputs(x) _plugin_logprintf("[" PLUGIN_NAME "] %s\n", x)
#define PLUG_EXPORT extern "C" __declspec(dllexport)

//superglobal variables
extern int pluginHandle;
extern HWND hwndDlg;
extern int hMenu;
extern int hMenuDisasm;
extern int hMenuDump;
extern int hMenuStack;
//extern HMODULE hModule;


//plugin data
#define PLUGIN_NAME "yaraEx"
#define PLUGIN_VERSION 1

//
//yara commands
//

#define CMD_YARAEX_LL						"yaraEx.SetLogLevel,yaraEx.ll"
#define CMD_YARAEX							"yaraEx"
#define CMD_YARAFIND						"yarafind"
#define CMD_YARAFINDALL						"yarafindall"

//
//type commands
//

#define CMD_TYPE_STRUCT_ADD					"Type.addStruct,Type.as"
#define CMD_TYPE_UNION_ADD					"Type.addUnion,Type.au"
#define CMD_TYPE_REMOVE						"Type.remove"
#define CMD_TYPE_ADD_MEMBER					"Type.addMember,Type.am"
#define CMD_TYPE_REMOVE_MEMBER				"Type.removeMember,Type.rm"
#define CMD_TYPE_SET_COMMENT				"Type.comment"
#define CMD_TYPE_SET_MEMBER_COMMENT			"Type.memberComment,Type.mcomment"
#define CMD_TYPE_PRINT						"Type.print"
#define CMD_TYPE_REMOVE_ALL					"Type.removeAll"
#define CMD_TYPE_SIZE						"Type.size"
#define CMD_TYPE_REFERENCE					"Type.reference,Type.ref"
#define CMD_TYPE_OFFSET						"Type.offset"

//This three commands are for structures only.
#define CMD_TYPE_ADD_ANCESTOR				"Type.addAncestor,Type.aanc"
#define CMD_TYPE_INSERT_ANCESTOR			"Type.insertAncestor,Type.ianc"
#define CMD_TYPE_REMOVE_ANCESTOR			"Type.removeAncestor,Type.ranc"
#define CMD_TYPE_ADD_DECLARATION			"Type.addDeclaration,Type.ad"
#define CMD_TYPE_REMOVE_DECLARATION			"Type.removeDeclaration,Type.rd"

//
//array commands
//

#define CMD_ARRAY_SET						"Array.set"
#define CMD_ARRAY_GET						"Array.get"
#define CMD_ARRAY_REMOVE					"Array.remove"
#define CMD_ARRAY_REMOVEALL					"Array.removeAll"






