
#ifndef _CLIPARSER_H_
# define _CLIPARSER_H_

# ifndef _INTRO_H_
#  include "intro.h"
# endif//_INTRO_H_

# ifndef _STREAM_H_
#  include "stream.h"
# endif//_STREAM_H_

# include <list>
# include <string>
# include <string_view>

/**
    User provided keys MUST NOT include white characters.
    All conesecutive rules except consecutive terminal tokens MUST have at least one white token separating them.
    Consecutive terminal tokens are to be lexed as explicitly specified.
    Short key groups are resolved from left to right by the longest valid short key at point.
    Value sequence is bound to the earliest previous key.

    00. command_line            :=  (  long_key  |  short_key_group  )  value*  )*

    01. long_key                :=  <hyphen>  <hyphen>  [[  <!white>+  ]]

    02. short_key_group         :=  <hyphen>  [[ !(  <white>  |  <hyphen>  )  <!white>*  ]]

    03. value                   :=  unquoted_value  |  singly_quoted_value  |  doubly_quoted_value

    04. unquoted_value          :=  [[ !(  <white>  |  <hyphen>  )  !<white>*  ]]

    05. singly_quoted_value     :=  <single_quotes> [[  !<single_quotes>* ]]  <single_quotes>

    06. doubly_quoted_value     :=  <double_quotes> [[  !<double_quotes>* ]]  <double_quotes>
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
    CommandLineArgumentLexer( int argc, char * argv[]);

    bool lex( CliLexTokenList & outTokens);

private:
    void skip_white();

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

    std::string         m_CommandLine;
    StreamView          m_CommandLineStream;
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
