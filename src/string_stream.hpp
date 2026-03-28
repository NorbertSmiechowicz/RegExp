
#ifndef _STRING_TYPES_H_
# define _STRING_TYPES_H_

# ifndef _MACRO_LIB_H_
#  include "macro_lib.hpp"
# endif

# include <string_view>

class StringStreamCheckpoint;

////////////////////////////////////////////////
//  StringStream - definition

class StringStream
{
    friend class StringStreamCheckpoint;

public:
    StringStream( std::string_view text);
    StringStream( char const * text, unsigned long length);
    StringStream( char const * text);

    NODISCARD bool          is_exhausted();
    NODISCARD bool          peek_char( char & outChar);
    NODISCARD bool          get_char( char & outChar);
    NODISCARD bool          peek_string( std::string_view & outString, unsigned long length);
    NODISCARD bool          get_string( std::string_view & outString, unsigned long length);
    NODISCARD bool          skip_char_count( unsigned long count);
    StringStreamCheckpoint  set_checkpoint();

private:
    char const *        m_Cursor;
    char const *        m_TextEnd;
};

////////////////////////////////////////////////
//  StreamViewCheckpoint - declaration

class StringStreamCheckpoint
{
    friend class StringStream;

private:
    StringStreamCheckpoint( StringStream & ss);

public:
    bool                update();
    bool                rollback();

private:
    StringStream &      m_Stream;
    char const *        m_CommitedCursor;
};


#endif//_STRING_TYPES_H_
