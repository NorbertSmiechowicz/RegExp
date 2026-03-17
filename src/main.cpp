
#include "intro.h"
#include "command_line_interface.h"
#include "forward_string_index.h"

#include <memory>
#include <vector>

////////////////////////////////////////////////




enum class CliArgType
{
    Flag,
    Sequence
};

enum class CliArgId
{
};

using CliArgTemplate = CommandLineArgumentTemplate< CliArgType, CliArgId>;


int main( int /*argc*/, char* /*argv*/[])
{
    ////////////////////////////////////////////////

    ForwardStringIndexFactory< char> testFactory;

    std::vector< char const *> testOpt{ "a", "aa", "ab", "baaa", "baab", "baa", "bb", "bb", "cccBcOcc", "cccBccccc", "cccDccc"};

    for( char const * opt : testOpt)
    {
        if( ! testFactory.add( opt, CC( char *, opt)))
            printf( "fail: %s\n", opt);
    }

    std::unique_ptr< ForwardStringIndex< char>> testIdx{ testFactory.emit()};

    printf( "%s\n", testIdx->find( "cccBcOcc"));
    printf( "%s\n", testIdx->find( "cccBccccc"));
    printf( "%s\n", testIdx->find( "cccDccc"));

    return 0;
}
