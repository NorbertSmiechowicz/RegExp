
#include "fwdstridx.h"
#include <memory>

#define RC( T, X) reinterpret_cast< T>( X)
#define SC( T, X) static_cast< T>( X)

enum class ArgIds : long
{
    AB = 1,
    AC,
    AD,
    C
};


int main( int /*argc*/, char* /*argv*/[])
{
    ForwardStringIndexFactory idxFactory;

#define IDX_ADD( factory, name) factory.add( "" #name "", RC( void *, ArgIds::name))

    IDX_ADD( idxFactory, AB);
    IDX_ADD( idxFactory, AB);
    IDX_ADD( idxFactory, AC);
    IDX_ADD( idxFactory, AD);
    IDX_ADD( idxFactory, C);

#undef IDX_ADD

    std::unique_ptr< ForwardStringIndex> fsi{ idxFactory.emit()};


    printf( "%ld\n", RC( long, fsi->find( "D")));
    printf( "%ld\n", RC( long, fsi->find( "AB")));
    printf( "%ld\n", RC( long, fsi->find( "AC")));


    return 0;
}
