
#include "command_line_interface.h"
#include "stream.h"

#include <string.h>
#include <utility>

////////////////////////////////////////////////
#define TerminalTokenList\
    TOKEN_ID( White)                TOKEN_CHARS( " \t\r\n")\
    TOKEN_ID( SingleQuotes)         TOKEN_CHARS( "'")\
    TOKEN_ID( DoubleQuotes)         TOKEN_CHARS( "\"")\
    TOKEN_ID( Hyphen)               TOKEN_CHARS( "-")\
    TOKEN_ID( Equals)               TOKEN_CHARS( "=")

#define TOKEN_ID( X) X,
#define TOKEN_CHARS( X)

namespace TerminalTokenId
{
    enum
    {
        TerminalTokenList
        TokenCount
    };
};

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

CommandLineArgumentLexer::CommandLineArgumentLexer( int argc, char* argv[])
{
    for( int argi = 1; argi < argc; argi ++)
        m_CommandLine += argv[ argi];

};

bool
CommandLineArgumentLexer::lex( CliLexTokenList & outTokens)
{
    m_CommandLineStream = StreamView( m_CommandLine.data(), m_CommandLine.data() + m_CommandLine.size());
    m_CommandLineStream.set_white( TerminalTokenArray[ TerminalTokenId::White]);

    if( ! try_syntax_command_line())
        return false;

    outTokens = std::exchange( m_TokenList, CliLexTokenList());
    return true;
};

void
CommandLineArgumentLexer::skip_white()
{
    m_CommandLineStream.skip_white();
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

    StreamViewCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    while( true)
    {
        chckpt.update();

        if( ! try_syntax_short_argument())
            chckpt.rollback();

        if( ! try_syntax_long_argument())
            return true;

        while( true)
        {
            chckpt.update();

            if( ! try_syntax_value())
                break;
        };
    };
};

bool
CommandLineArgumentLexer::try_syntax_short_argument()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId::Hyphen))
        return false;

    if( peek_terminal( TerminalTokenId::Hyphen) || peek_terminal( TerminalTokenId::White))
        return false;

    StreamViewCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId::White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    m_TokenList.emplace_back( CliLexTokenId::ShortKeyGroup, std::move( capture));
    return chckpt.update();
};

bool
CommandLineArgumentLexer::try_syntax_long_argument()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId::Hyphen))
        return false;

    if( ! try_terminal( TerminalTokenId::Hyphen))
        return false;

    if( peek_terminal( TerminalTokenId::White))
        return false;

    StreamViewCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId::White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    m_TokenList.emplace_back( CliLexTokenId::LongKey, std::move( capture));
    return chckpt.update();
};

bool
CommandLineArgumentLexer::try_syntax_value()
{
    skip_white();

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

    if( peek_terminal( TerminalTokenId::Hyphen))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId::White))
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

    if( ! try_terminal( TerminalTokenId::SingleQuotes))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId::SingleQuotes))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    if( ! try_terminal( TerminalTokenId::SingleQuotes))
        return false;

    m_TokenList.emplace_back( CliLexTokenId::Value, std::move( capture));
    return true;
};

bool
CommandLineArgumentLexer::try_syntax_doubly_quoted_value()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId::DoubleQuotes))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId::DoubleQuotes))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    if( ! try_terminal( TerminalTokenId::DoubleQuotes))
        return false;

    m_TokenList.emplace_back( CliLexTokenId::Value, std::move( capture));
    return true;
};
