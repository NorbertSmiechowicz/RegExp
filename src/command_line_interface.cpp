
/*
    Simple Command Line Interface Argument Parser
    Copyright (C) 2026 Norbert Śmiechowicz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "command_line_interface.hpp"
#include "string_stream.hpp"

#include <string.h>
#include <string_view>
#include <utility>

////////////////////////////////////////////////
#define TerminalTokenList\
    TOKEN_ID( White)                TOKEN_CHARS( " \t\n\v\f\r")\
    TOKEN_ID( SingleQuotes)         TOKEN_CHARS( "'")\
    TOKEN_ID( DoubleQuotes)         TOKEN_CHARS( "\"")\
    TOKEN_ID( Hyphen)               TOKEN_CHARS( "-")\

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
// CommandLineArgumentLexer - definition

CommandLineArgumentLexer::CommandLineArgumentLexer( std::string_view commandLine)
:
    m_CommandLineStream{ commandLine}
{
}

bool
CommandLineArgumentLexer::lex( CliLexTokenList & outTokens)
{
    if( ! try_syntax_command_line())
        return false;

    outTokens = std::exchange( m_TokenList, CliLexTokenList{});
    return true;
}

bool
CommandLineArgumentLexer::skip_white()
{
    bool textContinues = true;

    while( peek_terminal( TerminalTokenId_White))
        textContinues &= m_CommandLineStream.skip_char_count( 1);

    return textContinues;
}

bool
CommandLineArgumentLexer::peek_terminal( unsigned long terminalTokenId)
{
    char currentChar;

    if( m_CommandLineStream.peek_char( currentChar))
        return strchr( TerminalTokenArray[ terminalTokenId], currentChar) != nullptr;

    return false;
}

bool
CommandLineArgumentLexer::try_terminal( unsigned long terminalTokenId)
{
    char currentChar;

    if( m_CommandLineStream.get_char( currentChar))
        return strchr( TerminalTokenArray[ terminalTokenId], currentChar) != nullptr;

    return false;
}

bool
CommandLineArgumentLexer::try_syntax_command_line()
{
    skip_white();

    StringStreamCheckpoint chckpt = m_CommandLineStream.set_checkpoint();

    while( ! m_CommandLineStream.is_exhausted())
    {
        chckpt.update();

        if( ! try_syntax_long_key())
            chckpt.rollback();
        else
            continue;

        if( ! try_syntax_short_key())
            chckpt.rollback();
        else
            continue;

        if( ! try_syntax_value())
            return false;
    }

    return true;
}

bool
CommandLineArgumentLexer::try_syntax_long_key()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId_Hyphen))
        return false;

    if( ! try_terminal( TerminalTokenId_Hyphen))
        return false;

    if( peek_terminal( TerminalTokenId_White))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId_White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            break;

    m_TokenList.emplace_back( CliLexTokenId::LongKey, std::move( capture));
    return true;
}

bool
CommandLineArgumentLexer::try_syntax_short_key()
{
    skip_white();

    if( ! try_terminal( TerminalTokenId_Hyphen))
        return false;

    if( peek_terminal( TerminalTokenId_Hyphen) || peek_terminal( TerminalTokenId_White))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( TerminalTokenId_White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            break;

    m_TokenList.emplace_back( CliLexTokenId::ShortKeyGroup, std::move( capture));
    return true;
}

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

    chckpt.update();
    return false;
}

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
            break;

    m_TokenList.emplace_back( CliLexTokenId::Value, std::move( capture));
    return true;
}

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
}

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
}

////////////////////////////////////////////////
// CommandLineArgumentDictBase - definition

CommandLineArgumentDictBase::iterator
CommandLineArgumentDictBase::end()
{
    return nullptr;
}

CommandLineArgumentDictBase::value_type const &
CommandLineArgumentDictBase::get_process_value()
{
    return m_ProcessValue;
}

CommandLineArgumentDictBase::iterator
CommandLineArgumentDictBase::base_get_key_value( InternalArgIdT argId)
{
    ContainerT::iterator entry = m_Dict.find( argId);

    return entry != m_Dict.end() ? &entry->second : nullptr;
}

CommandLineArgumentDictBase::value_type &
CommandLineArgumentDictBase::add_empty_key( InternalArgIdT argId)
{
    ContainerT::iterator entry = m_Dict.find( argId);

    if( entry == m_Dict.end())
        entry = m_Dict.emplace( argId, value_type{}).first;

    return entry->second;
}

void
CommandLineArgumentDictBase::add_key_value( InternalArgIdT argId, std::string_view value)
{
    value_type & entry = add_empty_key( argId);

    entry.emplace_back( value);
}

void
CommandLineArgumentDictBase::add_process_value( std::string_view value)
{
    m_ProcessValue.emplace_back( value);
}

////////////////////////////////////////////////
//  CommandLineArgumentParserBase - definition

CommandLineArgumentParserBase::CommandLineArgumentParserBase( int argc, char * argv[])
:
    m_ArgCount{ argc},
    m_ArgValues{ argv}
{
}

bool
CommandLineArgumentParserBase::base_parse( CommandLineArgumentDictBase & outDict)
{
    if( ! lex())
        return false;

    m_BuildDict = &outDict;

    bool allOk = true;

    for( CliLexToken & lexToken : m_LexTokenList)
    {
        switch( lexToken.Id)
        {
        case CliLexTokenId::LongKey:
            allOk &= try_parse_long_key( lexToken.Text);
            break;

        case CliLexTokenId::ShortKeyGroup:
            allOk &= try_parse_short_key_group( lexToken.Text);
            break;

        case CliLexTokenId::Value:
            allOk &= try_parse_value( lexToken.Text);
            break;
        }
    }

    return allOk;
}

bool
CommandLineArgumentParserBase::lex()
{
    m_LexTokenList.clear();
    std::string commandLine{ ""};

    for( int argi = 1; argi < m_ArgCount; argi ++)
    {
        commandLine += m_ArgValues[ argi];
        commandLine += ' ';
    }

    CommandLineArgumentLexer lexer{ commandLine};

    return lexer.lex( m_LexTokenList);
}

bool
CommandLineArgumentParserBase::load_argument_template( CommandLineArgumentTemplateBase const & argTemplate)
{
    bool result = true;

    result &= register_lookup( m_LongKeyLookup, argTemplate.m_LongKey, argTemplate.m_Id);
    result &= register_lookup( m_ShortKeyLookup, argTemplate.m_ShortKey, argTemplate.m_Id);

    return result;
}

bool
CommandLineArgumentParserBase::register_lookup( KeyMap & map, std::string_view key, InternalArgIdT argId)
{
    if( key.empty())
        return true;

    KeyMap::iterator duplicateEntry = map.find( key);

    if( duplicateEntry != map.end())
    {
        if( duplicateEntry->second == argId)
            return true;
        else
            return false;
    }

    map[ key] = argId;
    return true;
}

void
CommandLineArgumentParserBase::clear_key_lookup_tables()
{
    m_LongKeyLookup.clear();
    m_ShortKeyLookup.clear();
}

bool
CommandLineArgumentParserBase::try_parse_long_key( std::string_view longKey)
{
    if( InternalArgIdT argId; lookup_long_key( argId, longKey))
    {
        m_BuildDict->add_empty_key( argId);
        m_LastKey = argId;
        return true;
    }

    m_LastKey.reset();
    return false;
}

bool
CommandLineArgumentParserBase::try_parse_short_key( std::string_view shortKey)
{
    if( InternalArgIdT argId; lookup_short_key( argId, shortKey))
    {
        m_BuildDict->add_empty_key( argId);
        m_LastKey = argId;
        return true;
    }

    m_LastKey.reset();
    return false;
}

bool
CommandLineArgumentParserBase::try_parse_short_key_group( std::string_view shortKeyGroup)
{
    StringStream ss{ shortKeyGroup};
    std::string_view potentialKey{};
    InternalArgIdT argId{};
    bool allOk = true;

    unsigned long keyLen{ 1};

    while( ss.peek_string( potentialKey, keyLen))
        if( ! lookup_short_key( argId, potentialKey))
        {
            keyLen --;

            if( ss.get_string( potentialKey, keyLen))
                allOk &= try_parse_short_key( potentialKey);

            keyLen = 1;
        }
        else
        {
            keyLen ++;
        }

    keyLen --;

    if( ss.get_string( potentialKey, keyLen))
        allOk &= try_parse_short_key( potentialKey);

    return allOk;
}

bool
CommandLineArgumentParserBase::try_parse_value( std::string_view value)
{
    if( value.empty())
        return false;

    if( m_LastKey.has_value())
        m_BuildDict->add_key_value( m_LastKey.value(), value);
    else
        m_BuildDict->add_process_value( value);

    return true;
}

bool
CommandLineArgumentParserBase::lookup_long_key( InternalArgIdT & outArgId, std::string_view key)
{
    KeyMap::iterator keyValPair = m_LongKeyLookup.find( key);

    if( keyValPair == m_LongKeyLookup.end())
        return false;

    outArgId = keyValPair->second;
    return true;
}

bool
CommandLineArgumentParserBase::lookup_short_key( InternalArgIdT & outArgId, std::string_view key)
{
    KeyMap::iterator keyValPair = m_ShortKeyLookup.find( key);

    if( keyValPair == m_ShortKeyLookup.end())
        return false;

    outArgId = keyValPair->second;
    return true;
}
