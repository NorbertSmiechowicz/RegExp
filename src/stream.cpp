
#include "stream.h"
#include <cstdint>

/* Stream */

Stream::Stream( char const * string) :
    m_Cursor{ reinterpret_cast< uint8_t const *>( string)},
    m_BufferEnd{ reinterpret_cast< uint8_t const *>( string + strlen( string))}
{
};

Stream::Stream( uint8_t const * bufferStart, uint8_t const * bufferEnd) :
    m_Cursor{ bufferStart},
    m_BufferEnd{ bufferEnd}
{    
};

bool
Stream::get_byte( uint8_t & outByte)
{
    if( m_Cursor < m_BufferEnd)
    {
	outByte = *(m_Cursor ++);	
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
