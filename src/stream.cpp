
#include "stream.h"
#include <string_view>

/* Stream */

Stream::Stream( char const * string) :
    m_Cursor{ string},
    m_BufferEnd{ string + strlen( string)}
{
};

Stream::Stream( char const * bufferStart, char const * bufferEnd) :
    m_Cursor{ bufferStart},
    m_BufferEnd{ bufferEnd}
{
};

bool
Stream::get_char( char & outByte)
{
    if( m_Cursor < m_BufferEnd)
    {
	outByte = *(m_Cursor ++);
	return true;
    };

    return false;
};

bool
Stream::get_string( std::string_view & outString, size_t length)
{
    if( m_Cursor + length < m_BufferEnd)
    {
	outString = std::string_view( m_Cursor, length);
	m_Cursor += length;
	return true;
    };

    return false;
};

StreamCheckpoint
Stream::set_checkpoint()
{
    return StreamCheckpoint( *this);
};

/* StreamCheckpoint */

StreamCheckpoint::StreamCheckpoint( Stream & stream) :
    m_Stream{ stream}
{
    commit();
};

StreamCheckpoint::~StreamCheckpoint()
{
    rollback();
};

void
StreamCheckpoint::commit()
{
    m_CommitedCursor = m_Stream.m_Cursor;
};

void
StreamCheckpoint::rollback()
{
    m_Stream.m_Cursor = m_CommitedCursor;
};
