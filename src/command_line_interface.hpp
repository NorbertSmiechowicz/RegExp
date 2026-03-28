
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
public:
    CommandLineArgumentLexer( std::string_view commandLine);

    NODISCARD bool                              lex( CommandLineLexToken::List & outTokens);

private:
    using TokenList = CommandLineLexToken::List;

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

    StringStream                                m_CommandLineStream;
    TokenList *                                 m_LexedTokens;
};

////////////////////////////////////////////////
//  CommandLineArgumentTemplateBase

class CommandLineArgumentTemplateBase
{
    friend class CommandLineArgumentIdLookupTable;

public:
    CommandLineArgumentTemplateBase( CommandLineArgumentId id) : m_Id{ id} {};

protected:
    CommandLineArgumentId                       m_Id;

public:
    std::string_view                            m_ShortKey;
    std::string_view                            m_LongKey;
    std::string_view                            m_ShortHelp;
    std::string_view                            m_VerboseHelp;
};

////////////////////////////////////////////////
//  CommandLineArgumentDictBase

class CommandLineArgumentDictBase
{
    friend class CommandLineArgumentParserBase;

public:
    using value_type = std::list< std::string>;
    using const_iterator = value_type const *;

    const_iterator                              end() const;
    const_iterator                              get_process_value() const;

protected:
    using InnerDict = std::unordered_map< CommandLineArgumentId, value_type>;

    void                                        clear();
    const_iterator                              do_get_argument_value( CommandLineArgumentId argId) const;
    value_type &                                add_empty_key( CommandLineArgumentId argId);
    void                                        add_key_value( CommandLineArgumentId argId, std::string_view value);
    void                                        add_process_value( std::string_view value);

    InnerDict                                   m_Dict;
    value_type                                  m_ProcessValue;
};

////////////////////////////////////////////////
//  CommandLineArgumentIdLookupTable

class CommandLineArgumentIdLookupTable
{
public:
    using InnerDict = std::unordered_map< std::string_view, CommandLineArgumentId>;

    bool                                        lookup_short_key( CommandLineArgumentId & outArgId, std::string_view key) const;
    bool                                        lookup_long_key( CommandLineArgumentId & outArgId, std::string_view key) const;

    bool                                        load_argument_template( CommandLineArgumentTemplateBase const & argTemplate);
    bool                                        register_long_key( std::string_view key, CommandLineArgumentId argId);
    bool                                        register_short_key( std::string_view key, CommandLineArgumentId argId);

    InnerDict                                   m_LongKeyLookup;
    InnerDict                                   m_ShortKeyLookup;

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

    CommandLineArgumentParserBase( int argc, char const * argv[], CommandLineArgumentIdLookupTable && argLookupTable);

    NODISCARD bool                              lex();
    NODISCARD bool                              do_parse( CommandLineArgumentDictBase & outDict);

    NODISCARD bool                              try_parse_long_key( std::string_view key);
    NODISCARD bool                              try_parse_short_key( std::string_view key);
    NODISCARD bool                              try_parse_short_key_group( std::string_view keyGroup);
    NODISCARD bool                              try_parse_value( std::string_view value);

    int                                         m_ArgCount;
    char const **                               m_ArgValues;
    CommandLineArgumentIdLookupTable const      m_ArgIdLookupTable;
    CommandLineLexToken::List                   m_LexTokenList;
    std::optional< CommandLineArgumentId>       m_ActiveKeyArgId;
    CommandLineArgumentDictBase *               m_BuildDict;
};

////////////////////////////////////////////////
//  CommandLineArgumentTemplate

template< typename ArgTypeT, typename ArgIdT> requires
    non_narrowing_cast< CommandLineArgumentId, ArgIdT>
class CommandLineArgumentTemplate : public CommandLineArgumentTemplateBase
{
public:
    using key_type = ArgIdT;

    CommandLineArgumentTemplate( ArgIdT id)
    :
        CommandLineArgumentTemplateBase( static_cast< CommandLineArgumentId>( id))
    {
    };

    ArgTypeT                                    m_ArgType;
};

////////////////////////////////////////////////
//  CommandLineArgumentDict

template< typename ArgIdT> requires
    non_narrowing_cast< CommandLineArgumentId, ArgIdT>
class CommandLineArgumentDict : public CommandLineArgumentDictBase
{
public:
    using key_type = ArgIdT;

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
    template< std::ranges::view CliArgTemplateView> requires
        std::is_same_v< std::ranges::range_value_t< CliArgTemplateView>, ArgTemplateT>
    CommandLineArgumentParser( int argc, char const * argv[], CliArgTemplateView argTemplates)
    :
        CommandLineArgumentParserBase(
            argc,
            argv,
            [argTemplates](){
                bool allOk = true;
                CommandLineArgumentIdLookupTable lut{};

                for( ArgTemplateT const & argt : argTemplates)
                    allOk &= lut.load_argument_template( static_cast< CommandLineArgumentTemplateBase const &>( argt));

                if( ! allOk)
                {
                    std::printf( "Command line argument definitions are ill formed. Terminating the process...\n");
                    std::terminate();
                }

                return lut;
            }()
         )
    {
    };

    template< typename ArgDictT> requires
        std::derived_from< ArgDictT, CommandLineArgumentDictBase> &&
        std::is_same_v< typename ArgDictT::key_type, typename ArgTemplateT::key_type>
    bool
    parse( ArgDictT & outDict)
    {
        return do_parse( outDict);
    };
};

#endif//_CLIPARSER_H_
