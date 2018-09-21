#pragma once
#include "pluginmain.h"
#include "strutils.h"

struct CmdArgumentSet
{
	std::string structName;
	std::string ancestorName;
	std::string beforeAncestor;
	std::string memberType;
	std::string memberName;
	std::string beforeMember;
	std::string comment;
	int arraySize = 0;
	int memberOffset = -1;

	//used by struct.print
	int offsetLength = -1;
	int typeLength = 0;
	int memberNameLength = 0;
};

class CStructHelper;
typedef bool (CStructHelper::*CmdCall)(CmdArgumentSet&);

struct STRUCT_CMD_ENTRY
{
	const char * cmd_name;	//command name
	int min_argc;			//least argument count
	CmdCall fnCmdCall;		//command function pointer
};

#define DECLARE_STRUCT_CMD_MAP()	\
	static STRUCT_CMD_ENTRY _cmdEntries[];	\
	static const STRUCT_CMD_ENTRY * GetCmdEntries();


#define BEGIN_STRUCT_CMD_MAP(theClass)	\
	const STRUCT_CMD_ENTRY * theClass::GetCmdEntries() { return &theClass::_cmdEntries[0]; } \
	STRUCT_CMD_ENTRY theClass::_cmdEntries[] = {

#define END_STRUCT_CMD_MAP()	\
		{ nullptr, 0, nullptr }};

#define ON_STRUCT_CMD(cmd_name, min_argc, cmd_func)	\
		{ cmd_name, min_argc, cmd_func },

class CStructHelper
{
public:

	enum Primitive
	{
		Void,
		Int8,
		Uint8,
		Int16,
		Uint16,
		Int32,
		Uint32,
		Int64,
		Uint64,
		Dsint,
		Duint,
		Float,
		Double,
		Pointer,
		PtrString, //char* (null-terminated)
		PtrWString //wchar_t* (null-terminated)
	};

	enum Cmd
	{
		CmdAddStruct,
		CmdRemoveStruct,
		CmdAddAncestor,
		CmdInsertAncestor,
		CmdRemoveAncestor,
		CmdAddMember,
		CmdInsertMember,
		CmdRemoveMember,
		CmdSetMemberComment,
		CmdPrintStruct
	};

	struct _Type
	{
		std::string name; //Type identifier.
		Primitive primitive; //Primitive type.
		duint size = 0; //Size in bytes.
	};


	struct _Member
	{
		std::string type;
		std::string name;
		std::string comment;
		int type_size;
		int array_size;	//0 indicates no array
		int offset;
		bool ispadding;

		int get_size() const { return array_size ? array_size * type_size : type_size; }
		int get_next_offset() const { return offset + get_size(); }

		void set_padding(int _offset, int _size)
		{
			offset = _offset;
			type_size = 1;
			type = "char";
			array_size = _size;
			ispadding = true;
			name = string_formatA("__pad_%04x", offset);
		}

		_Member& operator=(const _Member& member)
		{
			type = member.type;
			name = member.name;
			type_size = member.type_size;
			array_size = member.array_size;
			offset = member.offset;
			ispadding = member.ispadding;
			return *this;
		}
	};

	struct _Ancestor
	{
		enum
		{
			ACCESS_PUBLIC,
			ACCESS_PROTECTED,
			ACCESS_PRIVATE
		};

		std::string struct_name;
		unsigned int access_type;

		_Ancestor& operator=(const _Ancestor& ancestor)
		{
			struct_name = ancestor.struct_name;
			access_type = ancestor.access_type;
			return *this;
		}
	};

	struct _Struct
	{
		std::string name;
		std::vector<_Ancestor> ancestors;
		std::vector<_Member> members;
		int size;

		_Struct& operator=(const _Struct& struc)
		{
			name = struc.name;
			ancestors = struc.ancestors;
			members = struc.members;
			size = struc.size;
			return *this;
		}
	};



public:
	CStructHelper();
	~CStructHelper();


	bool doCommand(int argc, char ** argv);
	

protected:
	DECLARE_STRUCT_CMD_MAP();

	bool addStruct(CmdArgumentSet& args);
	bool removeStruct(CmdArgumentSet& args);
	bool addAncestor(CmdArgumentSet& args);
	bool insertAncestor(CmdArgumentSet& args);
	bool removeAncestor(CmdArgumentSet& args);
	bool addMember(CmdArgumentSet& args);
	bool removeMember(CmdArgumentSet& args);
	bool setMemberComment(CmdArgumentSet& args);

	bool printStruct(CmdArgumentSet& args);
	bool removeAll(CmdArgumentSet& args);
	bool sizeOfType(CmdArgumentSet& args);

	int _sizeOfType(const std::string& type) const;
	bool _beforeExpandStruct(const std::string& changedStruct, int expand_size);
	bool _removeAncestor(_Struct& struc, std::vector<_Ancestor>::const_iterator itrAncestor);
	bool _removeMember(_Struct& struc, size_t& memberId);
private:
	std::unordered_map<std::string, _Struct> m_structs;
	std::unordered_map<std::string, _Type> m_types;
	std::unordered_map<Primitive, int> m_primitivesizes;
};

