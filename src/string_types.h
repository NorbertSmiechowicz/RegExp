
#ifndef _STRING_TYPES_H_
# define _STRING_TYPES_H_

# ifndef _MACRO_LIB_H_
#  include "macro_lib.h"
# endif

# ifdef __cplusplus
extern "C"
{
# endif

////////////////////////////////////////////////
//  StringView - declaration

typedef struct StringView
{
    char const *        Data;
    unsigned long       Length;
}
StringView;

# define PrintFmtStringView "%.*s"
# define PrintArgStringView( sv) (sv).Length, (sv).Data

////////////////////////////////////////////////
//  StreamView - declaration

typedef struct StringStream
{
    StringView          Text;
    char const *        TextEnd;
    char const *        Cursor;
}
StringStream;

////////////////////////////////////////////////
//  StreamViewCheckpoint - declaration

typedef struct StringStreamCheckpoint
{
    StringStream *      Stream;
    char const *        CommitedCursor;
}
StringStreamCheckpoint;

////////////////////////////////////////////////
//  StringView - prototypes

StringView              StringView_create_1( char const * text, unsigned long length);
StringView              StringView_create_2( char const * text);
char *                  StringView_build_cstring( StringView sv);
void                    StringView_trim_left( StringView * sv, unsigned long length);
void                    StringView_trim_right( StringView * sv, unsigned long length);
void                    StringView_trim_space_left( StringView * sv);
void                    StringView_trim_space_right( StringView * sv);
void                    StringView_trim_space( StringView * sv);

////////////////////////////////////////////////
//  StringStream - prototypes

StringStream            StringStream_create_1( StringView text);
StringStream            StringStream_create_2( char const * text, unsigned long length);
StringStream            StringStream_create_3( char const * text);
NODISCARD bool          StringStream_peek_char( StringStream * sv, char * outChar);
NODISCARD bool          StringStream_get_char( StringStream * sv, char * outChar);
NODISCARD bool          StringStream_peek_string( StringStream * sv, StringView * outString, unsigned long length);
NODISCARD bool          StringStream_get_string( StringStream * sv, StringView * outString, unsigned long length);
NODISCARD bool          StringStream_skip_char_count( StringStream * sv, unsigned long count);
StringStreamCheckpoint  StringStream_set_checkpoint( StringStream * sv);

////////////////////////////////////////////////
//  StringStreamCheckpoint - prototypes

bool                    StringStreamCheckpoint_update( StringStreamCheckpoint * svc);
bool                    StringStreamCheckpoint_rollback( StringStreamCheckpoint * svc);

# ifdef __cplusplus
}
# endif

#endif//_STRING_TYPES_H_
