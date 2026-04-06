
#include "command_line_interface.hpp"
#include "intrusive_list.hpp"

#include <cstdio>
#include <string>
#include <iostream>
#include <array>
#include <numeric>
#include <ranges>

////////////////////////////////////////////////

enum class CliArgType
{
    Flag,
    Sequence
};

#define CLI_ARG_LIST_DEF\
    CLI_ARG_LIST_ITEM( CliArgType::Flag,         Help,       "h",    "help",     "",     "")\
    CLI_ARG_LIST_ITEM( CliArgType::Sequence,     File,       "f",    "file",     "",     "")

#define CLI_ARG_LIST_ITEM( TYPE, ID, ...) ID,

enum class CliArgId
{
    CLI_ARG_LIST_DEF
    CliArgIdCount
};

#undef CLI_ARG_LIST_ITEM

using CliArgTemplate = CommandLineArgumentTemplate< CliArgType, CliArgId>;
using CliArgParser = CommandLineArgumentParser< CliArgTemplate>;

#define CLI_ARG_LIST_ITEM( TYPE, ID, ...) CliArgTemplate{ TYPE, CliArgId::ID, __VA_ARGS__},

static constexpr std::array< CliArgTemplate, static_cast< std::size_t>( CliArgId::CliArgIdCount)> argTemplates
{
    CLI_ARG_LIST_DEF
};

#undef CLI_ARG_LIST_ITEM
#undef CLI_ARG_LIST_DEF

////////////////////////////////////////////////


class TestListItem
{
private:
    ListLink< TestListItem>  m_Link;

public:
    using List = IntrusiveList< TestListItem, &TestListItem::m_Link, true>;

public:
    constexpr TestListItem( std::size_t val) : m_Size{ val}, m_Data{ nullptr} {}

    std::size_t     m_Size;
    void *          m_Data;
};


consteval std::size_t
consteval_correctness_test()
{
    TestListItem::List testList{};

    testList.emplace_front( 1U);
    testList.emplace_back( 2U);
    testList.emplace_back( 3U);
    testList.emplace_back( 4U);
    testList.emplace_front( 5U);

    TestListItem::List trashList{};

    auto itemsToUnlink = testList
        | std::views::filter([]( TestListItem & item)
            {
                return item.m_Size % 2 == 0;
            });

    for( TestListItem & item : itemsToUnlink)
    {
        testList.unlink( item);
        trashList.link_back( item);
    }

    return std::accumulate( testList.begin(), testList.end(), 0U,
        []( TestListItem const & left, TestListItem const & right)
        {
            return left.m_Size + right.m_Size;
        });
};


int main( int argc, char const * argv[])
{
    CliArgParser cliArgParser{ argTemplates};
    std::optional< CliArgParser::Dict> argDict{ cliArgParser.parse( argc, argv)};

    if( ! argDict.has_value())
        return 1;

    // ahh te dedykowane obsługi znaków w konsolach...
    // run -hf testa testb testc --testd teste --file \'\|testf\ \ \ \|\'

    if( argDict->get_argument_value( CliArgId::Help))
        printf( "Help was invoked.\n");

    if( CliArgParser::Dict::const_iterator fileValue = argDict->get_argument_value( CliArgId::File); fileValue != argDict->end())
        for( std::string const & fileText : *fileValue)
            std::cout<< fileText << ' ';

    std::cout << '\n';


    static_assert( std::sentinel_for< TestListItem::List::iterator, TestListItem::List::iterator>);

    std::size_t test{ consteval_correctness_test()};
    assert( test == 9U);

    return 0;
}
