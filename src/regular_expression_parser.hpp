
/*
    escape_character: <backslash> that is neither in a bracket expression or itself escaped

    1. Matching is based on bit pattern used for encoding the character -> streaming pure bytes.
    2. Search starts at the beggining of a string.
    3. Match is the longest of the leftmost matches.
    4. Null string is longer that no match at all.
    5. Length of the match is measured in bytes.
    6. Invalid REs generate an error condition.
    7. '\0' is not permitted in REs or string to be matched, and are treated as the end of the string.
    8. Must support at least 256 byte REs.

Mamy trzy konteksty ?:

    1. Bracket Expression
    2. Basic Regular Expression
    3. Extended Regular Expression

-------------------

##2. Bracket Expression

    01. bracket_expression                  :=  <open_bracket>  be_matching_list     <close_bracket>
                                             |  <open_bracket>  be_nonmatching_list  <close_bracket>

    02. be_matching_list                    :=                  be_bracket_list

    03. be_nonmatching_list                 :=  <circumflex>    be_bracket_list

    04. be_bracket_list                     :=  be_follow_list
                                             |  be_follow_list  <hyphen>

    05. be_follow_list                      :=                  be_expression_term
                                             |  be_follow_list  be_expression_term

    06. be_expression_term                  :=  be_single_expression
                                             |  be_range_expression

    07. be_single_expression                :=  be_end_range
                                             |  be_character_class
                                             |  be_equivalence_class

    08. be_range_expression                 :=  be_start_range  be_end_range
                                             |  be_start_range  <hyphen>

    09. be_start_range                      :=  be_end_range    <hyphen>

    10. be_end_range                        :=  COLL_ELEM_SINGLE
                                             |  collating_symbol

    11. be_collating_symbol                 :=  <open_dot>    COLL_ELEM_SINGLE    <dot_close>
                                             |  <open_dot>    COLL_ELEM_MULTI     <dot_close>
                                             |  <open_dot>    META_CHAR           <dot_close>

    12. be_character_class                  :=  <open_equal>  COLL_ELEM_SINGLE    <equal_close>
                                             |  <open_equal>  COLL_ELEM_MULTI     <equal_close>

    13. be_equivalence_class                :=  <open_colon>  CLASS_NAME          <colon_close>



##3. Extended Regular Expressions

    %% tokens:  ERE_ORDINARY_CHARACTER, ERE_QUOTED_CHARACTER, ERE_DUPLICATION_COUNT

    01. extended_regular_expression         :=                                       ere_branch
                                             |  extended_regular_expression  <pipe>  ere_branch

    02. ere_branch                          :=              ere_expression
                                             |  ere_branch  ere_expression

    03. ere_expression                      :=  ere_single_or_collating_elem
                                             |  <circumflex>
                                             |  <dollar>
                                             |  <open_parenthesis>  extended_regular_expression  <close_parenthesis>
                                             |  ere_expression  ere_duplication_symbol

    04. ere_single_or_callating_element     :=  ERE_ORDINARY_CHARACTER
                                             |  ERE_QUOTED_CHARACTER
                                             |  <period>
                                             |  bracket_expression

    05. ere_duplication_symbol              :=  <asterisk>
                                             |  <plus>
                                             |  <question_mark>
                                             |  <open_brace>  ERE_DUPLICATION_COUNT                                   <close_brace>
                                             |  <open_brace>  ERE_DUPLICATION_COUNT  <period>                         <close_brace>
                                             |  <open_brace>  ERE_DUPLICATION_COUNT  <period>  ERE_DUPLICATION_COUNT  <close_brace>

*/

#ifndef _REGULAR_EXPRESSION_PARSER_H_
# define _REGULAR_EXPRESSION_PARSER_H_

# ifndef _STRING_STREAM_H_
#  include "string_stream.hpp"
# endif//_STRING_STREAM_H_

# include <array>
# include <string_view>

////////////////////////////////////////////////

#define REGEXP_TOKEN_LIST\
    REGEXP_TOKEN_ELEM( Backslash, "\\") /* Haha ha... */\
    REGEXP_TOKEN_ELEM( Period, ".")\
    REGEXP_TOKEN_ELEM( Asterisk, "*")\
    REGEXP_TOKEN_ELEM( QuestionMark, "?")\
    REGEXP_TOKEN_ELEM( Plus, "+")\
    REGEXP_TOKEN_ELEM( Pipe, "|")\
    REGEXP_TOKEN_ELEM( Hyphen, "-")\
    REGEXP_TOKEN_ELEM( Circumflex, "^")\
    REGEXP_TOKEN_ELEM( Dollar, "$")\
    REGEXP_TOKEN_ELEM( OpenParenthesis, "(")\
    REGEXP_TOKEN_ELEM( CloseParenthesis, ")")\
    REGEXP_TOKEN_ELEM( OpenBrace, "{")\
    REGEXP_TOKEN_ELEM( CloseBrace, "}")\
    REGEXP_TOKEN_ELEM( OpenBracket, "[")\
    REGEXP_TOKEN_ELEM( CloseBracket, "]")\
    REGEXP_TOKEN_ELEM( OpenDot, "[.")\
    REGEXP_TOKEN_ELEM( DotClose, ".]")\
    REGEXP_TOKEN_ELEM( OpenEqual, "[=")\
    REGEXP_TOKEN_ELEM( EqualClose, "=]")\
    REGEXP_TOKEN_ELEM( OpenColon, "[:")\
    REGEXP_TOKEN_ELEM( ColonClose, ":]")\

#define REGEXP_TOKEN_ELEM( ID, TEXT) ID,

enum class RegularExpressionToken : std::size_t
{
    REGEXP_TOKEN_LIST
    Count
};

#undef REGEXP_TOKEN_ELEM
#define REGEXP_TOKEN_ELEM( ID, TEXT) TEXT,

static constexpr std::array< std::string_view, static_cast< std::size_t>( RegularExpressionToken::Count)> m_TokenArray
{
    REGEXP_TOKEN_LIST
};

#undef REGEXP_TOKEN_ELEM
#undef REGEXP_TOKEN_LIST

////////////////////////////////////////////////

struct RegularExpressionSyntaxNode
{
};

class RegularExpressionSyntaxTree
{
};

class RegularExpressionLexer
{
    StringStream & m_Stream;

public:
    RegularExpressionLexer( StringStream & stream)
    :
        m_Stream{ stream}
    {
    }

    template< RegularExpressionToken tokenId>
    bool
    try_token()
    {
        if constexpr( tokenId < RegularExpressionToken::Count)
        {
            std::string_view token = m_TokenArray[ static_cast< std::size_t>( tokenId)];

            if( std::string_view found; m_Stream.peek_string( found, token.length()))
                if( found == token)
                    return m_Stream.skip_char_count( token.length());

        }

        return false;
    }
};

class BracketExpressionLexer : public RegularExpressionLexer
{
    bool                        try_syntax();

    bool                        try_syntax_matching_list();
    bool                        try_syntax_nonmatching_list();
    bool                        try_syntax_bracket_list();
    bool                        try_syntax_follow_list();
    bool                        try_syntax_expression_term();
    bool                        try_syntax_single_expression();
    bool                        try_syntax_range_expression();
    bool                        try_syntax_start_range();
    bool                        try_syntax_end_range();
    bool                        try_syntax_collating_symbol();
    bool                        try_syntax_character_class();
    bool                        try_syntax_equivalence_class();
};

class ExtendedRegularExpressionLexer : public RegularExpressionLexer
{
    bool                        try_syntax();

    bool                        try_syntax_branch();
    bool                        try_syntax_expression();
    bool                        try_syntax_single_or_collating_element();
    bool                        try_syntax_duplication_symbol();

    bool                        try_syntax_ordinary_character();
    bool                        try_syntax_quoted_character();
    bool                        try_syntax_duplication_count();
};

#endif//_REGULAR_EXPRESSION_PARSER_H_
