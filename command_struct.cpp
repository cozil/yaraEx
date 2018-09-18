#include "command_struct.h"
#include "strutils.h"
#include "misc.h"
#include <algorithm>

BEGIN_STRUCT_CMD_MAP(CStructHelper)
	ON_STRUCT_CMD(CMD_NAME_ADD_STRUCT, 1, &addStruct)
	ON_STRUCT_CMD(CMD_NAME_REMOVE_STRUCT, 1, &removeStruct)
	ON_STRUCT_CMD(CMD_NAME_ADD_ANCESTOR, 2, &addAncestor)
	ON_STRUCT_CMD(CMD_NAME_INSERT_ANCESTOR, 2, &insertAncestor)
	ON_STRUCT_CMD(CMD_NAME_REMOVE_ANCESTOR, 2, &removeAncestor)
	ON_STRUCT_CMD(CMD_NAME_ADD_MEMBER, 3, &addMember)
	//ON_STRUCT_CMD(CMD_NAME_INSERT_MEMBER, 3, &insertMember)
	ON_STRUCT_CMD(CMD_NAME_REMOVE_MEMBER, 2, &removeMember)
	ON_STRUCT_CMD(CMD_NAME_SET_MEMBER_COMMENT, 2, &setMemberComment)
	ON_STRUCT_CMD(CMD_NAME_PRINT_STRUCT, 1, &printStruct)
	ON_STRUCT_CMD(CMD_NAME_REMOVE_ALL, 0, &removeAll)
END_STRUCT_CMD_MAP()

CStructHelper::CStructHelper()
{
	auto p = [this](const std::string & n, Primitive p, int size)
	{
		m_primitivesizes[p] = size;
		std::vector<std::string> splits;
		split_stringA(n, splits, ",");
		for (const auto & split : splits)
		{
			_Type t;
			t.name = split;
			t.primitive = p;
			t.size = size;
			m_types.insert({ split, t });
		}
	};
	p("int8_t,int8,char,byte,bool,signed char", Int8, sizeof(char));
	p("uint8_t,uint8,uchar,unsigned char,ubyte", Uint8, sizeof(unsigned char));
	p("int16_t,int16,wchar_t,char16_t,short", Int16, sizeof(short));
	p("uint16_t,uint16,ushort,unsigned short", Int16, sizeof(unsigned short));
	p("int32_t,int32,int,long", Int32, sizeof(int));
	p("uint32_t,uint32,unsigned int,unsigned long", Uint32, sizeof(unsigned int));
	p("int64_t,int64,long long", Int64, sizeof(long long));
	p("uint64_t,uint64,unsigned long long", Uint64, sizeof(unsigned long long));
	p("dsint", Dsint, sizeof(void*));
	p("duint,size_t", Duint, sizeof(void*));
	p("float", Float, sizeof(float));
	p("double", Double, sizeof(double));
	p("ptr,void*", Pointer, sizeof(void*));
	p("char*,const char*", PtrString, sizeof(char*));
	p("wchar_t*,const wchar_t*", PtrWString, sizeof(wchar_t*));
}


CStructHelper::~CStructHelper()
{
}

int CStructHelper::typeSize(const std::string& type) const
{
	auto found = m_types.find(type);
	if (found != m_types.cend())
		return found->second.size;
	return -1;
}

int CStructHelper::_structSize(const std::string& structName, std::vector<std::string>& stack) const
{
	//std::vector<std::string> stack;
	//stack.push_back(structName);


	
	auto itrStruct = std::find_if(m_structs.cbegin(), m_structs.cend(), [&structName](const _Struct& struc)
	{ return !compare_stringA(struc.name, structName, true); });
	
	if (itrStruct == m_structs.cend())
		return -1;



	
	int size = 0;
	for (const _Ancestor& ancestor : itrStruct->ancestors)
	{
		int _size = structSize(ancestor.struct_name);
		if (_size < 0)
			return -1;

		size += _size;
	}
}

bool CStructHelper::doCommand(int argc, char ** argv)
{
	if (argc <= 0 || !argv)
		return false;

	CmdArgumentSet args;
	std::string cmdName = argv[0];

	auto pos = cmdName.find(' ');
	if (pos == std::string::npos)
		pos = cmdName.find('\t');
	if (pos != std::string::npos)
		cmdName.erase(cmdName.cbegin() + pos, cmdName.cend());

	auto eval = [&cmdName](const char * expression, int& result) -> bool
	{
		bool success;
		result = (int)DbgEval(expression, &success);
		if (!success)
			logprintf(cmdName.c_str(), "Invalid expression: \"%s\"", expression);
		return success;
	};

	const STRUCT_CMD_ENTRY * pEntries = GetCmdEntries();
	while (pEntries->cmd_name)
	{
		if (!compare_stringA(cmdName, pEntries->cmd_name, true))
			break;
		++pEntries;
	}

	if (!pEntries->cmd_name || !pEntries->fnCmdCall)
	{
		logprintf(argv[0], "Internal error: command entry not found!\n");
		return false;
	}
	
	int min_argc = pEntries->min_argc;
	if (argc <= min_argc)
	{
		logprintf(argv[0], "Not enough Arguments!\n");
		print_usages(argv[0]);
		return false;
	}

	if (!compare_stringA(cmdName, CMD_NAME_ADD_STRUCT, true) ||
		!compare_stringA(cmdName, CMD_NAME_REMOVE_STRUCT, true) ||
		!compare_stringA(cmdName, CMD_NAME_PRINT_STRUCT, true))
	{
		args.structName = argv[1];
	}
	else if (
		!compare_stringA(cmdName, CMD_NAME_ADD_ANCESTOR, true) ||
		!compare_stringA(cmdName, CMD_NAME_INSERT_ANCESTOR, true) ||
		!compare_stringA(cmdName, CMD_NAME_REMOVE_ANCESTOR, true))
	{
		args.structName = argv[1];
		args.ancestorName = argv[2];
		if (argc > 3)
			args.beforeAncestor = argv[3];
	}
	else if (
		!compare_stringA(cmdName, CMD_NAME_ADD_MEMBER, true)) // ||	!compare_stringA(cmdName, CMD_NAME_INSERT_MEMBER, true))
	{
		args.structName = argv[1];
		args.memberType = argv[2];
		args.memberName = argv[3];
		if (argc > 4 && !eval(argv[4], args.arraySize))
			return false;

		if (argc > 5 && !eval(argv[5], args.memberOffset))
			return false;
	}
	else if (
		!compare_stringA(cmdName, CMD_NAME_REMOVE_MEMBER, true) ||
		!compare_stringA(cmdName, CMD_NAME_SET_MEMBER_COMMENT, true))
	{
		args.structName = argv[1];
		args.memberType = argv[2];
		if (argc > 3)
			args.comment = argv[3];
	}

	return (this->*pEntries->fnCmdCall)(args);
}



bool CStructHelper::addStruct(CmdArgumentSet& args)
{
	auto itr = std::find_if(m_structs.cbegin(), m_structs.cend(), [&args](const _Struct& struc)
	{ return !compare_stringA(struc.name, args.structName, true); });

	if (itr == m_structs.cend())
	{
		_Struct st;
		st.name= args.structName;
		m_structs.push_back(std::move(st));
		return true;
	}

	logprintf(CMD_NAME_ADD_STRUCT, "struct \"%s\" already exists!", args.structName.c_str());
	return false;
}

bool CStructHelper::removeStruct(CmdArgumentSet& args)
{
	auto itr = std::find_if(m_structs.cbegin(), m_structs.cend(), [&args](const _Struct& struc)
	{ return !compare_stringA(struc.name, args.structName, true); });

	if (itr != m_structs.cend())
	{
		m_structs.erase(itr);
		logputs(string_formatA("Struct \"%s\" has been removed!\n", args.structName.c_str()).c_str());
	}
	else
	{
		logputs(string_formatA("Struct \"%s\" doesn't exists!\n", args.structName.c_str()).c_str());
	}

	return false;
}

bool CStructHelper::addAncestor(CmdArgumentSet& args)
{
	args.beforeAncestor.clear();
	return insertAncestor(args);
}

bool CStructHelper::insertAncestor(CmdArgumentSet& args)
{
	auto itrStruct = std::find_if(m_structs.begin(), m_structs.end(), [&args](_Struct& struc)
	{ return !compare_stringA(struc.name, args.structName, true); });

	if (itrStruct == m_structs.cend())
	{
		logputs(string_formatA("Struct \"%s\" doesn't exists!\n", args.structName.c_str()).c_str());
		return false;
	}

	auto itrAncestor = std::find_if(m_structs.cbegin(), m_structs.cend(), [&args](const _Struct& struc)
	{ return !compare_stringA(struc.name, args.ancestorName, true); });

	if (itrAncestor == m_structs.cend())
	{
		logputs(string_formatA("Ancestor struct \"%s\" doesn't exists!\n", args.structName.c_str()).c_str());
		return false;
	}

	auto itr = std::find_if(itrStruct->ancestors.cbegin(), itrStruct->ancestors.cend(), [&args](const _Ancestor& ancestor)
	{ return !compare_stringA(ancestor.struct_name, args.ancestorName, true); });

	if (itr == itrStruct->ancestors.cend())
	{
		size_t insertAt = -1;
		_Ancestor anc;
		anc.struct_name = args.ancestorName;
		anc.access_type = _Ancestor::ACCESS_PUBLIC;
		if (args.beforeAncestor.size())
		{
			itr = std::find_if(itrStruct->ancestors.cbegin(), itrStruct->ancestors.cend(), [&args](const _Ancestor& ancestor)
			{
				return !compare_stringA(ancestor.struct_name, args.beforeAncestor, true);
			});

			if (itr == itrStruct->ancestors.cend())
			{
				logputs(string_formatA("Ancestor \"%s\" doesn't exists!\n", args.beforeAncestor.c_str()).c_str());
				return false;
			}

			itrStruct->ancestors.insert(itr, anc);
		}
		else
		{
			itrStruct->ancestors.push_back(anc);
		}
		return true;
	}

	logputs(string_formatA("Struct \"%s\" already has ancestor \"%s\"!\n",
		args.structName.c_str(), args.ancestorName.c_str()).c_str());

	return false;
}

bool CStructHelper::removeAncestor(CmdArgumentSet& args)
{
	auto itrStruct = std::find_if(m_structs.begin(), m_structs.end(), [&args](_Struct& struc)
	{ return !compare_stringA(struc.name, args.structName, true); });

	if (itrStruct == m_structs.cend())
	{
		logputs(string_formatA("Struct \"%s\" doesn't exists!\n", args.structName.c_str()).c_str());
		return false;
	}

	auto itr = std::find_if(itrStruct->ancestors.cbegin(), itrStruct->ancestors.cend(), [&args](const _Ancestor& ancestor)
	{ return !compare_stringA(ancestor.struct_name, args.ancestorName, true); });

	if (itr != itrStruct->ancestors.cend())
	{
		itrStruct->ancestors.erase(itr);
		logputs(string_formatA("Ancestor \"%s\" has been removed from struct \"%s\"!\n",
			args.ancestorName.c_str(), args.structName.c_str()).c_str());
		return true;
	}

	logputs(string_formatA("Struct \"%s\" doesn't have ancestor \"%s\"!\n",
		args.structName.c_str(), args.ancestorName.c_str()).c_str());

	return false;
}

bool CStructHelper::addMember(CmdArgumentSet& args)
{
	auto itrStruct = std::find_if(m_structs.begin(), m_structs.end(), [&args](_Struct& struc)
	{ return !compare_stringA(struc.name, args.structName, true); });

	if (itrStruct == m_structs.cend())
	{
		logputs(string_formatA("Struct \"%s\" doesn't exists!\n", args.structName.c_str()).c_str());
		return false;
	}

	auto itrMember = std::find_if(itrStruct->members.cbegin(), itrStruct->members.cend(), [&args](const _Member& member)
	{ return !compare_stringA(member.name, args.memberName, true); });

	if (itrMember != itrStruct->members.cend())
	{
		logputs(string_formatA("Struct \"%s\" already has member \"%s\"!\n",
			args.structName.c_str(), args.memberName.c_str()).c_str());
		return false;
	}


	_Member member;
	member.name = args.memberName;
	member.type = args.memberType;
	member.type_size = typeSize(member.type);
	member.array_size = args.arraySize;
	member.ispadding = false;
	
	if (member.type_size < 0)
	{
		auto pos = member.type.find('*');
		if (pos != std::string::npos)
		{
			std::string structTypeName = trim_rightA(member.type.substr(0, pos));
			std::string rest = trim_rightA(member.type.substr(pos));

			auto itrTypeStruct = std::find_if(m_structs.cbegin(), m_structs.cend(), [&args](const _Struct& struc)
			{ return !compare_stringA(struc.name, args.structName, true); });

			if (itrTypeStruct != m_structs.cend() && rest == "*")
				member.type_size = m_primitivesizes[Pointer];
		}
		else
		{
			member.type_size = structSize(member.type);
		}

		if (member.type_size < 0)
		{
			logprintf(CMD_NAME_ADD_MEMBER, "Invalid type \"%s\"!", args.memberType.c_str());
			return false;
		}
	}

	if (args.memberOffset < 0)
	{
		itrStruct->members.push_back(member);
		return true;
	}



	return false;
}

//bool CStructHelper::insertMember(CmdArgumentSet& args)
//{
//	return false;
//}

bool CStructHelper::removeMember(CmdArgumentSet& args)
{
	return false;
}

bool CStructHelper::setMemberComment(CmdArgumentSet& args)
{
	return false;
}

bool CStructHelper::printStruct(CmdArgumentSet& args)
{
	logputs(string_formatA("%d structs exist!\n", m_structs.size()).c_str());
	return true;
}

bool CStructHelper::removeAll(CmdArgumentSet& args)
{
	size_t count = m_structs.size();
	m_structs.clear();
	logputs(string_formatA("All %d structs have been removed!\n", count).c_str());
	return true;
}