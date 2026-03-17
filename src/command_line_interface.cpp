
#include "command_line_interface.h"
#include "stream.h"

#include <memory>
#include <vector>
#include <string_view>

////////////////////////////////////////////////
#define TerminalTokenList\
    TOKEN_ID( White)                TOKEN_STRINGS( " ", "\t", "\r", "\n", "\0") \
    TOKEN_ID( SingleQuotes)         TOKEN_STRINGS( "'")\
    TOKEN_ID( DoubleQuotes)         TOKEN_STRINGS( "\"")\
    TOKEN_ID( LongKeyPrefix)        TOKEN_STRINGS( "--")\
    TOKEN_ID( ShortKeyPrefix)       TOKEN_STRINGS( "-")

#define TOKEN_ID( X) X,
#define TOKEN_STRINGS( ...)

enum class TermialToken
{
    TerminalTokenList
};

#undef TOKEN_STRINGS
#undef TOKEN_ID

#define TOKEN_ID( X)
#define TOKEN_STRINGS( ...) std::vector< std::string_view>{ __VA_ARGS__},

static std::vector< std::vector< std::string_view>> const TokenArray
{
    TerminalTokenList
};

#undef TOKEN_STRINGS
#undef TOKEN_ID

#undef TerminalTokenList
////////////////////////////////////////////////

enum class LexTokenId
{
    ArgValue,
    ArgShortKeyGroup,
    ArgLongKey
};

////////////////////////////////////////////////

#define TRY_TERMINAL( X) try_terminal( SC( unsigned long, X))
#define PEEK_TERMINAL( X) (peek_terminal( SC( unsigned long, X)) >= 0)
#define PUSH_TOKEN( lexId, text) m_TokenStack.emplace_back( std::make_unique< LexedToken>( SC( unsigned long, lexId), text))

void
CommandLineArgumentLexer::skip_white()
{
    long whiteSize = -1;

    while( true)
    {
        whiteSize = peek_terminal( SC( unsigned long, TermialToken::White));

        if( whiteSize < 0)
            return;

        if( ! m_CommandLineStream.skip_chars( whiteSize))
            return;
    };

};

long
CommandLineArgumentLexer::peek_terminal( unsigned long terminalTokenId)
{
    for( std::string_view const & tokenAlt : TokenArray[ terminalTokenId])
        if( m_CommandLineStream.at_string( tokenAlt.data(), tokenAlt.size()))
            return tokenAlt.size();

    return -1;
};

bool
CommandLineArgumentLexer::try_terminal( unsigned long terminalTokenId)
{
    long terminalSize = peek_terminal( terminalTokenId);

    if( terminalSize >= 0)
        return m_CommandLineStream.skip_chars( terminalSize);

    return false;
};

bool
CommandLineArgumentLexer::try_syntax_command_line()
{
    StreamViewCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    while( true)
    {
        chckpt.update();

        if( ! try_syntax_short_argument())
            chckpt.rollback();

        if( ! try_syntax_long_argument())
            return true;

        if( PEEK_TERMINAL( TermialToken::White))
            try_syntax_value_sequence();
    };
};

bool
CommandLineArgumentLexer::try_syntax_short_argument()
{
    skip_white();

    if( ! TRY_TERMINAL( TermialToken::ShortKeyPrefix))
        return false;

    char character = 0;
    std::string capture;

    while( ! PEEK_TERMINAL( TermialToken::White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    PUSH_TOKEN( LexTokenId::ArgShortKeyGroup, capture);
    return true;
};

bool
CommandLineArgumentLexer::try_syntax_long_argument()
{
    skip_white();

    if( ! TRY_TERMINAL( TermialToken::LongKeyPrefix))
        return false;

    char character = 0;
    std::string capture;

    while( ! PEEK_TERMINAL( TermialToken::White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    PUSH_TOKEN( LexTokenId::ArgLongKey, capture);
    return true;
};

bool
CommandLineArgumentLexer::try_syntax_value_sequence()
{
    StreamViewCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    if( ! try_syntax_value())
        return true;

    while( true)
    {
        chckpt.update();

        if( ! TRY_TERMINAL( TermialToken::White))
            return true;

        skip_white();

        if( ! try_syntax_value())
            return true;
    };
};

bool
CommandLineArgumentLexer::try_syntax_value()
{
    StreamViewCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    if( try_syntax_unquoted_value())
        return chckpt.update();

    chckpt.rollback();

    if( try_syntax_singly_quoted_value())
        return chckpt.update();

    chckpt.rollback();

    if( try_syntax_doubly_quoted_value())
        return chckpt.update();

    return false;
};

bool
CommandLineArgumentLexer::try_syntax_unquoted_value()
{
    skip_white();

    char character = 0;
    std::string capture;

    while( ! PEEK_TERMINAL( TermialToken::White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    PUSH_TOKEN( LexTokenId::ArgValue, capture);
    return true;
};

bool
CommandLineArgumentLexer::try_syntax_singly_quoted_value()
{
    skip_white();

    if( ! TRY_TERMINAL( TermialToken::SingleQuotes))
        return false;

    char character = 0;
    std::string capture;

    while( ! PEEK_TERMINAL( TermialToken::SingleQuotes))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    if( ! TRY_TERMINAL( TermialToken::SingleQuotes))
        return false;

    PUSH_TOKEN( LexTokenId::ArgValue, capture);
    return true;
};

bool
CommandLineArgumentLexer::try_syntax_doubly_quoted_value()
{
    skip_white();

    if( ! TRY_TERMINAL( TermialToken::DoubleQuotes))
        return false;

    char character = 0;
    std::string capture;

    while( ! PEEK_TERMINAL( TermialToken::DoubleQuotes))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    if( ! TRY_TERMINAL( TermialToken::DoubleQuotes))
        return false;

    PUSH_TOKEN( LexTokenId::ArgValue, capture);
    return true;
};
