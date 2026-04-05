
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

    11. be_collating_symbol                 :=  OPEN_DOT    COLL_ELEM_SINGLE    DOT_CLOSE
                                             |  OPEN_DOT    COLL_ELEM_MULTI     DOT_CLOSE
                                             |  OPEN_DOT    META_CHAR           DOT_CLOSE

    12. be_character_class                  :=  OPEN_EQUAL  COLL_ELEM_SINGLE    EQUAL_CLOSE
                                             |  OPEN_EQUAL  COLL_ELEM_MULTI     EQUAL_CLOSE

    13. be_equivalence_class                :=  OPEN_COLON  CLASS_NAME          COLON_CLOSE



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

# ifndef _STRING_STREAM_H_
#  include "string_stream.hpp"
# endif//_STRING_STREAM_H_

#include <array>
#include <string_view>

enum class ExtendedRegularExpressionToken
{
    Circumlfex,
    Count
};

class BracketExpressionLexer
{
    bool try_syntax();
};

class ExtendedRegularExpressionLexer
{
    static constexpr std::array< std::string_view, 128> m_TokenTexts;

    struct TokenAlts
    {
        std::size_t position;
        std::size_t count;
    };

    static constexpr std::array< TokenAlts, static_cast< std::size_t>( ExtendedRegularExpressionToken::Count)> m_TokenArray
    {
        TokenAlts{ 0, 1}
    };

    StringStream                m_Stream;

    bool                        try_syntax();

    bool                        try_syntax_branch();
    bool                        try_syntax_expression();
    bool                        try_syntax_single_or_collating_element();
    bool                        try_syntax_duplication_symbol();

    template< ExtendedRegularExpressionToken Token>
    int
    try_token()
    {
        constexpr auto const & token = m_TokenArray[ static_cast< std::size_t>( Token)];

        if constexpr( token.count == 1)
            return try_single_token< token.position>();
        else
            return try_sequence_token< token.position>( std::make_index_sequence< token.count>{});
    }

    template< std::size_t Begin, std::size_t... Offset>
    int
    try_sequence_token( std::index_sequence< Offset...>)
    {
        int matchedLength = -1;

        [[maybe_unused]] bool shortCircuit = (((matchedLength = try_single_token< Begin + Offset>()) != -1) || ...);

        return matchedLength;
    };

    template< std::size_t TokenPosition>
    int
    try_single_token()
    {
        constexpr std::string_view token = m_TokenTexts[ TokenPosition];

        if constexpr( token.length() == 1)
        {
            if( char found; m_Stream.peek_char( found))
                if( found == *token.data())
                    return 1;

            return -1;
        }
        else
        {
            if( std::string_view found; m_Stream.peek_string( found, token.length()))
                if( found == token)
                    return token.length();

            return -1;
        }
    }
};
