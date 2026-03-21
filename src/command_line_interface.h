
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

#ifndef _CLIPARSER_H_
# define _CLIPARSER_H_

#include <concepts>
#include <memory>
#include <unordered_map>
#include <vector>

# ifndef _CONCEPTS_H_
#  include "concepts.h"
# endif//_CONCEPTS_H_

# ifndef _STRING_TYPES_H_
#  include "string_stream.h"
# endif//_STRING_TYPES_H_

# include <list>
# include <string>
# include <string_view>
# include <ranges>
# include <optional>

/**
    Short key groups are resolved from left to right by the longest valid short key at point.
    All values are bound to the rightmost preceeding key or the program itself in case no key was specified.
    All rules allow any numer of preceeding whitespace.

    00. command_line            :=  (  long_key  |  short_key_group  |  value  )*

    01. long_key                :=  <hyphen>  <hyphen>  [[  <!white>+  ]]

    02. short_key_group         :=  <hyphen>  [[ !(  <white>  |  <hyphen>  )  <!white>*  ]]

    03. value                   :=  singly_quoted_value  |  doubly_quoted_value  |  unquoted_value

    04. singly_quoted_value     :=  <single_quotes> [[  !<single_quotes>* ]]  <single_quotes>

    05. doubly_quoted_value     :=  <double_quotes> [[  !<double_quotes>* ]]  <double_quotes>

    06. unquoted_value          :=  [[ !(  <white>  |  <hyphen>  )  !<white>*  ]]
*/

////////////////////////////////////////////////

enum class CliLexTokenId
{
    ShortKeyGroup,
    LongKey,
    Value
};

struct CliLexToken
{
    CliLexTokenId   Id;
    std::string     Text;
};

using CliLexTokenList = std::list< CliLexToken>;

////////////////////////////////////////////////
//  CommandLineArgumentLexer - declaration

class CommandLineArgumentLexer
{
public:
    CommandLineArgumentLexer( std::string_view commandLine);

    bool lex( CliLexTokenList & outTokens);

private:
    bool skip_white();

    bool peek_terminal( unsigned long terminalTokenId);
    bool try_terminal( unsigned long terminalTokenId);

    bool try_syntax_command_line();
    bool try_syntax_flag_group();
    bool try_syntax_short_argument();
    bool try_syntax_long_argument();
    bool try_syntax_value();
    bool try_syntax_unquoted_value();
    bool try_syntax_singly_quoted_value();
    bool try_syntax_doubly_quoted_value();

    StringStream        m_CommandLineStream;
    CliLexTokenList     m_TokenList;
};

////////////////////////////////////////////////
//  CommandLineArgumentTemplateBase - declaration

class CommandLineArgumentTemplateBase
{
    friend class CommandLineArgumentParserBase;

public:
    using InternalArgIdT = unsigned long;

protected:
    CommandLineArgumentTemplateBase( InternalArgIdT id) : m_Id{ id} {};

    std::string_view    m_ShortKey;
    std::string_view    m_LongKey;
    std::string_view    m_ShortHelp;
    std::string_view    m_VerboseHelp;
    InternalArgIdT      m_Id;
};

////////////////////////////////////////////////
//  CommandLineArgumentDictBase - declaration

class CommandLineArgumentDictBase
{
protected:
    friend class CommandLineArgumentParserBase;

    using ValueT = std::list< std::string>;
    using InternalArgIdT = CommandLineArgumentTemplateBase::InternalArgIdT;
    using ContainerT = std::unordered_map< unsigned long, ValueT>;

public:
    ValueT const &  get_process_value();

protected:
    ValueT *        base_get_key_value( InternalArgIdT argId);
    void            add_empty_key( InternalArgIdT argId);
    void            add_key_value( InternalArgIdT argId, std::string_view value);
    void            add_process_value( std::string_view value);

    ContainerT      m_Dict;
    ValueT          m_ProcessValue;
};

////////////////////////////////////////////////
//  CommandLineArgumentParserBase - declaration

class CommandLineArgumentParserBase
{
protected:
    using InternalArgIdT = CommandLineArgumentTemplateBase::InternalArgIdT;
    using KeyMap = std::unordered_map< std::string_view, InternalArgIdT>;

    CommandLineArgumentParserBase( int argc, char * argv[]);

    bool base_parse( CommandLineArgumentDictBase & outDict);
    bool lex();

    bool load_argument_template( CommandLineArgumentTemplateBase const & argTemplate);
    bool register_lookup( KeyMap & map, std::string_view key, InternalArgIdT argId);
    void clear_key_lookup_tables();

    bool try_parse_long_key( std::string_view key);
    bool try_parse_short_key_group( std::string_view keyGroup);
    bool try_parse_value( std::string_view value);

    bool lookup_short_key( InternalArgIdT & outArgId, std::string_view key);
    bool lookup_long_key( InternalArgIdT & outArgId, std::string_view key);

    int                             m_ArgCount;
    char **                         m_ArgValues;
    KeyMap                          m_LongKeyLookup;
    KeyMap                          m_ShortKeyLookup;
    CliLexTokenList                 m_LexTokenList;
    std::optional< InternalArgIdT>  m_LastKey;
    CommandLineArgumentDictBase *   m_BuildDict;
};

////////////////////////////////////////////////
//  Kraina STL'a

template< typename ArgTypeT, typename ArgIdT> requires
    non_narrowing_cast< CommandLineArgumentTemplateBase::InternalArgIdT, ArgIdT>
class CommandLineArgumentTemplate : public CommandLineArgumentTemplateBase
{
public:
    using index_type = ArgIdT;

    CommandLineArgumentTemplate( ArgTypeT type, ArgIdT id)
    :
        CommandLineArgumentTemplateBase( static_cast< InternalArgIdT>( id)),
        m_ArgType{ type}
    {};

protected:
    ArgTypeT m_ArgType;
};

template< typename ArgIdT> requires
    non_narrowing_cast< CommandLineArgumentTemplateBase::InternalArgIdT, ArgIdT>
class CommandLineArgumentDict : public CommandLineArgumentDictBase
{
public:
    using index_type = ArgIdT;

    ValueT *
    get_key_value( ArgIdT argId)
    {   return base_get_key_value( static_cast< InternalArgIdT>( argId)); };
};

template< class ArgTemplateT> requires
    std::derived_from< ArgTemplateT, CommandLineArgumentTemplateBase>
class CommandLineArgumentParser : public CommandLineArgumentParserBase
{
public:

    CommandLineArgumentParser( int argc, char * argv[])
    :
        CommandLineArgumentParserBase( argc, argv)
    {};

    template< std::ranges::view CliArgTemplateView> requires
        std::is_same_v< std::ranges::range_value_t< CliArgTemplateView>, ArgTemplateT>
    bool
    load_argument_templates( CliArgTemplateView const argTemplates)
    {
        clear_key_lookup_tables();

        bool allOk = true;

        for( ArgTemplateT const & argt : argTemplates)
            allOk &= load_argument_template( static_cast< CommandLineArgumentTemplateBase const &>( argt));

        return allOk;
    };

    template< typename ArgDictT> requires
        std::derived_from< ArgDictT, CommandLineArgumentDictBase> &&
        std::is_same_v< typename ArgDictT::index_type, typename ArgTemplateT::index_type>
    bool
    parse( ArgDictT & outDict)
    {   return base_parse( outDict); };
};

#endif//_CLIPARSER_H_
