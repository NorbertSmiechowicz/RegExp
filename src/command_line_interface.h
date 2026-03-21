
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

class CommandLineArgumentLexer
{
public:
    bool lex( CliLexTokenList & outTokens, std::string_view commandLine);

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

struct CommandLineArgumentTemplateBase
{
    unsigned long       Type;
    unsigned long       Id;
    std::string_view    ShortKey;
    std::string_view    LongKey;
    std::string_view    ShortHelp;
    std::string_view    VerboseHelp;
};

class CommandLineArgumentDictBase
{
    friend class CommandLineArgumentParserBase;

public:
    void * get_value( unsigned long argId);

private:
    void add_value( unsigned long argId, std::string_view value);

    using ValueT = std::list< std::string_view>;
    using ContainerT = std::unordered_map< unsigned long, ValueT>;

    ContainerT m_Data;
};

class CommandLineArgumentParserBase
{
    using KeyMap = std::unordered_map< std::string_view, unsigned long>;

public:
    CommandLineArgumentParserBase( int argc, char * argv[]);

    template< std::ranges::view CliArgTemplateRange> requires
        std::derived_from< std::ranges::range_value_t< CliArgTemplateRange>, CommandLineArgumentTemplateBase>
    bool load_argument_templates( CliArgTemplateRange && argTemplates)
    {
        clear_key_lookup_tables();

        bool allOk = true;

        for( CommandLineArgumentTemplateBase & argt : argTemplates)
            allOk &= load_argument_template( argt);

        return allOk;
    };

    bool parse( /* argument dict */);

private:
    bool load_argument_template( CommandLineArgumentTemplateBase & argTemplate);
    void clear_key_lookup_tables();

    bool parse_short_key_group( std::string_view keyGroup);
    bool find_short_key( unsigned long & outArgId, std::string_view key);
    bool find_long_key( unsigned long & outArgId, std::string_view key);

    int                 m_Argc;
    char **             m_Argv;
    CliLexTokenList     m_LexTokenList;
    unsigned long       m_LastKey;
    KeyMap              m_ShortKeyLookup;
    KeyMap              m_LongKeyLookup;
};


template< typename TypeT, typename IdT> requires
    non_narrowing_cast< TypeT, long> &&
    non_narrowing_cast< IdT, long>
struct CommandLineArgumentTemplate : CommandLineArgumentTemplateBase
{
    using index_type = IdT;
};

template< typename T>
class CommandLineArgumentParser
{
};

#endif//_CLIPARSER_H_
