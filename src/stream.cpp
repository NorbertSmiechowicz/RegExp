
#include "stream.h"
#include <string.h>
#include <string_view>

////////////////////////////////////////////////
//  Stream

StreamView::StreamView( char const * string)
:
    m_Cursor{ string},
    m_BufferEnd{ string + strlen( string)}
{
};

StreamView::StreamView( char const * bufferStart, char const * bufferEnd)
:
    m_Cursor{ bufferStart},
    m_BufferEnd{ bufferEnd}
{
};

bool
StreamView::peek_char( char & outChar)
{
    if( m_Cursor < m_BufferEnd)
    {
        outChar = *m_Cursor;
        return true;
    };

    return false;
};

bool
StreamView::get_char( char & outChar)
{
    if( m_Cursor + 1 < m_BufferEnd)
    {
        outChar = *m_Cursor;
        m_Cursor ++;
        return true;
    };

    return false;
};

bool
StreamView::get_string_view( char const *& outString, size_t length)
{
    if( m_Cursor + length < m_BufferEnd)
    {
        outString = m_Cursor;
        m_Cursor += length;
        return true;
    };

    return false;
};

bool
StreamView::at_string( char const * compareTo, unsigned long length)
{
    if( m_Cursor + length < m_BufferEnd)
    {
        std::string_view currentSubString{ m_Cursor, length};
        std::string_view compareToString{ compareTo, length};

        return currentSubString == compareToString;
    }

    return false;
};

bool
StreamView::skip_chars( unsigned long count)
{
    if( m_Cursor + count < m_BufferEnd)
    {
        m_Cursor += count;
        return true;
    };

    return false;
};

StreamViewCheckpoint
StreamView::set_checkpoint()
{
    return StreamViewCheckpoint( *this);
};

////////////////////////////////////////////////
//  StreamCheckpoint

StreamViewCheckpoint::StreamViewCheckpoint( StreamView & stream)
:
    m_Stream{ stream}
{
    update();
};

StreamViewCheckpoint::~StreamViewCheckpoint()
{
    rollback();
};

bool
StreamViewCheckpoint::update()
{
    m_CommitedCursor = m_Stream.m_Cursor;
    return true;
};

void
StreamViewCheckpoint::rollback()
{
    m_Stream.m_Cursor = m_CommitedCursor;
};
