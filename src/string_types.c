
#include "string_types.h"

#include "string.h"
#include "stdlib.h"
#include "ctype.h"

////////////////////////////////////////////////
//  StringView - definitions

StringView
StringView_create_1( char const * text, unsigned long length)
{
    StringView view =
    {
        .Data = text,
        .Length = length
    };

    return view;
}

StringView
StringView_create_2( char const * text)
{
    return StringView_create_1( text, strlen( text));
}

char *
StringView_build_cstring( StringView sv)
{
    unsigned long bufSize = sv.Length * sizeof( sv.Data);

    char * string = malloc( bufSize + 1);

    if( string == NULL)
        return NULL;

    memcpy( string, sv.Data, bufSize);

    string[ bufSize] = '\0';
    return string;
}

void
StringView_trim_left( StringView * sv, unsigned long length)
{
    unsigned long trimBy = sv->Length < length ? sv->Length : length;
    sv->Data += trimBy;
}

void
StringView_trim_right( StringView * sv, unsigned long length)
{
    unsigned long trimBy = sv->Length < length ? sv->Length : length;
    sv->Length -= trimBy;
}

void
StringView_trim_space_left( StringView * sv)
{
    char const * end = sv->Data + sv->Length;
    char const * iter = sv->Data;

    for( ; iter < end && isspace( *iter); iter ++)

    sv->Data += end - iter;
}

void
StringView_trim_space_right( StringView * sv)
{
    char const * reverseIter = sv->Data + sv->Length - 1;
    char const * reverseEnd = sv->Data - 1;

    for( ; reverseIter > reverseEnd && isspace( *reverseIter); reverseIter --);

    sv->Length -= reverseIter - reverseEnd;
}

void
StringView_trim_space( StringView * sv)
{
    StringView_trim_space_left( sv);
    StringView_trim_space_right( sv);
}

////////////////////////////////////////////////
//  StringStream - definitions

StringStream
StringStream_create_1( StringView text)
{
    StringStream stream =
    {
        .Text = text,
        .TextEnd = text.Data + text.Length,
        .Cursor = text.Data
    };

    return stream;
}

StringStream
StringStream_create_2( char const * text, unsigned long length)
{
    return StringStream_create_1( StringView_create_1( text, length));
}

StringStream
StringStream_create_3( char const * text)
{
    return StringStream_create_1( StringView_create_2( text));
}

bool
StringStream_peek_char( StringStream * RESTRICT sv, char * RESTRICT outChar)
{
    if( sv->Cursor < sv->TextEnd)
    {
        *outChar = *(sv->Cursor);
        return true;
    };

    return false;
}

bool
StringStream_get_char( StringStream * RESTRICT sv, char * RESTRICT outChar)
{
    if( sv->Cursor < sv->TextEnd)
    {
        *outChar = *(sv->Cursor);
        sv->Cursor ++;
        return true;
    };

    return false;
}

bool
StringStream_peek_string( StringStream * RESTRICT sv, StringView * RESTRICT outString, unsigned long length)
{
    if( sv->Cursor + length <= sv->TextEnd)
    {
        *outString = StringView_create_1( sv->Cursor, length);
        return true;
    };

    return false;
}

bool
StringStream_get_string( StringStream * RESTRICT sv, StringView * RESTRICT outString, unsigned long length)
{
    if( sv->Cursor + length <= sv->TextEnd)
    {
        *outString = StringView_create_1( sv->Cursor, length);
        sv->Cursor += length;
        return true;
    };

    return false;
}

bool
StringStream_skip_char_count( StringStream * sv, unsigned long count)
{
    if( sv->Cursor + count <= sv->TextEnd)
    {
        sv->Cursor += count;
        return true;
    };

    return false;
}

StringStreamCheckpoint
StringStream_set_checkpoint( StringStream * sv)
{
    StringStreamCheckpoint chckpt =
    {
        .Stream = sv,
        .CommitedCursor = sv->Cursor
    };

    return chckpt;
}

////////////////////////////////////////////////
//  StringStreamCheckpoint - definitions

bool
StringStreamCheckpoint_update( StringStreamCheckpoint * svc)
{
    svc->CommitedCursor = svc->Stream->Cursor;
    return true;
}

bool
StringStreamCheckpoint_rollback( StringStreamCheckpoint * svc)
{
    svc->Stream->Cursor = svc->CommitedCursor;
    return false;
}
