
#include "command_line_interface.h"

#include <string.h>
#include <utility>

////////////////////////////////////////////////
#define TerminalTokenList\
    TOKEN_ID( White)                TOKEN_CHARS( " \t\n\v\f\r")\
    TOKEN_ID( SingleQuotes)         TOKEN_CHARS( "'")\
    TOKEN_ID( DoubleQuotes)         TOKEN_CHARS( "\"")\
    TOKEN_ID( Hyphen)               TOKEN_CHARS( "-")\
    TOKEN_ID( Equals)               TOKEN_CHARS( "=")

#define TOKEN_ID( X) TerminalTokenId_##X,
#define TOKEN_CHARS( X)

typedef enum
{
    TerminalTokenList
    TokenCount
}
TerminalTokenId;

#undef TOKEN_CHARS
#undef TOKEN_ID

using TerminalCharSet = char const *;

#define TOKEN_ID( X)
#define TOKEN_CHARS( X) X,

static TerminalCharSet TerminalTokenArray[]
{
    TerminalTokenList
};

#undef TOKEN_CHARS
#undef TOKEN_ID

#undef TerminalTokenList
////////////////////////////////////////////////

bool
CommandLineArgumentLexer::lex( CliLexTokenList & outTokens, char const * commandLine, unsigned long lineLength)
{
    m_CommandLineStream = StringStream{ commandLine, lineLength};

    if( ! try_syntax_command_line())
        return false;

    outTokens = std::exchange( m_TokenList, CliLexTokenList());
    return true;
};

bool
CommandLineArgumentLexer::skip_white()
{
    unsigned long whiteCount = 0;

    while( peek_terminal( TerminalTokenId_White))
        whiteCount ++;

    return m_CommandLineStream.skip_char_count( whiteCount);
};

bool
CommandLineArgumentLexer::peek_terminal( unsigned long terminalTokenId)
{
    char currentChar;

    if( m_CommandLineStream.peek_char( currentChar))
        return strchr( TerminalTokenArray[ terminalTokenId], currentChar) != nullptr;

    return false;
};

bool
CommandLineArgumentLexer::try_terminal( unsigned long terminalTokenId)
{
    char currentChar;

    if( m_CommandLineStream.get_char( currentChar))
        return strchr( TerminalTokenArray[ terminalTokenId], currentChar) != nullptr;

    return false;
};

bool
CommandLineArgumentLexer::try_syntax_command_line()
{
    skip_white();

    StringStreamCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    while( true)
    {
        chckpt.update();

        if( ! try_syntax_short_argument())
            chckpt.rollback();

        if( ! try_syntax_long_argument())
            chckpt.rollback();

        if( ! try_syntax_value())
            return true;
    };
};

bool
CommandLineArgumentLexer::try_syntax_long_argument()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId_Hyphen))
        return false;

    if( ! try_terminal( TerminalTokenId_Hyphen))
        return false;

    if( peek_terminal( TerminalTokenId_White))
        return false;

    StringStreamCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId_White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    m_TokenList.emplace_back( CliLexTokenId::LongKey, std::move( capture));
    return chckpt.update();
};

bool
CommandLineArgumentLexer::try_syntax_short_argument()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId_Hyphen))
        return false;

    if( peek_terminal( TerminalTokenId_Hyphen) || peek_terminal( TerminalTokenId_White))
        return false;

    StringStreamCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId_White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    m_TokenList.emplace_back( CliLexTokenId::ShortKeyGroup, std::move( capture));
    return chckpt.update();
};

bool
CommandLineArgumentLexer::try_syntax_value()
{
    skip_white();

    StringStreamCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    if( try_syntax_singly_quoted_value())
        return chckpt.update();

    chckpt.rollback();

    if( try_syntax_doubly_quoted_value())
        return chckpt.update();

    chckpt.rollback();

    if( try_syntax_unquoted_value())
        return chckpt.update();

    return false;
};

bool
CommandLineArgumentLexer::try_syntax_unquoted_value()
{
    skip_white();

    if( peek_terminal( TerminalTokenId_Hyphen))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId_White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    m_TokenList.emplace_back( CliLexTokenId::Value, std::move( capture));
    return true;
};

bool
CommandLineArgumentLexer::try_syntax_singly_quoted_value()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId_SingleQuotes))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId_SingleQuotes))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    if( ! try_terminal( TerminalTokenId_SingleQuotes))
        return false;

    m_TokenList.emplace_back( CliLexTokenId::Value, std::move( capture));
    return true;
};

bool
CommandLineArgumentLexer::try_syntax_doubly_quoted_value()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId_DoubleQuotes))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId_DoubleQuotes))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    if( ! try_terminal( TerminalTokenId_DoubleQuotes))
        return false;

    m_TokenList.emplace_back( CliLexTokenId::Value, std::move( capture));
    return true;
};
