
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

#include <cstddef>
#include <cstring>
#include <string_view>
#include <utility>
#include <array>

////////////////////////////////////////////////
//  CommandLineLexTerminalId

#define LEX_TERMINAL_LIST_DEF\
    LEX_TERMINAL_ID( White)             LEX_TERMINAL_CHAR_SET( " \t\n\v\f\r")\
    LEX_TERMINAL_ID( SingleQuotes)      LEX_TERMINAL_CHAR_SET( "'")\
    LEX_TERMINAL_ID( DoubleQuotes)      LEX_TERMINAL_CHAR_SET( "\"")\
    LEX_TERMINAL_ID( Hyphen)            LEX_TERMINAL_CHAR_SET( "-")\

#define LEX_TERMINAL_ID( X) X,
#define LEX_TERMINAL_CHAR_SET( X)

enum class CommandLineLexTerminalId : std::size_t
{
    LEX_TERMINAL_LIST_DEF
    TerminalCount
};

#undef LEX_TERMINAL_CHAR_SET
#undef LEX_TERMINAL_ID

using CommandLineTerminalCharacterSet = char const *;

#define LEX_TERMINAL_ID( X)
#define LEX_TERMINAL_CHAR_SET( X) X,

static constexpr std::array< CommandLineTerminalCharacterSet, static_cast< std::size_t>( CommandLineLexTerminalId::TerminalCount)> CommandLineLexTerminalCharacterSets
{
    LEX_TERMINAL_LIST_DEF
};

#undef LEX_TERMINAL_CHAR_SET
#undef LEX_TERMINAL_ID

#undef LEX_TERMINAL_LIST_DEF

////////////////////////////////////////////////
//  CommandLineLexTokenId

enum class CommandLineLexTokenId : std::size_t
{
    ShortKeyGroup,
    LongKey,
    Value
};

////////////////////////////////////////////////
// CommandLineArgumentLexer - definition

CommandLineArgumentLexer::CommandLineArgumentLexer( std::string_view commandLine)
:
    m_CommandLineStream{ commandLine}
{
}

bool
CommandLineArgumentLexer::lex( std::list< CommandLineLexToken> & outTokens)
{
    outTokens.clear();
    m_LexedTokens = &outTokens;

    if( ! try_syntax_command_line())
        return false;

    return true;
}

bool
CommandLineArgumentLexer::skip_white()
{
    bool textContinues = true;

    while( peek_terminal( CommandLineLexTerminalId::White))
        textContinues &= m_CommandLineStream.skip_char_count( 1);

    return textContinues;
}

bool
CommandLineArgumentLexer::peek_terminal( CommandLineLexTerminalId terminalTokenId)
{
    char currentChar;

    if( m_CommandLineStream.peek_char( currentChar))
        return std::strchr( CommandLineLexTerminalCharacterSets[ static_cast< std::size_t>( terminalTokenId)], currentChar) != nullptr;

    return false;
}

bool
CommandLineArgumentLexer::try_terminal( CommandLineLexTerminalId terminalTokenId)
{
    char currentChar;

    if( m_CommandLineStream.get_char( currentChar))
        return std::strchr( CommandLineLexTerminalCharacterSets[ static_cast< std::size_t>( terminalTokenId)], currentChar) != nullptr;

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

    if( ! try_terminal( CommandLineLexTerminalId::Hyphen))
        return false;

    if( ! try_terminal( CommandLineLexTerminalId::Hyphen))
        return false;

    if( peek_terminal( CommandLineLexTerminalId::White))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( CommandLineLexTerminalId::White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            break;

    m_LexedTokens->emplace_back( CommandLineLexTokenId::LongKey, std::move( capture));
    return true;
}

bool
CommandLineArgumentLexer::try_syntax_short_key()
{
    skip_white();

    if( ! try_terminal( CommandLineLexTerminalId::Hyphen))
        return false;

    if( peek_terminal( CommandLineLexTerminalId::Hyphen) || peek_terminal( CommandLineLexTerminalId::White))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( CommandLineLexTerminalId::White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            break;

    m_LexedTokens->emplace_back( CommandLineLexTokenId::ShortKeyGroup, std::move( capture));
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

    if( peek_terminal( CommandLineLexTerminalId::Hyphen))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( CommandLineLexTerminalId::White))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            break;

    m_LexedTokens->emplace_back( CommandLineLexTokenId::Value, std::move( capture));
    return true;
}

bool
CommandLineArgumentLexer::try_syntax_singly_quoted_value()
{
    skip_white();

    if( ! try_terminal( CommandLineLexTerminalId::SingleQuotes))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( CommandLineLexTerminalId::SingleQuotes))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    if( ! try_terminal( CommandLineLexTerminalId::SingleQuotes))
        return false;

    m_LexedTokens->emplace_back( CommandLineLexTokenId::Value, std::move( capture));
    return true;
}

bool
CommandLineArgumentLexer::try_syntax_doubly_quoted_value()
{
    skip_white();

    if( ! try_terminal( CommandLineLexTerminalId::DoubleQuotes))
        return false;

    char character = 0;
    std::string capture;

    while( ! peek_terminal( CommandLineLexTerminalId::DoubleQuotes))
        if( m_CommandLineStream.get_char( character))
            capture += character;
        else
            return false;

    if( ! try_terminal( CommandLineLexTerminalId::DoubleQuotes))
        return false;

    m_LexedTokens->emplace_back( CommandLineLexTokenId::Value, std::move( capture));
    return true;
}

////////////////////////////////////////////////
// CommandLineArgumentDictBase - definition

CommandLineArgumentDictBase::const_iterator
CommandLineArgumentDictBase::end() const
{
    return nullptr;
}

CommandLineArgumentDictBase::const_iterator
CommandLineArgumentDictBase::get_process_value() const
{
    if( ! m_ProcessValue.empty())
        return &m_ProcessValue;

    return end();
}

void
CommandLineArgumentDictBase::clear()
{
    m_Dict.clear();
    m_ProcessValue.clear();
}

CommandLineArgumentDictBase::const_iterator
CommandLineArgumentDictBase::do_get_argument_value( CommandLineArgumentId argId) const
{
    InnerDict::const_iterator entry = m_Dict.find( argId);

    return entry != m_Dict.end() ? &entry->second : nullptr;
}

CommandLineArgumentDictBase::value_type &
CommandLineArgumentDictBase::add_empty_key( CommandLineArgumentId argId)
{
    InnerDict::iterator entry = m_Dict.find( argId);

    if( entry == m_Dict.end())
        entry = m_Dict.emplace( argId, value_type{}).first;

    return entry->second;
}

void
CommandLineArgumentDictBase::add_key_value( CommandLineArgumentId argId, std::string_view value)
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
//  CommandLineArgumentIdLookupTable

bool
CommandLineArgumentIdLookupTable::lookup_long_key( CommandLineArgumentId & outArgId, std::string_view key) const
{
    InnerDict::const_iterator keyValPair = m_LongKeyLookup.find( key);

    if( keyValPair == m_LongKeyLookup.end())
        return false;

    outArgId = keyValPair->second;
    return true;
}

bool
CommandLineArgumentIdLookupTable::lookup_short_key( CommandLineArgumentId & outArgId, std::string_view key) const
{
    InnerDict::const_iterator keyValPair = m_ShortKeyLookup.find( key);

    if( keyValPair == m_ShortKeyLookup.end())
        return false;

    outArgId = keyValPair->second;
    return true;
}

bool
CommandLineArgumentIdLookupTable::load_argument_template( CommandLineArgumentTemplateBase const & argTemplate)
{
    bool loadOk = true;

    loadOk &= register_long_key( argTemplate.m_LongKey, argTemplate.m_Id);
    loadOk &= register_short_key( argTemplate.m_ShortKey, argTemplate.m_Id);

    if( ! loadOk)
        std::printf( "Failed to load argument template for option: [ -" FMT_STR_VIEW " / --" FMT_STR_VIEW " ]\n",
            PRINT_STR_VIEW( argTemplate.m_ShortKey),
            PRINT_STR_VIEW( argTemplate.m_LongKey)
        );

    return loadOk;
}

bool
CommandLineArgumentIdLookupTable::register_long_key( std::string_view key, CommandLineArgumentId argId)
{
    if( key.empty())
        return true;

    InnerDict::iterator duplicateEntry = m_LongKeyLookup.find( key);

    if( duplicateEntry != m_LongKeyLookup.end())
    {
        if( duplicateEntry->second == argId)
        {
            return true;
        }
        else
        {
            std::printf( "Command line long argument key conflict for options: %zu, %zu\n", argId, duplicateEntry->second);
            return false;
        }
    }

    m_LongKeyLookup[ key] = argId;
    return true;
}

bool
CommandLineArgumentIdLookupTable::register_short_key( std::string_view key, CommandLineArgumentId argId)
{
    if( key.empty())
        return true;

    InnerDict::iterator duplicateEntry = m_ShortKeyLookup.find( key);

    if( duplicateEntry != m_ShortKeyLookup.end())
    {
        if( duplicateEntry->second == argId)
        {
            return true;
        }
        else
        {
            std::printf( "Command line short argument key conflict for options: %zu, %zu\n", argId, duplicateEntry->second);
            return false;
        }
    }

    m_ShortKeyLookup[ key] = argId;
    return true;
}

////////////////////////////////////////////////
//  CommandLineArgumentParserBase - definition

CommandLineArgumentParserBase::CommandLineArgumentParserBase( int argc, char const * argv[], CommandLineArgumentIdLookupTable && argLookupTable)
:
    m_ArgCount{ argc},
    m_ArgValues{ argv},
    m_ArgIdLookupTable{ std::move( argLookupTable)}
{
}

bool
CommandLineArgumentParserBase::lex()
{
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
CommandLineArgumentParserBase::do_parse( CommandLineArgumentDictBase & outDict)
{
    if( ! lex())
        return false;

    outDict.clear();
    m_BuildDict = &outDict;

    bool allOk = true;

    for( CommandLineLexToken & lexToken : m_LexTokenList)
    {
        switch( lexToken.m_Id)
        {
        case CommandLineLexTokenId::LongKey:
            allOk &= try_parse_long_key( lexToken.m_Text);
            break;

        case CommandLineLexTokenId::ShortKeyGroup:
            allOk &= try_parse_short_key_group( lexToken.m_Text);
            break;

        case CommandLineLexTokenId::Value:
            allOk &= try_parse_value( lexToken.m_Text);
            break;
        }
    }

    return allOk;
}

bool
CommandLineArgumentParserBase::try_parse_long_key( std::string_view longKey)
{
    if( CommandLineArgumentId argId; m_ArgIdLookupTable.lookup_long_key( argId, longKey))
    {
        m_BuildDict->add_empty_key( argId);
        m_ActiveKeyArgId = argId;
        return true;
    }

    m_ActiveKeyArgId.reset();
    return false;
}

bool
CommandLineArgumentParserBase::try_parse_short_key( std::string_view shortKey)
{
    if( CommandLineArgumentId argId; m_ArgIdLookupTable.lookup_short_key( argId, shortKey))
    {
        m_BuildDict->add_empty_key( argId);
        m_ActiveKeyArgId = argId;
        return true;
    }

    m_ActiveKeyArgId.reset();
    return false;
}

bool
CommandLineArgumentParserBase::try_parse_short_key_group( std::string_view shortKeyGroup)
{
    StringStream ss{ shortKeyGroup};
    std::string_view potentialKey{};
    CommandLineArgumentId argId{};
    bool allOk = true;

    unsigned long keyLen{ 1};

    while( ss.peek_string( potentialKey, keyLen))
        if( ! m_ArgIdLookupTable.lookup_short_key( argId, potentialKey))
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

    if( m_ActiveKeyArgId.has_value())
        m_BuildDict->add_key_value( m_ActiveKeyArgId.value(), value);
    else
        m_BuildDict->add_process_value( value);

    return true;
}
