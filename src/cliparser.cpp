
#include "cliparser.h"

CommandLineArgumentParser::CommandLineArgumentParser( int argc, char* argv[]) :
    m_ArgumentCount{ argc},
    m_ArgumentValues{ argv}
{
};
