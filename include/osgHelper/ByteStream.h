#pragma once

#include <iostream>

namespace osgHelper
{
	class ByteStream
	{
	public:
		ByteStream(char* data);

		template <class T>
		T read()
		{
			T v;
			memcpy(&v, &m_data[m_pos], sizeof(T));
			m_pos += sizeof(T);

			return v;
		}

		char* readString(int size);

		int getPos() const;

	private:
		char* m_data;
		int   m_pos;
	};
}