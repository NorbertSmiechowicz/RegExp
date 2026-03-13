
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
    Stream( uint8_t const * bufferStart, uint8_t const * bufferEnd);

public:
    bool 			get_byte( uint8_t & outByte);    
    StreamCheckpoint		set_checkpoint();
    
private:
    uint8_t const *		m_Cursor;
    uint8_t const * const	m_BufferEnd;
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
    uint8_t const *		m_CommitedCursor;
};


#endif//_STREAM_H_
