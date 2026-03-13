
#ifndef _CLIPARSER_H_
# define _CLIPARSER_H_

#include <unordered_map>
# ifndef _EXTERNAL_LIBRARIES_H_
#  include "extlib.h"
# endif//_EXTERNAL_LIBRARIES_H_

# ifndef _STREAM_H_
#  include "stream.h"
# endif//_STREAM_H_


template< std::convertible_to< size_t> ArgId>
struct CommandLineArgumentTemplates
{
    ArgId			m_Id;
    char const *		m_ShortName;
    char const *		m_LongName;
};


template< std::convertible_to< size_t> ArgId>
class CommandLineArgumentDict
{
    friend class CommandLineArgumentParser;

public:
    using ArgValue = std::list< std::string>;

private:
    using Dict = std::unordered_map< size_t, ArgValue>;
    
public:
    bool 			is_set( ArgId id);
    ArgValue *			get_value( ArgId id);
    
private:
    void 			add_value( ArgId id, std::string_view value);

private:
    Dict			m_Data;
};

class CommandLineArgumentParser
{
private:
    static constexpr char const * 	ShortNameArgPrefix = "-";
    static constexpr char const * 	LongNameArgPRefix = "--";

public:
    CommandLineArgumentParser( int argc, char* argv[]);

private:
    int			    	m_ArgumentCount;
    char ** 			m_ArgumentValues;
};	


#endif//_CLIPARSER_H_
