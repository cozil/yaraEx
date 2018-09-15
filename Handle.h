#pragma once


#include <windows.h>

class Handle
{
public:
	Handle(HANDLE h = nullptr)
	{
		mHandle = h;
	}

	~Handle()
	{
		Close();
	}

	void Close()
	{
		if (mHandle != INVALID_HANDLE_VALUE)
		{
			DWORD dwFlags = 0;
			if (GetHandleInformation(mHandle, &dwFlags) && !(dwFlags & HANDLE_FLAG_PROTECT_FROM_CLOSE))
				CloseHandle(mHandle);
			mHandle = INVALID_HANDLE_VALUE;
		}
	}

	const HANDLE & operator=(const HANDLE & h)
	{
		return mHandle = h;
	}

	operator HANDLE & ()
	{
		return mHandle;
	}

	bool operator!() const
	{
		return (!mHandle || mHandle == INVALID_HANDLE_VALUE);
	}

	operator bool() const
	{
		return !this;
	}

private:
	HANDLE mHandle;
};

class FileHandle
{
public:
	FILE * m_file;
	FileHandle(FILE * fp = nullptr) :m_file(fp){}
	~FileHandle() { if (m_file) fclose(m_file); }

	const FILE * operator=(FILE * fp)
	{
		m_file = fp;
		return m_file;
	}

	operator FILE * ()
	{
		return m_file;
	}
};