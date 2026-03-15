
#ifndef _CLIPARSER_H_
# define _CLIPARSER_H_

# ifndef _EXTERNAL_LIBRARIES_H_
#  include "extlib.h"
# endif//_EXTERNAL_LIBRARIES_H_

# ifndef _STREAM_H_
#  include "stream.h"
# endif//_STREAM_H_


/**
    1.   command_line           :=  ( flag_group | short_argument | long_argument )*

    2.  flag_group              :=  <short_key_prefix> ( <short_key> )+

    3.  short_argument          :=  <short_key_prefix> <short_key> <value_prefix> value_sequence

    4.  long_argument           :=  <long_key_prefix> <long_key> <value_prefix> value_sequence

    5.  value_sequence          :=  ( value ( <value_separator> value )* )?

    6.  value                   :=  unquoted_value | singly_quoted_value | doubly_quoted_value

    7.  unquoted_value          :=  (! <white> )*

    8.  singly_quoted_value     :=  <single_quotes> (! <single_quotes> )* <single_quotes>

    9.  doubly_quoted_value     :=  <double_quotes> (! <double_quotes> )* <double_quotes>
*/


using ArgId = size_t;


struct CommandLineArgumentTemplate
{
    ArgId               m_Id;
    char const *        m_ShortKey;
    char const *        m_LongKey;
};

class CommandLineArgumentDict
{
    friend class CommandLineArgumentParser;

 public:
    using Value = std::vector< std::string_view>;

 protected:
    using Dict = std::unordered_map< ArgId, Value>;

 public:
    Value *     find_value( ArgId id);

 protected:
    void        add_value( ArgId id, std::string_view value);

 protected:
    Dict        m_Data;
};

class CommandLineArgumentParser
{
 private:
    static constexpr std::string    ShortKeyPrefix{ "-"};
    static constexpr std::string    LongKeyPrefix{ "--"};
    static constexpr std::string    ValuePrefix{ ""};
    static constexpr std::string    ValueSeparator{ ""};

 public:
    CommandLineArgumentParser( int argc, char* argv[], std::vector< CommandLineArgumentTemplate const> & cliArgTemplateListy);

 private:
    void skip_white();
    bool try_syntax_command_line();
    bool try_syntax_flag_group();
    bool try_syntax_short_argument();
    bool try_syntax_long_argument();
    bool try_syntax_value_sequence();
    bool try_syntax_value();
    bool try_syntax_unquoted_value();
    bool try_syntax_singly_quoted_value();
    bool try_syntax_doubly_quoted_value();
    bool try_syntax_short_key_prefix();
    bool try_syntax_long_key_prefix();
    bool try_syntax_value_prefix();
    bool try_syntax_value_separator();

 private:
    int                                                         m_ArgumentCount;
    char **                                                     m_ArgumentValues;
    std::unique_ptr< CommandLineArgumentDict>                   m_OutArgumentDict;
    std::vector< CommandLineArgumentTemplate const> const &     m_ArgumentTemplates;
    std::unordered_map< std::string_view, ArgId>                m_ShortNameLookUpTable;
    std::unordered_map< std::string_view, ArgId>                m_LongNameLookUpTable;
};


CommandLineArgumentParser::CommandLineArgumentParser( int argc, char* argv[], std::vector< CommandLineArgumentTemplate const> & cliArgTemplateList)
:
    m_ArgumentCount{ argc},
    m_ArgumentValues{ argv},
    m_ArgumentTemplates{ cliArgTemplateList}
{
};

#endif//_CLIPARSER_H_
