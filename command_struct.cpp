#include "command_struct.h"
#include "misc.h"
#include <algorithm>

bool eval(const char * expression, int& result)
{
	bool success;
	result = (int)DbgEval(expression, &success);
	if (!success)
		logprintf(LL::Error, "Invalid expression: \"%s\"", expression);
	return success;
};

//Fill 'text' with space char at right side til the 'text' length equals 'maxlength'
void fill_spaces(std::string& text, int maxlength)
{
	if (maxlength > 0 && text.size() < (size_t)maxlength)
		text.append((size_t)maxlength - text.size() + 1, ' ');
	else
		text.append(1, ' ');
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

CTypeHelper::CTypeHelper()
{
	auto p = [this](const std::string & n, Primitive p, int size)
	{
		m_primitivesizes[p] = size;
		std::vector<std::string> splits = split_stringA(n, ",");
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


CTypeHelper::~CTypeHelper()
{
}



bool CTypeHelper::cmd_type_AddStruct(int argc, char* argv[])
{
	_Struct struc;
	struc.name = argv[1];
	struc.size = 0;

	if (_typeExists(struc.name))
	{
		logprintf(LL::Warning, "Type \"%s\" already exists!", struc.name.c_str());
		return false;
	}

	m_structs.insert({ struc.name, struc });
	logprintf(LL::Message, "Struct \"%s\" has been added!", struc.name.c_str());
	return true;
}

bool CTypeHelper::cmd_type_AddUnion(int argc, char* argv[])
{
	_Union u;
	u.name = argv[1];
	u.size = 0;

	if (_typeExists(u.name))
	{
		logprintf(LL::Warning, "Type \"%s\" already exists!", u.name.c_str());
		return false;
	}

	m_unions.insert({ u.name, u });
	logprintf(LL::Message, "Union \"%s\" has been added!", u.name.c_str());
	return true;
}

bool CTypeHelper::cmd_type_Remove(int argc, char* argv[])
{
	std::string typeName = argv[1];
	auto itrStruct = m_structs.find(typeName);
	if (itrStruct != m_structs.cend())
		return structRemove(itrStruct);

	auto itrUnion = m_unions.find(typeName);
	if (itrUnion != m_unions.cend())
		return unionRemove(itrUnion);

	logprintf(LL::Warning, "Type \"%s\" doesn't exists!", typeName.c_str());
	return false;
}

bool CTypeHelper::cmd_type_AddMember(int argc, char* argv[])
{
	_Member member;
	std::string typeName = argv[1];
	member.type = argv[2];
	member.name = argv[3];
	member.offset = -1;
	member.array_size = 0;
	member.ispadding = false;

	if (!compare_stringA(typeName, member.type, true))
	{
		logprintf(LL::Error, "Unable to add a member of its own type.!");
		return false;
	}

	if (argc > 4 && !eval(argv[4], member.array_size))
		return false;

	if (argc > 5 && !eval(argv[5], member.offset))
		return false;

	member.type_size = _sizeOfType(member.type);
	if (member.type_size < 0)
	{
		logprintf(LL::Error, "Invalid type \"%s\"!", member.type.c_str());
		return false;
	}

	auto itrStruct = m_structs.find(typeName);
	if (itrStruct != m_structs.end())
		return structAddMember(itrStruct->second, member);

	auto itrUnion = m_unions.find(typeName);
	if (itrUnion != m_unions.end())
		return unionAddMember(itrUnion->second, member);

	logprintf(LL::Warning, "Type \"%s\" doesn't exists!", typeName.c_str());
	return false;
}

bool CTypeHelper::cmd_type_RemoveMember(int argc, char* argv[])
{
	std::string typeName = argv[1];
	std::string memberName = argv[2];

	auto itrStruct = m_structs.find(typeName);
	if (itrStruct != m_structs.end())
		return structRemoveMember(itrStruct->second, memberName);

	auto itrUnion = m_unions.find(typeName);
	if (itrUnion != m_unions.end())
		return unionRemoveMember(itrUnion->second, memberName);

	logprintf(LL::Warning, "Type \"%s\" doesn't exists!", typeName.c_str());
	return false;
}

bool CTypeHelper::cmd_type_SetComment(int argc, char* argv[])
{
	std::string typeName = argv[1];
	std::string comment;

	if (argc > 2) comment = argv[2];

	auto itrStruct = m_structs.find(typeName);
	if (itrStruct != m_structs.end())
	{
		itrStruct->second.comment = comment;
		return true;
	}

	auto itrUnion = m_unions.find(typeName);
	if (itrUnion != m_unions.end())
	{
		itrUnion->second.comment = comment;
		return true;
	}

	logprintf(LL::Warning, "Type \"%s\" doesn't exists!", typeName.c_str());
	return false;
}

bool CTypeHelper::cmd_type_SetMemberComment(int argc, char* argv[])
{
	std::string typeName = argv[1];
	std::string memberName = argv[2];
	std::string comment;

	if (argc > 3) comment = argv[3];

	auto itrStruct = m_structs.find(typeName);
	if (itrStruct != m_structs.cend())
	{
		auto itrMember = std::find_if(itrStruct->second.members.begin(), itrStruct->second.members.end(),
			[&memberName](_Member& member)
		{ return !compare_stringA(member.name, memberName, true); });

		if (itrMember == itrStruct->second.members.cend())
		{
			logprintf(LL::Warning, "Struct \"%s\" doesn't have member \"%s\"!",
				typeName.c_str(), memberName.c_str());
			return false;
		}

		itrMember->comment = comment;
		return true;
	}

	auto itrUnion = m_unions.find(typeName);
	if (itrUnion != m_unions.end())
	{
		auto itrMember = std::find_if(itrUnion->second.members.begin(), itrUnion->second.members.end(),
			[&memberName](_Member& member)
		{ return !compare_stringA(member.name, memberName, true); });

		if (itrMember == itrUnion->second.members.cend())
		{
			logprintf(LL::Warning, "Union \"%s\" doesn't have member \"%s\"!",
				typeName.c_str(), memberName.c_str());
			return false;
		}

		itrMember->comment = comment;
		return true;
	}

	logprintf(LL::Warning, "Type \"%s\" doesn't exists!", typeName.c_str());
	return false;
}

bool CTypeHelper::cmd_type_Print(int argc, char* argv[])
{
	std::string typeName = argv[1];
	int offsetLen = -1, typeLen = 0, memberLen = 0;

	if (argc > 2 && !eval(argv[2], offsetLen))
		return false;
	if (argc > 3 && !eval(argv[3], typeLen))
		return false;
	if (argc > 4 && !eval(argv[4], memberLen))
		return false;

	auto itrStruct = m_structs.find(typeName);
	if (itrStruct != m_structs.cend())
	{
		structPrint(itrStruct->second, offsetLen, typeLen, memberLen);
		return true;
	}

	auto itrUnion = m_unions.find(typeName);
	if (itrUnion != m_unions.cend())
	{
		unionPrint(itrUnion->second, offsetLen, typeLen, memberLen);
		return true;
	}

	logprintf(LL::Warning, "Type \"%s\" doesn't exists!", typeName.c_str());
	return false;
}

bool CTypeHelper::cmd_type_RemoveAll(int argc, char* argv[])
{
	size_t count = m_structs.size();
	m_structs.clear();
	logprintf(LL::Message, "All %d structs have been removed!", count);

	count = m_unions.size();
	m_unions.clear();
	logprintf(LL::Message, "All %d unions have been removed!", count);

	return true;
}

bool CTypeHelper::cmd_type_Size(int argc, char* argv[])
{
	int size = _sizeOfType(argv[1]);
	logprintf(LL::Message, "sizeof(%s) = 0x%x (.%d)", argv[1], size, size);
	return DbgScriptCmdExec(string_formatA("$RESULT=0x%x", size).c_str());
}

bool CTypeHelper::cmd_type_AddAncestor(int argc, char* argv[])
{
	std::string typeName = argv[1];
	std::string ancName = argv[2];
	std::string beforeAnc;

	return structInsertAncestor(typeName, ancName, beforeAnc);
}

bool CTypeHelper::cmd_type_InsertAncestor(int argc, char* argv[])
{
	std::string typeName = argv[1];
	std::string ancName = argv[2];
	std::string beforeAnc;
	if (argc > 3)
		beforeAnc = argv[3];

	return structInsertAncestor(typeName, ancName, beforeAnc);
}

bool CTypeHelper::cmd_type_RemoveAncestor(int argc, char* argv[])
{
	std::string typeName = argv[1];
	std::string ancName = argv[2];
	return structRemoveAncestor(typeName, ancName);
}

bool CTypeHelper::cmd_type_Reference(int argc, char* argv[])
{
	std::string typeName = argv[1];
	int count = 0;

	for (const auto& v : m_structs)
	{
		if (v.second.ancestors.cend() != std::find_if(v.second.ancestors.cbegin(),
			v.second.ancestors.cend(), [&typeName](const _Ancestor& anc)
		{ return !compare_stringA(typeName, anc.struct_name, true); }))
		{
			++count;
			logprintf(LL::Message, "Ancestor of struct \"%s\"", v.first.c_str());
		}

		for (const auto& member : v.second.members)
		{
			if (!compare_stringA(member.type, typeName, true))
			{
				++count;
				logprintf(LL::Message, "%s => %s", v.first.c_str(), member.name.c_str());
			}
		}
	}

	for (const auto& v : m_unions)
	{
		for (const auto& member : v.second.members)
		{
			if (!compare_stringA(member.type, typeName, true))
			{
				++count;
				logprintf(LL::Message, "%s => %s", v.first.c_str(), member.name.c_str());
			}
		}
	}

	logprintf(LL::Message, "%d references found!", count);
	return true;
}

bool CTypeHelper::structRemove(StructMap::const_iterator itrStruct)
{
	const std::string& structName = itrStruct->second.name;
	std::vector<_Struct> interim;
	for (auto& pair : m_structs)
	{
		_Struct struc = pair.second;
		bool found = false;
		auto itrAncestor = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(), 
			[&structName](const _Ancestor& ancestor)
		{ return !compare_stringA(ancestor.struct_name, structName, true); });

		if (itrAncestor != struc.ancestors.cend())
		{
			if (!_structRemoveAncestor(struc, itrAncestor))
				return false;

			logprintf(LL::Message, "Struct \"%s\" has been removed from its derived struct \"%s\"!",
				structName.c_str(), struc.name.c_str());
			found = true;
		}

		for (size_t i = 0; i < struc.members.size(); ++i)
		{
			_Member& member = struc.members[i];
			if (compare_stringA(member.type, structName, true))
				continue;

			if (!_structRemoveMember(struc, i))
				return false;

			logprintf(LL::Message, "Struct \"%s\" has been removed from struct \"%s\" as member \"%s\"!",
				structName.c_str(), struc.name.c_str(), member.name.c_str());
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
	logprintf(LL::Message, "Struct \"%s\" has been removed!", structName.c_str());
	return false;
}


bool CTypeHelper::structInsertAncestor(const std::string& typeName, const std::string& ancName, const std::string& beforeAnc)
{
	auto itrStruct = m_structs.find(typeName);
	if (itrStruct == m_structs.end())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", typeName.c_str());
		return false;
	}

	auto itrAncestor = m_structs.find(ancName);
	if (itrAncestor == m_structs.cend())
	{
		logprintf(LL::Warning, "Ancestor struct \"%s\" doesn't exists!", ancName.c_str());
		return false;
	}

	_Struct& struc = itrStruct->second;
	auto itr = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(),
		[&ancName](const _Ancestor& ancestor)
	{ return !compare_stringA(ancestor.struct_name, ancName, true); });

	if (itr != struc.ancestors.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" already has ancestor \"%s\"!",
			typeName.c_str(), ancName.c_str());
		return false;
	}

	size_t insertAt = -1;
	_Ancestor anc;

	anc.struct_name = ancName;
	anc.access_type = _Ancestor::ACCESS_PUBLIC;

	//Find insertion position
	if (beforeAnc.size())
	{
		itr = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(),
			[&beforeAnc](const _Ancestor& ancestor)
		{ return !compare_stringA(ancestor.struct_name, beforeAnc, true); });

		if (itr == struc.ancestors.cend())
		{
			logprintf(LL::Warning, "Ancestor \"%s\" doesn't exists!", beforeAnc.c_str());
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
		logprintf(LL::Error, "Not enough space for inserting ancestor \"%s\"!", ancName.c_str());
		return false;
	}

	if (struc.members.size() != 0)
	{
		struc.members[0].set_padding(struc.members[0].offset + itrAncestor->second.size, 
			struc.members[0].get_size() - itrAncestor->second.size);

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
		ancName.c_str(), typeName.c_str());
	return true;

}

bool CTypeHelper::structRemoveAncestor(const std::string& typeName, const std::string& ancName)
{
	auto itrStruct = m_structs.find(typeName);
	if (itrStruct == m_structs.end())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't exists!", typeName.c_str());
		return false;
	}

	_Struct& struc = itrStruct->second;
	auto itrAncestor = std::find_if(struc.ancestors.cbegin(), struc.ancestors.cend(), [&ancName](const _Ancestor& ancestor)
	{ return !compare_stringA(ancestor.struct_name, ancName, true); });

	if (itrAncestor == struc.ancestors.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't have ancestor \"%s\"!",
			typeName.c_str(), ancName.c_str());
		return false;
	}
	
	if (!_structRemoveAncestor(struc, itrAncestor))
		return false;

	logprintf(LL::Message, "Ancestor \"%s\" has been removed from struct \"%s\"!",
		ancName.c_str(), typeName.c_str());
	return true;
}

bool CTypeHelper::structAddMember(_Struct& struc, _Member& member)
{
	if (struc.members.cend() != std::find_if(struc.members.cbegin(), struc.members.cend(),
		[&member](const _Member& _member)
	{ return !compare_stringA(member.name, _member.name, true); }))
	{
		logprintf(LL::Warning, "Struct \"%s\" already has member \"%s\"!",
			struc.name.c_str(), member.name.c_str());
		return false;
	}
	
	//Add member at the end. It will increase the struct size. Those structs
	//who derived from it or have members of the struct must keep their sizes unchanged.
	//The increased size which is member.type_size can be neutralized by an immediately followed padding block.
	//If there is no such block or the block size is not enough, then it fails.

	if (member.offset < 0)
	{
		if (!_beforeExpandStruct(struc.name, member.get_size()))
			return false;

		member.offset = struc.size;
		struc.members.push_back(member);
	}
	else
	{
		//find out which member is at address arg.memberOffset
		size_t i;
		for (i = 0; i < struc.members.size(); ++i)
		{
			const _Member& m = struc.members[i];

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
		
		if (i < struc.members.size())
		{
			if (!struc.members[i].ispadding)
			{
				logprintf(LL::Error, "Can't add member inside \"%s\" at offset %p which is not a padding block!", 
					struc.members[i].name.c_str(), member.offset);
				return false;
			}

			int increased_size = member.get_next_offset() - struc.members[i].get_next_offset();
			logprintf(LL::Debug, "Increased size:%d", increased_size);

			if (increased_size > 0)
			{
				if (i + 1 < struc.members.size())
				{
					if (!struc.members[i + 1].ispadding)
					{
						logprintf(LL::Error, "Can't add member crossed with two members at offset %p. One of them is not a padding block!", 
							member.offset);
						return false;
					}

					struc.members[i].set_padding(struc.members[i].offset,
						member.offset - struc.members[i].offset);

					struc.members[i + 1].set_padding(member.get_next_offset(),
						struc.members[i + 1].get_next_offset() - member.get_next_offset());

					struc.members.insert(struc.members.cbegin() + i + 1, member);
				}
				else
				{
					if (!_beforeExpandStruct(struc.name, increased_size))
						return false;

					struc.members[i].set_padding(struc.members[i].offset,
						member.offset - struc.members[i].offset);

					struc.members.push_back(member);
				}
			}
			else
			{
				if (increased_size < 0)
				{
					_Member padding;
					padding.set_padding(member.get_next_offset(), - increased_size);
					struc.members.insert(struc.members.cbegin() + i + 1, padding);
				}

				struc.members[i].set_padding(struc.members[i].offset,
					member.offset - struc.members[i].offset);

				if (struc.members[i].array_size == 0)
					struc.members[i] = member;
				else
					struc.members.insert(struc.members.cbegin() + i + 1, member);
			}
		}
		else
		{
			int increased_size = member.get_next_offset() - struc.size;
			if (!_beforeExpandStruct(struc.name, increased_size))
				return false;

			if (struc.members.size() == 0 || !struc.members[struc.members.size() - 1].ispadding)
			{
				_Member padding;
				padding.set_padding(struc.size, member.offset - struc.size);
				struc.members.push_back(padding);
			}
			else
			{
				_Member& last = struc.members[struc.members.size() - 1];
				last.set_padding(last.offset, member.offset - last.offset);
			}
			struc.members.push_back(member);
		}
	}

	struc.size = struc.members[struc.members.size() - 1].get_next_offset();

	logprintf(LL::Message, "Member \"%s\" successfully added. Offset:%p!",
		member.name.c_str(), member.offset);
	return true;
}

bool CTypeHelper::structRemoveMember(_Struct& struc, const std::string& memberName)
{
	auto itrMember = std::find_if(struc.members.cbegin(), struc.members.cend(),
		[&memberName](const _Member& member)
	{ return !compare_stringA(member.name, memberName, true); });

	if (itrMember == struc.members.cend())
	{
		logprintf(LL::Warning, "Struct \"%s\" doesn't have member \"%s\"!",
			struc.name.c_str(), memberName.c_str());
		return false;
	}

	size_t id = itrMember - struc.members.cbegin();
	if (!_structRemoveMember(struc, id))
		return false;

	logprintf(LL::Message, "Member \"%s\" has been removed from struct \"%s\"!",
		memberName.c_str(), struc.name.c_str());
	return true;
}

void CTypeHelper::structPrint(const _Struct& struc, int offsetLen, int typeLen, int memberLen)
{
	std::string result, str;
	result.reserve(1024);

	//struct comment text
	if (struc.comment.size())
	{
		result += string_formatA("/*\n%s\nsizeof(%s):0x%x(%d)\n*/\n",
			struc.comment.c_str(), struc.name.c_str(), struc.size, struc.size);
	}
	else
	{
		result += string_formatA("//sizeof(%s):0x%x(%d)\n",
			struc.name.c_str(), struc.size, struc.size);
	}

	//struct name
	result += string_formatA("struct %s", struc.name.c_str());

	//ancestors
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

		if (offsetLen > sizeof(void*))
			offsetLen = -1;

		//Formatting offset text
		if (offsetLen < 0)
		{
			result += string_formatA("/*%p*/    ", member.offset);
		}
		else if (offsetLen > 0)
		{
			result += string_formatA(string_formatA("/*%%0%dx*/    ", offsetLen).c_str(),
				member.offset);
		}
		else
		{
			result += "    ";
		}

		//Formatting type name text
		str = member.type;
		fill_spaces(str, typeLen);
		result += str;

		//Formatting member name text
		if (member.array_size == 0)
			str = string_formatA("%s;", member.name.c_str());
		else
			str = string_formatA("%s[0x%x];", member.name.c_str(), member.array_size);

		if (set_comment)
		{
			fill_spaces(str, memberLen);
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
}


void CTypeHelper::unionPrint(const _Union& un, int offsetLen, int typeLen, int memberLen)
{
	std::string result, str;
	result.reserve(1024);

	//union comment text
	if (un.comment.size())
	{
		result += string_formatA("/*\n%s\nsizeof(%s):0x%x(%d)\n*/\n",
			un.comment.c_str(), un.name.c_str(), un.size, un.size);
	}
	else
	{
		result += string_formatA("//sizeof(%s):0x%x(%d)\n",
			un.name.c_str(), un.size, un.size);
	}

	//union name
	result += string_formatA("union %s\n{\n", un.name.c_str());
	for (const auto& member : un.members)
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

		if (offsetLen > sizeof(void*))
			offsetLen = -1;

		//Formatting offset text
		//if (offsetLen < 0)
		//{
		//	result += string_formatA("/*%p*/    ", member.offset);
		//}
		//else if (offsetLen > 0)
		//{
		//	result += string_formatA(string_formatA("/*%%0%dx*/    ", offsetLen).c_str(),
		//		member.offset);
		//}
		//else
		//{
		result += "    ";
		//}

		//Formatting type name text
		str = member.type;
		fill_spaces(str, typeLen);
		result += str;

		//Formatting member name text
		if (member.array_size == 0)
			str = string_formatA("%s;", member.name.c_str());
		else
			str = string_formatA("%s[0x%x];", member.name.c_str(), member.array_size);

		if (set_comment)
		{
			fill_spaces(str, memberLen);
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
}

bool CTypeHelper::unionRemove(UnionMap::const_iterator itrUnion)
{
	const std::string unionName = itrUnion->second.name;
	std::vector<_Struct> interim;
	for (auto& pair : m_structs)
	{
		_Struct struc = pair.second;
		bool found = false;
		for (size_t i = 0; i < struc.members.size(); ++i)
		{
			_Member& member = struc.members[i];
			if (compare_stringA(member.type, unionName, true))
				continue;

			if (!_structRemoveMember(struc, i))
				return false;

			logprintf(LL::Message, "Union \"%s\" has been removed from struct \"%s\" as member \"%s\"!",
				unionName.c_str(), struc.name.c_str(), member.name.c_str());
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

	m_unions.erase(itrUnion);
	logprintf(LL::Message, "Union \"%s\" has been removed!", unionName.c_str());

	return false;
}

bool CTypeHelper::unionAddMember(_Union& un, _Member& member)
{
	if (un.members.cend() != std::find_if(un.members.cbegin(), un.members.cend(), [&member](const _Member& _member)
	{ return !compare_stringA(member.name, _member.name, true); }))
	{
		logprintf(LL::Warning, "Union \"%s\" already has member \"%s\"!",
			un.name.c_str(), member.name.c_str());
		return false;
	}

	//Recaculating the size of union.
	int union_size = max(member.type_size, un.size);
	int expand_size = union_size - un.size;

	//Update all structs who own members of this union type
	//while the size of this union is raised.	
	if (expand_size > 0)
	{
		std::vector<_Struct> interim;
		for (auto& pair : m_structs)
		{
			_Struct struc = pair.second;
			bool found = false;
			for (size_t i = 0; i < struc.members.size(); ++i)
			{
				_Member& member = struc.members[i];
				if (compare_stringA(member.type, un.name, true))
					continue;

				if (i + 1 < struc.members.size() &&
					struc.members[i + 1].ispadding &&
					struc.members[i + 1].get_size() >= expand_size)
				{
					struc.members[i + 1].set_padding(
						struc.members[i + 1].offset + expand_size,
						struc.members[i + 1].get_size() - expand_size);

					if (struc.members[i + 1].array_size == 0)
					{
						struc.members.erase(struc.members.cbegin() + i + 1);
					}
				}
				else if (i + 1 >= struc.members.size() && !_isTypeUsed(struc.name))
				{
					//Do nothing
				}
				//The next member is not a padding block, or block size is smaller than expand_size.
				else
				{
					logprintf(LL::Error, "Not enough space for expanding member in \"%s=>%s\"!",
						struc.name.c_str(), member.name.c_str());
					return false;
				}

				member.type_size = union_size;
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
	}

	member.offset = 0;
	un.members.push_back(member);
	un.size = union_size;

	logprintf(LL::Message, "Member \"%s\" successfully added!",
		member.name.c_str());
	return true;
}

bool CTypeHelper::_beforeExpandStruct(const std::string& changedStruct, int expand_size)
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

	//Saving them.
	std::for_each(interim.begin(), interim.end(), [this](_Struct& struc)
	{ m_structs[struc.name] = std::move(struc);	});

	return true;
}

bool CTypeHelper::_structRemoveAncestor(_Struct& struc, std::vector<_Ancestor>::const_iterator itrAncestor)
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

bool CTypeHelper::_isTypeUsed(const std::string& typeName) const
{
	for (auto& pair : m_structs)
	{
		if (pair.second.ancestors.cend() !=
			std::find_if(pair.second.ancestors.cbegin(),
			pair.second.ancestors.cend(),
			[&typeName](const _Ancestor& ancestor)
		{ return !compare_stringA(ancestor.struct_name, typeName, true); }))
		{
			return true;
		}

		if (pair.second.members.cend() !=
			std::find_if(pair.second.members.cbegin(),
			pair.second.members.cend(),
			[&typeName](const _Member& member)
		{ return !compare_stringA(member.type, typeName, true); }))
		{
			return true;
		}
	}

	for (auto pair : m_unions)
	{
		if (pair.second.members.cend() !=
			std::find_if(pair.second.members.cbegin(),
			pair.second.members.cend(),
			[&typeName](const _Member& member)
		{ return !compare_stringA(member.type, typeName, true); }))
		{
			return true;
		}
	}

	return false;
}

bool CTypeHelper::_structRemoveMember(_Struct& struc, size_t& memberId)
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


bool CTypeHelper::unionRemoveMember(_Union& un, const std::string& memberName)
{
	auto itrMember = std::find_if(un.members.begin(), un.members.end(),
		[&memberName](_Member& member)
	{ return !compare_stringA(member.name, memberName, true); });

	if (itrMember == un.members.cend())
	{
		logprintf(LL::Warning, "Union \"%s\" doesn't have member \"%s\"!",
			un.name.c_str(), memberName.c_str());
		return false;
	}

	//Recaculating the size of union.
	int new_size = 0;
	for (const _Member& member : un.members)
	{
		if (!compare_stringA(member.name, memberName, true))
			continue;

		if (member.get_size() > new_size)
			new_size = member.get_size();
	}

	//Update all structs who own members of this union type
	//while the size of this union is cutdown.
	int shrink_size = un.size - new_size;
	if (shrink_size > 0)
	{
		std::vector<_Struct> interim;
		for (auto& pair : m_structs)
		{
			_Struct struc = pair.second;
			bool found = false;
			for (size_t i = 0; i < struc.members.size(); ++i)
			{
				_Member& member = struc.members[i];
				if (compare_stringA(member.type, un.name, true))
					continue;

				//The next member is a padding block.
				//Just expand it.
				if (i + 1 < struc.members.size() && struc.members[i + 1].ispadding)
				{
					struc.members[i + 1].set_padding(
						struc.members[i + 1].offset - shrink_size,
						struc.members[i + 1].get_size() + shrink_size);
				}
				//Or insert a new padding block.
				else
				{
					_Member padding;
					padding.set_padding(member.get_next_offset() - shrink_size,
						shrink_size);
					struc.members.insert(struc.members.cbegin() + i + 1, padding);
				}

				member.type_size = new_size;
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
	}

	un.members.erase(itrMember);
	un.size = new_size;

	logprintf(LL::Message, "Member \"%s\" has been removed from union \"%s\"!",
		memberName.c_str(), un.name.c_str());
	return true;
}


int CTypeHelper::_sizeOfType(const std::string& type) const
{
	auto found = m_types.find(type);
	if (found != m_types.cend())
		return found->second.size;

	auto pos = type.find('*');
	if (pos != std::string::npos)
	{
		std::string rest = trim_rightA(type.substr(pos));
		std::string typeName = trim_rightA(type.substr(0, pos));

		if (rest == "*" || rest == "**")
		{
			if (m_types.find(typeName) != m_types.cend() ||
				m_structs.find(typeName) != m_structs.cend() ||
				m_unions.find(typeName) != m_unions.cend())
				return m_primitivesizes.at(Pointer);
		}
	}
	else
	{
		auto itrStruct = m_structs.find(type);
		if (itrStruct != m_structs.cend())
			return itrStruct->second.size;

		auto itrUnion = m_unions.find(type);
		if (itrUnion != m_unions.cend())
			return itrUnion->second.size;
	}

	return -1;
}

bool CTypeHelper::_typeExists(const std::string& typeName) const
{
	return (m_structs.find(typeName) != m_structs.cend() ||
		m_unions.find(typeName) != m_unions.cend() ||
		m_types.find(typeName) != m_types.cend());
}