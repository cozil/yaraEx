#include "command_struct.h"
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
	ON_STRUCT_CMD(CMD_NAME_SIZEOFTYPE, 1, &sizeOfType)
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

int CStructHelper::_sizeOfType(const std::string& type) const
{
	auto found = m_types.find(type);
	if (found != m_types.cend())
		return found->second.size;

	auto pos = type.find('*');
	if (pos != std::string::npos)
	{
		std::string rest = trim_rightA(type.substr(pos));
		std::string structName = trim_rightA(type.substr(0, pos));

		auto itrStruct = m_structs.find(structName);
		if (itrStruct != m_structs.cend() && rest == "*")
			return m_primitivesizes.at(Pointer);
	}
	else
	{
		auto itrStruct = m_structs.find(type);
		if (itrStruct != m_structs.cend())
			return itrStruct->second.size;
	}

	return -1;
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
			logprintf(LL::Error, "Invalid expression: \"%s\"", expression);
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
		logprintf(LL::Error, "Internal error: command entry not found!");
		return false;
	}
	
	int min_argc = pEntries->min_argc;
	if (argc <= min_argc)
	{
		logprintf(LL::Error, "Not enough Arguments!");
		print_usages(pEntries->cmd_name);
		return false;
	}

	if (!compare_stringA(cmdName, CMD_NAME_SIZEOFTYPE, true) || 
		!compare_stringA(cmdName, CMD_NAME_ADD_STRUCT, true) ||
		!compare_stringA(cmdName, CMD_NAME_REMOVE_STRUCT, true))
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
		args.memberName = argv[2];
		if (argc > 3)
			args.comment = argv[3];
	}
	else if (!compare_stringA(cmdName, CMD_NAME_PRINT_STRUCT, true))
	{
		args.structName = argv[1];
		if (argc > 2 && !eval(argv[2], args.offsetLength))
			return false;

		if (argc > 3 && !eval(argv[3], args.typeLength))
			return false;

		if (argc > 4 && !eval(argv[4], args.memberNameLength))
			return false;
	}

	return (this->*pEntries->fnCmdCall)(args);
}

bool CStructHelper::addStruct(CmdArgumentSet& args)
{
	if (m_structs.find(args.structName) != m_structs.cend())
	{
		logprintf(LL::Warning, "struct \"%s\" already exists!", args.structName.c_str());
		return false;
	}

	_Struct struc;
	struc.name = args.structName;
	struc.size = 0;
	m_structs.insert({ args.structName, struc });
	logprintf(LL::Message, "Struct \"%s\" has been added!", args.structName.c_str());
	return true;
}


bool CStructHelper::removeStruct(CmdArgumentSet& args)
{
	auto itrStruct = m_structs.find(args.structName);
	if (itrStruct == m_structs.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", args.structName.c_str());
		return false;
	}

	std::vector<_Struct> interim;
	for (auto& pair : m_structs)
	{
		_Struct struc = pair.second;
		bool found = false;
		auto itrAncestor = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(), [&args](const _Ancestor& ancestor)
		{ return !compare_stringA(ancestor.struct_name, args.structName, true); });

		if (itrAncestor != struc.ancestors.cend())
		{
			if (!_removeAncestor(struc, itrAncestor))
				return false;

			logprintf(LL::Message, "Struct \"%s\" has been removed from its derived struct \"%s\"!",
				args.structName.c_str(), struc.name.c_str());
			found = true;
		}

		for (size_t i = 0; i < struc.members.size(); ++i)
		{
			_Member& member = struc.members[i];
			if (compare_stringA(member.name, args.structName, true))
				continue;

			if (!_removeMember(struc, i))
				return false;

			logprintf(LL::Message, "Struct \"%s\" has been removed from struct \"%s\" as member \"%s\"!",
				args.structName.c_str(), struc.name.c_str(), member.name.c_str());
			found = true;
		}

		if (found)
		{
			interim.push_back(struc);
		}
	}

	for (const _Struct& struc : interim)
	{
		m_structs[struc.name] = struc;
	}

	m_structs.erase(itrStruct);
	logprintf(LL::Message, "Struct \"%s\" has been removed!", args.structName.c_str());

	return false;
}

bool CStructHelper::addAncestor(CmdArgumentSet& args)
{
	args.beforeAncestor.clear();
	return insertAncestor(args);
}

bool CStructHelper::insertAncestor(CmdArgumentSet& args)
{
	if (!compare_stringA(args.structName, args.ancestorName, true))
	{
		logprintf(LL::Error, "Unable to add itself to be ancestor!");
		return false;
	}

	auto itrStruct = m_structs.find(args.structName);
	if (itrStruct == m_structs.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", args.structName.c_str());
		return false;
	}

	auto itrAncestor = m_structs.find(args.ancestorName);
	if (itrAncestor == m_structs.cend())
	{
		logprintf(LL::Warning, "Ancestor struct \"%s\" doesn't exists!", args.ancestorName.c_str());
		return false;
	}

	_Struct& struc = itrStruct->second;
	auto itr = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(),
		[&args](const _Ancestor& ancestor) 
	{ return !compare_stringA(ancestor.struct_name, args.ancestorName, true); });

	if (itr != struc.ancestors.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" already has ancestor \"%s\"!",
			args.structName.c_str(), args.ancestorName.c_str());
		return false;
	}

	size_t insertAt = -1;
	_Ancestor anc;

	anc.struct_name = args.ancestorName;
	anc.access_type = _Ancestor::ACCESS_PUBLIC;

	//Find insertion position
	if (args.beforeAncestor.size())
	{
		itr = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(),
			[&args](const _Ancestor& ancestor)
		{ return !compare_stringA(ancestor.struct_name, args.beforeAncestor, true); });

		if (itr == struc.ancestors.cend())
		{
			logprintf(LL::Warning, "Ancestor \"%s\" doesn't exists!", args.beforeAncestor.c_str());
			return false;
		}
	}
	else
	{
		itr = struc.ancestors.cend();
	}

	//Inserting
	if (struc.members.size() != 0 &&
		(!struc.members[0].ispadding || struc.members[0].get_size() < itrAncestor->second.size))
	{
		logprintf(LL::Error, "Not enough space for inserting ancestor \"%s\"!", args.ancestorName.c_str());
		return false;
	}

	if (struc.members.size() != 0)
	{
		struc.members[0].set_padding(struc.members[0].offset + itrAncestor->second.size, struc.members[0].get_size() - itrAncestor->second.size);
		if (struc.members[0].array_size == 0)
			struc.members.erase(struc.members.cbegin());
	}
	else
	{
		if (!_beforeExpandStruct(struc.name, itrAncestor->second.size))
			return false;

		struc.size += itrAncestor->second.size;
	}

	struc.ancestors.insert(itr, anc);

	logprintf(LL::Message, "Struct \"%s\" has been inserted into ancestor list of struct \"%s\"!",
		args.ancestorName.c_str(), args.structName.c_str());
	return true;

}

bool CStructHelper::removeAncestor(CmdArgumentSet& args)
{
	auto itrStruct = m_structs.find(args.structName);
	if (itrStruct == m_structs.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", args.structName.c_str());
		return false;
	}

	_Struct& struc = itrStruct->second;
	auto itrAncestor = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(), [&args](const _Ancestor& ancestor)
	{ return !compare_stringA(ancestor.struct_name, args.ancestorName, true); });

	if (itrAncestor == struc.ancestors.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't have ancestor \"%s\"!",
			args.structName.c_str(), args.ancestorName.c_str());
		return false;
	}
	
	if (!_removeAncestor(struc, itrAncestor))
		return false;

	logprintf(LL::Message, "Ancestor \"%s\" has been removed from struct \"%s\"!",
		args.ancestorName.c_str(), args.structName.c_str());
	return true;
}

bool CStructHelper::addMember(CmdArgumentSet& args)
{
	if (!compare_stringA(args.structName, args.memberType, true))
	{
		logprintf(LL::Error, "Unable to add members of its own type to the structure.!");
		return false;
	}

	auto itrStruct = m_structs.find(args.structName);
	if (itrStruct == m_structs.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", args.structName.c_str());
		return false;
	}

	_Struct& currentStruct = itrStruct->second;
	if (currentStruct.members.cend() != std::find_if(currentStruct.members.cbegin(), currentStruct.members.cend(), [&args](const _Member& member)
	{ return !compare_stringA(member.name, args.memberName, true); }))
	{
		logprintf(LL::Warning, "Struct \"%s\" already has member \"%s\"!",
			args.structName.c_str(), args.memberName.c_str());
		return false;
	}
	
	_Member member;
	member.name = args.memberName;
	member.type = args.memberType;
	member.type_size = _sizeOfType(member.type);
	member.array_size = args.arraySize;
	member.offset = args.memberOffset;
	member.ispadding = false;

	if (member.type_size < 0)
	{
		logprintf(LL::Error, "Invalid type \"%s\"!", member.type.c_str());
		return false;
	}

	if (member.array_size == 0)
	{
		logprintf(LL::Debug, "Add member: /*%p*/ %s %s; sizeof(%s)=0x%x",
			member.offset, member.type.c_str(), member.name.c_str(), member.name.c_str(), member.type_size);
	}
	else
	{
		logprintf(LL::Debug, "Add member: /*%p*/ %s %s[0x%x]; sizeof(%s)=0x%x",
			member.offset, member.type.c_str(), member.name.c_str(), member.array_size, member.name.c_str(), member.type_size);
	}

	//Add member at the end. It will increase the struct size. Those structs
	//who derived from it or have members of the struct must keep their sizes unchanged.
	//The increased size which is member.type_size can be neutralized by an immediately followed padding block.
	//If there is no such block or the block size is not enough, then it fails.

	if (member.offset < 0)
	{
		if (!_beforeExpandStruct(args.structName, member.get_size()))
			return false;

		member.offset = currentStruct.size;
		currentStruct.size = member.get_next_offset();
		currentStruct.members.push_back(member);
	}
	else
	{
		//find out which member is at address arg.memberOffset
		size_t i;
		for (i = 0; i < currentStruct.members.size(); ++i)
		{
			const _Member& m = currentStruct.members[i];

			logprintf(LL::Debug, "Comparing offset %p with first member at %p",
				member.offset, m.offset);

			if (member.offset < m.offset)
			{
				--i;
				break;				
			}

			if (member.offset < (m.offset + m.get_size()))
				break;
		}

		logprintf(LL::Debug, "Overlapped with member id: %d", i);

		if ((int)i < 0)
		{
			logprintf(LL::Error, "No padding block for new member at offset %p!", member.offset);
			return false;
		}
		
		if (i < currentStruct.members.size())
		{
			if (!currentStruct.members[i].ispadding)
			{
				logprintf(LL::Error, "Can't add member inside \"%s\" at offset %p which is not a padding block!", 
					currentStruct.members[i].name.c_str(), member.offset);
				return false;
			}

			int increased_size = member.get_next_offset() - currentStruct.members[i].get_next_offset();
			logprintf(LL::Debug, "Increased size:%d", increased_size);

			if (increased_size > 0)
			{
				if (i + 1 < currentStruct.members.size())
				{
					if (!currentStruct.members[i + 1].ispadding)
					{
						logprintf(LL::Error, "Can't add member crossed with two members at offset %p. One of them is not a padding block!", 
							member.offset);
						return false;
					}

					currentStruct.members[i].set_padding(currentStruct.members[i].offset,
						member.offset - currentStruct.members[i].offset);

					currentStruct.members[i + 1].set_padding(member.get_next_offset(),
						currentStruct.members[i + 1].get_next_offset() - member.get_next_offset());

					currentStruct.members.insert(currentStruct.members.cbegin() + i + 1, member);
				}
				else
				{
					if (!_beforeExpandStruct(args.structName, increased_size))
						return false;

					currentStruct.members[i].set_padding(currentStruct.members[i].offset,
						member.offset - currentStruct.members[i].offset);

					currentStruct.members.push_back(member);
				}
			}
			else
			{
				if (increased_size < 0)
				{
					_Member padding;
					padding.set_padding(member.get_next_offset(), - increased_size);
					currentStruct.members.insert(currentStruct.members.cbegin() + i + 1, padding);
				}

				currentStruct.members[i].set_padding(currentStruct.members[i].offset,
					member.offset - currentStruct.members[i].offset);

				if (currentStruct.members[i].array_size == 0)
					currentStruct.members[i] = member;
				else
					currentStruct.members.insert(currentStruct.members.cbegin() + i + 1, member);
			}
		}
		else
		{
			int increased_size = member.get_next_offset() - currentStruct.size;
			if (!_beforeExpandStruct(args.structName, increased_size))
				return false;

			if (currentStruct.members.size() == 0 || !currentStruct.members[currentStruct.members.size()-1].ispadding)
			{
				_Member padding;
				padding.set_padding(currentStruct.size, member.offset - currentStruct.size);
				currentStruct.members.push_back(padding);
			}
			else
			{
				_Member& last = currentStruct.members[currentStruct.members.size() - 1];
				last.set_padding(last.offset, member.offset - last.offset);
			}

			currentStruct.size = member.get_next_offset();
			currentStruct.members.push_back(member);
		}
	}

	logprintf(LL::Message, "Member \"%s\" successfully added. Offset:%p!",
		member.name.c_str(), member.offset);
	return true;
}

bool CStructHelper::removeMember(CmdArgumentSet& args)
{
	auto itrStruct = m_structs.find(args.structName);
	if (itrStruct == m_structs.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", args.structName.c_str());
		return false;
	}

	_Struct& currentStruct = itrStruct->second;
	auto itrMember = std::find_if(currentStruct.members.cbegin(), currentStruct.members.cend(),
		[&args](const _Member& member)
	{ return !compare_stringA(member.name, args.memberName, true); });

	if (itrMember == currentStruct.members.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't have member \"%s\"!",
			args.structName.c_str(), args.memberName.c_str());
		return false;
	}

	size_t id = itrMember - currentStruct.members.cbegin();
	if (!_removeMember(currentStruct, id))
		return false;

	logprintf(LL::Message, "Member \"%s\" has been removed from \"%s\"!",
		args.memberName.c_str(), args.structName.c_str());
	return true;
}

bool CStructHelper::setMemberComment(CmdArgumentSet& args)
{
	auto itrStruct = m_structs.find(args.structName);
	if (itrStruct == m_structs.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", args.structName.c_str());
		return false;
	}

	_Struct& currentStruct = itrStruct->second;
	auto itrMember = std::find_if(currentStruct.members.begin(), currentStruct.members.end(),
		[&args](_Member& member)
	{ return !compare_stringA(member.name, args.memberName, true); });

	if (itrMember == currentStruct.members.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't have member \"%s\"!",
			args.structName.c_str(), args.memberName.c_str());
		return false;
	}

	itrMember->comment = args.comment;
	return true;
}

bool CStructHelper::printStruct(CmdArgumentSet& args)
{
	auto itrStruct = m_structs.find(args.structName);
	if (itrStruct == m_structs.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", args.structName.c_str());
		return false;
	}

	const _Struct& struc = itrStruct->second;
	std::string result, str;

	result.reserve(1024);
	result = string_formatA("//sizeof(%s):0x%x(%d)\nstruct %s",
		struc.name.c_str(), struc.size,struc.size, struc.name.c_str());

	std::for_each(struc.ancestors.cbegin(), struc.ancestors.cend(), [&str](const _Ancestor& ancestor)
	{
		if (str.size() != 0) str += ",";
		switch (ancestor.access_type)
		{
		case _Ancestor::ACCESS_PRIVATE:
			str += string_formatA(" private %s", ancestor.struct_name.c_str());
			break;
		case _Ancestor::ACCESS_PROTECTED:
			str += string_formatA(" protected %s", ancestor.struct_name.c_str());
			break;
		case _Ancestor::ACCESS_PUBLIC:
			str += string_formatA(" public %s", ancestor.struct_name.c_str());
			break;
		}
	});

	if (str.size() != 0)
		result += ":" + str;
	result += "\n{\n";

	auto fill_spaces = [](std::string& text, int maxlength)
	{
		if (maxlength > 0 && text.size() < (size_t)maxlength)
			text.append((size_t)maxlength - text.size() + 1, ' ');
		else
			text.append(1, ' ');
	};

	auto str_replace = [](std::string& str, const std::string& find, const char * replacedby)
	{
		auto pos = str.find(find.c_str());
		while (pos != std::string::npos)
		{
			str.replace(pos, find.length(), replacedby);
			pos = str.find(find.c_str());
		}
	};


	
	for (const auto& member : struc.members)
	{
		bool set_comment = (member.comment.size() != 0);

		//If comment text has multiple lines, putting it before member text
		if (set_comment && member.comment.find("\\n") != std::string::npos)
		{
			set_comment = false;
			str = member.comment;
			str_replace(str, "\\n", "\n");
			result += string_formatA("\n/*\n%s\n*/\n", str.c_str());
		}

		if (args.offsetLength > sizeof(void*))
			args.offsetLength = -1;

		//Formatting offset text
		if (args.offsetLength < 0)
		{
			result += string_formatA("/*%p*/    ", member.offset);
		}
		else if (args.offsetLength > 0)
		{
			result += string_formatA(string_formatA("/*%%0%dx*/    ", args.offsetLength).c_str(),
				member.offset);
		}
		else
		{
			result += "    ";
		}

		//Formatting type name text
		str = member.type;
		fill_spaces(str, args.typeLength);
		result += str;

		//Formatting member name text
		if (member.array_size == 0)
			str = string_formatA("%s;", member.name.c_str());
		else
			str = string_formatA("%s[0x%x];", member.name.c_str(), member.array_size);

		if (set_comment)
		{
			fill_spaces(str, args.memberNameLength);
			result += str;
			result += " // ";
			result += member.comment;
		}
		else
		{
			result += str;
		}

		result += "\n";
	}

	result += "};\n";
	_plugin_logputs(result.c_str());
	return true;
}

bool CStructHelper::removeAll(CmdArgumentSet& args)
{
	size_t count = m_structs.size();
	m_structs.clear();
	logprintf(LL::Message, "All %d structs have been removed!", count);
	return true;
}

bool CStructHelper::sizeOfType(CmdArgumentSet& args)
{
	int size = _sizeOfType(args.structName);
	logprintf(LL::Message, "sizeof(%s) = 0x%x (.%d)",
		args.structName.c_str(), size, size);

	return DbgScriptCmdExec(string_formatA("$RESULT=0x%x", size).c_str());
}

bool CStructHelper::_beforeExpandStruct(const std::string& changedStruct, int expand_size)
{
	//Maintain a vector to keep modified structs.
	//Save them to m_structs after all relevant structs have been successfully modified.
	std::vector<_Struct> interim;
	_Struct tempStruct;

	//Searching all structs derived from this struct or own members of it.
	for (auto& _pair : m_structs)
	{
		//
		// Checking ancestors.
		//
		_Struct * pCurrentCopy = nullptr;
		const _Struct& struc = _pair.second;

		auto itrAncestor = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(),
			[&changedStruct](const _Ancestor& ancestor)
		{ return !compare_stringA(changedStruct, ancestor.struct_name); });

		if (itrAncestor != struc.ancestors.cend())
		{
			if (struc.members.size() == 0 ||
				!struc.members[0].ispadding ||
				struc.members[0].get_size() < expand_size)
			{
				logprintf(LL::Error, "Struct \"%s\" derived from \"%s\" doesn't have enough space for increased size!",
					struc.name.c_str(), changedStruct.c_str());
				return false;
			}

			int rest_size = struc.members[0].get_size() - expand_size;
			tempStruct = struc;
			pCurrentCopy = &tempStruct;

			if (rest_size != 0)
			{
				tempStruct.members[0].set_padding(tempStruct.members[0].offset + expand_size, rest_size);
			}
			else
			{
				tempStruct.members.erase(tempStruct.members.cbegin());
			}
		}

		//
		// Checking members.
		//

		for (size_t i = 0; i < struc.members.size(); ++i)
		{
			if (compare_stringA(struc.members[i].type, changedStruct, true))
				continue;

			if (i + 1 >= struc.members.size() ||
				!struc.members[i + 1].ispadding ||
				struc.members[i + 1].get_size() < expand_size)
			{
				logprintf(LL::Error, "Struct \"%s\" who owns a member of \"%s\" doesn't have enough space for increased size!",
					struc.name.c_str(), changedStruct.c_str());
				return false;
			}

			int rest_size = struc.members[++i].get_size() - expand_size;
			if (!pCurrentCopy)
			{
				tempStruct = struc;
				pCurrentCopy = &tempStruct;
			}

			if (rest_size != 0)
			{
				pCurrentCopy->members[i].set_padding(pCurrentCopy->members[i].offset + expand_size, rest_size);
			}
			else
			{
				pCurrentCopy->members.erase(pCurrentCopy->members.cbegin() + i--);
			}
		}

		if (pCurrentCopy)
			interim.push_back(std::move(tempStruct));
	}

	//Save them.
	std::for_each(interim.begin(), interim.end(), [this](_Struct& struc)
	{ m_structs[struc.name] = std::move(struc);	});

	return true;
}

bool CStructHelper::_removeAncestor(_Struct& struc, std::vector<_Ancestor>::const_iterator itrAncestor)
{
	auto& ancestorStruct = m_structs.find(itrAncestor->struct_name);
	if (ancestorStruct == m_structs.end())
	{
		logprintf(LL::Error, "Ancestor \"%s\" doesn't exist!", itrAncestor->struct_name.c_str());
		return false;
	}

	if (struc.members.size() == 0)
	{
		_Member padding;
		padding.set_padding(struc.size - ancestorStruct->second.size, ancestorStruct->second.size);
		struc.members.push_back(padding);
	}
	else if (!struc.members[0].ispadding)
	{
		_Member padding;
		padding.set_padding(struc.members[0].offset - ancestorStruct->second.size, ancestorStruct->second.size);
		struc.members.insert(struc.members.cbegin(), padding);
	}
	else
	{
		struc.members[0].set_padding(struc.members[0].offset - ancestorStruct->second.size, 
			struc.members[0].get_size() + ancestorStruct->second.size);
	}

	struc.ancestors.erase(itrAncestor);
	return true;
}

bool CStructHelper::_removeMember(_Struct& struc, size_t& memberId)
{
	if (memberId >= struc.members.size())
	{
		logprintf(LL::Error, "Internal error: member index \"%d\" is out of range", memberId);
		return false;
	}

	_Member& current = struc.members[memberId];
	if (memberId != 0 && struc.members[memberId - 1].ispadding)
	{
		if (memberId + 1 < struc.members.size() && struc.members[memberId + 1].ispadding)
		{
			struc.members[memberId - 1].set_padding(struc.members[memberId - 1].offset,
				struc.members[memberId + 1].get_next_offset() - struc.members[memberId - 1].offset);

			//Remove memberId and memberId+1
			struc.members.erase(struc.members.cbegin() + memberId, 
				struc.members.cbegin() + memberId + 2);
		}
		else
		{
			struc.members[memberId - 1].set_padding(struc.members[memberId - 1].offset,
				current.get_next_offset() - struc.members[memberId - 1].offset);

			struc.members.erase(struc.members.cbegin() + memberId);
		}
		--memberId;
	}
	else
	{
		if (memberId + 1 < struc.members.size() && struc.members[memberId + 1].ispadding)
		{
			current.set_padding(current.offset, struc.members[memberId + 1].get_next_offset() - current.offset);
			struc.members.erase(struc.members.cbegin() + memberId + 1);
		}
		else
		{
			current.set_padding(current.offset, current.get_size());
		}
	}
	return true;
}