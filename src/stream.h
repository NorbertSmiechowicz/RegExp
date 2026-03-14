
#ifndef _STREAM_H_
# define _STREAM_H_

# ifndef _EXTERNAL_LIBRARIES_H_
#  include "extlib.h"
# endif//_EXTERNAL_LIBRARIES_H_

class StreamCheckpoint;

/* Stream */

class Stream
{
    friend class StreamCheckpoint;

public:
    Stream( char const * string);
    Stream( char const * bufferStart, char const * bufferEnd);

public:
    bool 			get_char( char & outByte);
    bool 			get_string( std::string_view & outString, size_t length);
    StreamCheckpoint		set_checkpoint();

private:
    char const *		m_Cursor;
    char const * const		m_BufferEnd;
};

/* Streamcheckpoint */

class StreamCheckpoint
{
public:
    StreamCheckpoint( Stream & stream);
    ~StreamCheckpoint();

    void			commit();
    void			rollback();

private:
    Stream &			m_Stream;
    char const *		m_CommitedCursor;
};


#endif//_STREAM_H_
