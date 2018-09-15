#pragma once

template<typename T>
class Memory
{
public:
	explicit Memory()
	{
		m_Ptr = nullptr;
		m_Size = 0;
	}

	explicit Memory(size_t Size)
	{
		m_Ptr = reinterpret_cast<T>(Script::Misc::Alloc(Size));
		m_Size = Size;
		memset(m_Ptr, 0, Size);
	}

	~Memory()
	{
		free();
	}

	void free()
	{
		if (m_Ptr)
		{
			Script::Misc::Free(m_Ptr);
			m_Ptr = nullptr;
		}
	}

	T realloc(size_t Size)
	{
		if (m_Ptr)
			Script::Misc::Free(m_Ptr);

		m_Ptr = reinterpret_cast<T>(Script::Misc::Alloc(Size));
		m_Size = Size;
		return (T)memset(m_Ptr, 0, m_Size);
	}

	size_t size() const
	{
		return m_Size;
	}

	T operator()()
	{
		return m_Ptr;
	}

private:
	T           m_Ptr;
	size_t      m_Size;
};