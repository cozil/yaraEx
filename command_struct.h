#pragma once
#include "pluginmain.h"
#include "strutils.h"

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

	struct _Type
	{
		std::string name;		//Type identifier.
		Primitive primitive;	//Primitive type.
		duint size = 0;			//Size in bytes.
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
		std::string comment;
		int size;

		_Struct& operator=(const _Struct& struc)
		{
			name = struc.name;
			ancestors = struc.ancestors;
			members = struc.members;
			size = struc.size;
			comment = struc.comment;
			return *this;
		}
	};

	struct _Union
	{
		std::string name;
		std::vector<_Member> members;
		std::string comment;
		int size;

		_Union& operator=(const _Union& u)
		{
			name = u.name;
			members = u.members;
			size = u.size;
			comment = u.comment;
			return *this;
		}
	};

	typedef std::unordered_map<std::string, _Union> UnionMap;
	typedef std::unordered_map<std::string, _Struct> StructMap;
	typedef std::unordered_map<std::string, _Type> TypeMap;

public:
	CStructHelper();
	~CStructHelper();


	
	bool cmd_type_AddStruct(int argc, char* argv[]);
	bool cmd_type_AddUnion(int argc, char* argv[]);
	bool cmd_type_Remove(int argc, char* argv[]);
	bool cmd_type_AddMember(int argc, char* argv[]);
	bool cmd_type_RemoveMember(int argc, char* argv[]);
	bool cmd_type_SetComment(int argc, char* argv[]);
	bool cmd_type_SetMemberComment(int argc, char* argv[]);
	bool cmd_type_Print(int argc, char* argv[]);
	bool cmd_type_RemoveAll(int argc, char* argv[]);
	bool cmd_type_Size(int argc, char* argv[]);
	bool cmd_type_AddAncestor(int argc, char* argv[]);
	bool cmd_type_InsertAncestor(int argc, char* argv[]);
	bool cmd_type_RemoveAncestor(int argc, char* argv[]);
	
protected:
	bool structAddMember(_Struct& struc, _Member& member);
	bool unionAddMember(_Union& un, _Member& member);

	bool structRemove(StructMap::const_iterator itrStruct);
	bool unionRemove(UnionMap::const_iterator itrUnion);

	bool structRemoveMember(_Struct& struc, const std::string& memberName);
	bool unionRemoveMember(_Union& un, const std::string& memberName);

	void structPrint(const _Struct& struc, int offsetLen = -1, int typeLen = 0, int memberLen = 0);
	void unionPrint(const _Union& un, int offsetLen = -1, int typeLen = 0, int memberLen = 0);

	bool structInsertAncestor(const std::string& typeName, const std::string& ancName, const std::string& beforeAnc);
	bool structRemoveAncestor(const std::string& typeName, const std::string& ancName);
	
	
	int _sizeOfType(const std::string& type) const;
	bool _beforeExpandStruct(const std::string& changedStruct, int expand_size);
	bool _structRemoveAncestor(_Struct& struc, std::vector<_Ancestor>::const_iterator itrAncestor);
	bool _typeExists(const std::string& typeName) const;
	bool _isTypeUsed(const std::string& typeName) const;
	static bool _structRemoveMember(_Struct& struc, size_t& memberId);

private:


	UnionMap m_unions;
	StructMap m_structs;
	TypeMap m_types;
	std::unordered_map<Primitive, int> m_primitivesizes;
};

