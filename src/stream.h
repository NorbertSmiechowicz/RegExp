
#ifndef _STREAM_H_
# define _STREAM_H_

class StreamViewCheckpoint;

////////////////////////////////////////////////
//  Stream

class StreamView
{
    friend class StreamViewCheckpoint;

public:
    StreamView();
    StreamView( char const * string);
    StreamView( char const * bufferStart, char const * bufferEnd);

public:
    void                    set_white( char const * charset);

    [[nodiscard]] bool      peek_char( char & outChar);
    [[nodiscard]] bool      get_char( char & outChar);
    [[nodiscard]] bool      get_string_view( char const *& outString, unsigned long length);
    [[nodiscard]] bool      at_string( char const * compareTo, unsigned long length);
    [[nodiscard]] bool      skip_char_count( unsigned long count);
    void                    skip_white();

    StreamViewCheckpoint    set_checkpoint();

private:
    char const *            m_WhiteCharSet;
    char const *            m_Cursor;
    char const *            m_BufferEnd;
};

////////////////////////////////////////////////
//  StreamViewCheckpoint

class StreamViewCheckpoint
{
public:
    StreamViewCheckpoint( StreamView & stream);
    ~StreamViewCheckpoint();

    bool                    update();
    void                    rollback();

private:
    StreamView &            m_Stream;
    char const *            m_CommitedCursor;
};


#endif//_STREAM_H_
