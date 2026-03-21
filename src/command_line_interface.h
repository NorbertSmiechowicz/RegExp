
#ifndef _CLIPARSER_H_
# define _CLIPARSER_H_

# ifndef _CONCEPTS_H_
#  include "concepts.h"
# endif//_CONCEPTS_H_

# ifndef _STRING_TYPES_H_
#  include "string_stream.h"
# endif//_STRING_TYPES_H_

# include <list>
# include <string>
# include <string_view>

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
    bool lex( CliLexTokenList & outTokens, char const * commandLine, unsigned long lineLength);

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
    std::string_view    ShortName;
    std::string_view    LongName;
    std::string_view    ShortHelp;
    std::string_view    VerboseHelp;
};


class CommandLineArgumentParser
{
public:
    CommandLineArgumentParser( int argc, char * argv[]);

    bool parse( /* argument dict */);

private:
    bool parse_short_key_group( std::string_view keyGroup);
    bool find_short_key( unsigned long & outArgId, std::string_view key);
    bool find_long_key( unsigned long & outArgId, std::string_view key);

    int                 m_Argc;
    char **             m_Argv;
    CliLexTokenList     m_LexTokenList;
    unsigned long       m_LastKey;
};


template< typename TypeT, typename IdT> requires
    non_narrowing_cast< TypeT, long> &&
    non_narrowing_cast< IdT, long>
struct CommandLineArgumentTemplate : CommandLineArgumentTemplateBase
{
};

#endif//_CLIPARSER_H_
