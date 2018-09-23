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

#define CMD_NAME_YARAEX_LL					"yaraEx.SetLogLevel,yaraEx.ll"
#define CMD_NAME_YARAEX						"yaraEx"
#define CMD_NAME_YARAFIND					"yarafind"
#define CMD_NAME_YARAFINDALL				"yarafindall"

//#define CMD_NAME_STRUCT_ADD					"struct.add"
//#define CMD_NAME_STRUCT_REMOVE				"struct.remove"
//#define CMD_NAME_STRUCT_ADD_ANCESTOR		"struct.addAncestor"
//#define CMD_NAME_STRUCT_INSERT_ANCESTOR		"struct.insertAncestor"
//#define CMD_NAME_STRUCT_REMOVE_ANCESTOR		"struct.removeAncestor"
//#define CMD_NAME_STRUCT_ADD_MEMBER			"struct.addMember"
//#define CMD_NAME_STRUCT_REMOVE_MEMBER		"struct.removeMember"
//#define CMD_NAME_STRUCT_SET_MEMBER_COMMENT	"struct.setMemberComment"
//#define CMD_NAME_STRUCT_SET_COMMENT			"struct.setComment"
//#define CMD_NAME_STRUCT_PRINT				"struct.print"
//#define CMD_NAME_STRUCT_REMOVE_ALL			"struct.removeAll"
//#define CMD_NAME_STRUCT_SIZEOFTYPE			"struct.sizeOfType"
//
//#define CMD_NAME_UNION_ADD					"union.add"
//#define CMD_NAME_UNION_REMOVE				"union.remove"
//#define CMD_NAME_UNION_ADD_MEMBER			"union.addMember"
//#define CMD_NAME_UNION_REMOVE_MEMBER		"union.removeMember"
//#define CMD_NAME_UNION_SET_COMMENT			"union.setComment"
//#define CMD_NAME_UNION_SET_MEMBER_COMMENT	"union.setMemberComment"
//#define CMD_NAME_UNION_PRINT				"union.print"

////////////////////////////////////////////////
#define CMD_TYPE_STRUCT_ADD					"Type.AddStruct,Type.as"
#define CMD_TYPE_UNION_ADD					"Type.AddUnion,Type.au"
#define CMD_TYPE_REMOVE						"Type.Remove"
#define CMD_TYPE_ADD_MEMBER					"Type.AddMember,Type.am"
#define CMD_TYPE_REMOVE_MEMBER				"Type.removeMember,Type.rm"
#define CMD_TYPE_SET_COMMENT				"Type.comment"
#define CMD_TYPE_SET_MEMBER_COMMENT			"Type.memberComment,Type.mcomment"
#define CMD_TYPE_PRINT						"Type.print"
#define CMD_TYPE_REMOVE_ALL					"Type.removeAll"
#define CMD_TYPE_SIZE						"Type.size"

//This three commands are for structures only.
#define CMD_TYPE_ADD_ANCESTOR				"Type.addAncestor,Type.aanc"
#define CMD_TYPE_INSERT_ANCESTOR			"Type.insertAncestor,Type.ianc"
#define CMD_TYPE_REMOVE_ANCESTOR			"Type.removeAncestor,Type.ranc"

////////////////////////////////////////////////


