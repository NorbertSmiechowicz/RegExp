
#ifndef _CLIPARSER_H_
# define _CLIPARSER_H_

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

    ////////////////////////////////////////////////

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

# ifndef _CONCEPTS_H_
#  include "concepts.hpp"
# endif//_CONCEPTS_H_

# ifndef _STRING_TYPES_H_
#  include "string_stream.hpp"
# endif//_STRING_TYPES_H_

# include <cstdio>
# include <list>
# include <string>
# include <string_view>
# include <ranges>
# include <optional>
# include <unordered_map>

////////////////////////////////////////////////

using CommandLineArgumentId = std::size_t;

////////////////////////////////////////////////
//  CommandLineArgumentTemplateBase

class CommandLineArgumentTemplateBase
{
    friend class CommandLineArgumentIdLookupTable;

protected:
    CommandLineArgumentId                       m_Id;

public:
    std::string_view                            m_ShortKey;
    std::string_view                            m_LongKey;
    std::string_view                            m_ShortHelp;
    std::string_view                            m_VerboseHelp;

public:
    constexpr
    CommandLineArgumentTemplateBase( CommandLineArgumentId id, std::string_view skey, std::string_view lkey, std::string_view shelp, std::string_view vhelp)
    :
        m_Id{ id},
        m_ShortKey{ skey},
        m_LongKey{ lkey},
        m_ShortHelp{ shelp},
        m_VerboseHelp{ vhelp}
    {
    }
};

////////////////////////////////////////////////
//  CommandLineArgumentDictBase

class CommandLineArgumentDictBase
{
    friend class CommandLineArgumentParserBase;

public:
    using value_type = std::list< std::string>;
    using const_iterator = value_type const *;

protected:
    using InnerDict = std::unordered_map< CommandLineArgumentId, value_type>;

protected:
    InnerDict                                   m_Dict;
    value_type                                  m_ProcessValue;

public:
    const_iterator                              end() const;
    const_iterator                              get_process_value() const;

protected:
    void                                        clear();
    const_iterator                              do_get_argument_value( CommandLineArgumentId argId) const;
    value_type &                                add_empty_key( CommandLineArgumentId argId);
    void                                        add_key_value( CommandLineArgumentId argId, std::string_view value);
    void                                        add_process_value( std::string_view value);
};

////////////////////////////////////////////////
//  CommandLineArgumentIdLookupTable

class CommandLineArgumentIdLookupTable
{
public:
    using InnerDict = std::unordered_map< std::string_view, CommandLineArgumentId>;

public:
    InnerDict                                   m_LongKeyLookup;
    InnerDict                                   m_ShortKeyLookup;

public:
    bool                                        is_empty() const;
    bool                                        lookup_long_key( CommandLineArgumentId & outArgId, std::string_view key) const;
    bool                                        lookup_short_key( CommandLineArgumentId & outArgId, std::string_view key) const;

    void                                        clear();
    bool                                        load_argument_template( CommandLineArgumentTemplateBase const & argTemplate);
    bool                                        register_long_key( std::string_view key, CommandLineArgumentId argId);
    bool                                        register_short_key( std::string_view key, CommandLineArgumentId argId);
};

////////////////////////////////////////////////

enum class CommandLineLexTerminalId : std::size_t;
enum class CommandLineLexTokenId : std::size_t;

////////////////////////////////////////////////
//  CommandLineLexToken

struct CommandLineLexToken
{
    using List = std::list< CommandLineLexToken>;

    CommandLineLexTokenId                       m_Id;
    std::string                                 m_Text;
};

////////////////////////////////////////////////
//  CommandLineArgumentLexer

class CommandLineArgumentLexer
{
private:
    StringStream                                m_CommandLineStream;
    CommandLineLexToken::List *                 m_LexedTokens;

public:
    CommandLineArgumentLexer( std::string_view commandLine);

    NODISCARD bool                              lex( CommandLineLexToken::List & outTokens);

private:
    bool                                        skip_white();

    NODISCARD bool                              peek_terminal( CommandLineLexTerminalId terminalTokenId);
    NODISCARD bool                              try_terminal( CommandLineLexTerminalId terminalTokenId);

    NODISCARD bool                              try_syntax_command_line();
    NODISCARD bool                              try_syntax_flag_group();
    NODISCARD bool                              try_syntax_short_key();
    NODISCARD bool                              try_syntax_long_key();
    NODISCARD bool                              try_syntax_value();
    NODISCARD bool                              try_syntax_unquoted_value();
    NODISCARD bool                              try_syntax_singly_quoted_value();
    NODISCARD bool                              try_syntax_doubly_quoted_value();
};

////////////////////////////////////////////////
//  CommandLineArgumentParserBase

class CommandLineArgumentParserBase
{
protected:
    using ArgIdLUT = CommandLineArgumentIdLookupTable;
    using LexTokenList = CommandLineLexToken::List;
    using OptionalArgId = std::optional< CommandLineArgumentId>;
    using ArgDict = CommandLineArgumentDictBase;

protected:
    int                                         m_ArgCount;
    char const **                               m_ArgValues;
    CommandLineArgumentIdLookupTable            m_ArgIdLookupTable;
    CommandLineLexToken::List                   m_LexTokenList;
    std::optional< CommandLineArgumentId>       m_ActiveKeyArgId;
    CommandLineArgumentDictBase *               m_BuildDict;

protected:
    CommandLineArgumentParserBase( int argc, char const * argv[]);

    NODISCARD bool                              lex();
    NODISCARD bool                              do_parse( CommandLineArgumentDictBase & outDict);

    NODISCARD bool                              try_parse_long_key( std::string_view key);
    NODISCARD bool                              try_parse_short_key( std::string_view key);
    NODISCARD bool                              try_parse_short_key_group( std::string_view keyGroup);
    NODISCARD bool                              try_parse_value( std::string_view value);
};

////////////////////////////////////////////////
//  CommandLineArgumentTemplate

template< typename ArgTypeT, typename ArgIdT> requires
    non_narrowing_cast< CommandLineArgumentId, ArgIdT>
class CommandLineArgumentTemplate : public CommandLineArgumentTemplateBase
{
public:
    using key_type = ArgIdT;

public:
    ArgTypeT                                    m_ArgType;

public:
    template< typename ...ConstructorArgs>
    constexpr
    CommandLineArgumentTemplate( ArgTypeT type, ArgIdT id, ConstructorArgs... other)
    :
        CommandLineArgumentTemplateBase( static_cast< CommandLineArgumentId>( id), other...),
        m_ArgType{ type}
    {
    }

    ArgIdT
    get_id() const
    {
        return static_cast< ArgIdT>( m_Id);
    }
};

////////////////////////////////////////////////
//  CommandLineArgumentDict

template< typename ArgIdT> requires
    non_narrowing_cast< CommandLineArgumentId, ArgIdT>
class CommandLineArgumentDict : public CommandLineArgumentDictBase
{
public:
    using key_type = ArgIdT;

public:
    const_iterator
    get_argument_value( ArgIdT argId) const
    {
        return do_get_argument_value( static_cast< CommandLineArgumentId>( argId));
    };
};

////////////////////////////////////////////////
//  CommandLineArgumentParser

template< class ArgTemplateT> requires
    std::derived_from< ArgTemplateT, CommandLineArgumentTemplateBase>
class CommandLineArgumentParser : public CommandLineArgumentParserBase
{
public:
    using Dict = CommandLineArgumentDict< typename ArgTemplateT::key_type>;

public:
    template< std::ranges::range CliArgTemplateRange> requires
        std::is_same_v< std::ranges::range_value_t< CliArgTemplateRange>, ArgTemplateT>
    CommandLineArgumentParser( int argc, char const * argv[], CliArgTemplateRange & argTemplates)
    :
        CommandLineArgumentParserBase( argc, argv)
    {
        bool allOk = true;

        for( ArgTemplateT const & argt : argTemplates)
            allOk &= m_ArgIdLookupTable.load_argument_template( static_cast< CommandLineArgumentTemplateBase const &>( argt));

        if( ! allOk)
            m_ArgIdLookupTable.clear();
    };

    std::optional< Dict>
    parse()
    {
        std::optional< Dict> outDict = Dict{};

        if( ! do_parse( *outDict))
            outDict.reset();

        return outDict;
    };
};

#endif//_CLIPARSER_H_
