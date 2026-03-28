
#include "string_stream.hpp"

#include "string.h"
#include "stdlib.h"

////////////////////////////////////////////////
//  StringStream - definitions

StringStream::StringStream( char const * text, unsigned long length)
:
    m_Cursor{ text},
    m_TextEnd{ text + length}
{
}

StringStream::StringStream( char const * text)
:
    StringStream( std::string_view( text, strlen( text)))
{
}

StringStream::StringStream( std::string_view text)
:
    m_Cursor{ text.data()},
    m_TextEnd{ text.data() + text.length()}
{
}

bool
StringStream::is_exhausted()
{
    return m_Cursor >= m_TextEnd;
}

bool
StringStream::peek_char( char & outChar)
{
    if( ! is_exhausted())
    {
        outChar = *m_Cursor;
        return true;
    };

    return false;
}

bool
StringStream::get_char( char & outChar)
{
    if( ! is_exhausted())
    {
        outChar = *m_Cursor;
        m_Cursor ++;
        return true;
    };

    return false;
}

bool
StringStream::peek_string( std::string_view & outString, unsigned long length)
{
    if( m_Cursor + length <= m_TextEnd)
    {
        outString = std::string_view{ m_Cursor, length};
        return true;
    };

    return false;
}

bool
StringStream::get_string( std::string_view & outString, unsigned long length)
{
    if( m_Cursor + length <= m_TextEnd)
    {
        outString = std::string_view( m_Cursor, length);
        m_Cursor += length;
        return true;
    };

    return false;
}

bool
StringStream::skip_char_count( unsigned long count)
{
    if( m_Cursor + count <= m_TextEnd)
    {
        m_Cursor += count;
        return true;
    };

    return false;
}

StringStreamCheckpoint
StringStream::set_checkpoint()
{
    return StringStreamCheckpoint{ *this};
}

////////////////////////////////////////////////
//  StringStreamCheckpoint - definitions

StringStreamCheckpoint::StringStreamCheckpoint( StringStream & ss)
:
    m_Stream{ ss},
    m_CommitedCursor{ ss.m_Cursor}
{
};

bool
StringStreamCheckpoint::update()
{
    m_CommitedCursor = m_Stream.m_Cursor;
    return true;
}

bool
StringStreamCheckpoint::rollback()
{
    m_Stream.m_Cursor = m_CommitedCursor;
    return false;
}
