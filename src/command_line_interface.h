
#ifndef _CLIPARSER_H_
# define _CLIPARSER_H_

#include <memory>
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

    00. command_line            :=  (  (  short_argument  |  long_argument  )  (  <white>+  value_sequence  )?  )*

    01. short_argument          :=  <short_key_prefix>  [[  <short_key>+  ]]

    02. long_argument           :=  <long_key_prefix>  [[  <long_key>  ]]

    03. value_sequence          :=  (  value  (  <white>+  value  )*  )?

    04. value                   :=  unquoted_value  |  singly_quoted_value  |  doubly_quoted_value

    05. unquoted_value          :=  [[ <!white>*  ]]

    06. singly_quoted_value     :=  <single_quotes> [[  <!single_quotes>* ]]  <single_quotes>

    07. doubly_quoted_value     :=  <double_quotes> [[  <!double_quotes>* ]]  <double_quotes>
*/

////////////////////////////////////////////////

struct LexedToken
{
    unsigned long   LexId;
    std::string     Text;
};

class CommandLineArgumentLexer
{
public:
    CommandLineArgumentLexer( int argc, char* argv[]);

    void skip_white();
    long peek_terminal( unsigned long terminalTokenId);
    bool try_terminal( unsigned long terminalTokenId);

    bool try_syntax_command_line();
    bool try_syntax_flag_group();
    bool try_syntax_short_argument();
    bool try_syntax_long_argument();
    bool try_syntax_value_sequence();
    bool try_syntax_value();
    bool try_syntax_unquoted_value();
    bool try_syntax_singly_quoted_value();
    bool try_syntax_doubly_quoted_value();

private:
    using TokenStack = std::list< std::unique_ptr< LexedToken>>;

    StreamView      m_CommandLineStream;
    TokenStack      m_TokenStack;
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

template< typename TypeT, typename IdT> requires
    non_narrowing_cast< TypeT, long> &&
    non_narrowing_cast< IdT, long>
struct CommandLineArgumentTemplate : CommandLineArgumentTemplateBase
{
};

#endif//_CLIPARSER_H_
