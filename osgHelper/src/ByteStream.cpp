#include <osgHelper/ByteStream.h>

#include <cstring>

namespace osgHelper
{

ByteStream::ByteStream(char* data)
	: m_data(data),
	  m_pos(0)
{
}

char* ByteStream::readString(int size)
{
	char* s = new char[size + 1];
	std::memcpy(&s[0], &m_data[m_pos], size);
	s[size] = '\0';

	m_pos += size;

	return s;
}

int ByteStream::getPos() const
{
	return m_pos;
}

}
