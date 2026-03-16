
#include "forward_string_index.h"
#include "indexed_vector.h"

#include <cstddef>
#include <cstring>
#include <string_view>
#include <utility>


////////////////////////////////////////////////

static constexpr std::string_view const EmptyKey = "";

////////////////////////////////////////////////
// ForwardStringIndexPath

class ForwardStringIndexPath
{
public:
    using iterator = std::string_view::iterator;

    constexpr ForwardStringIndexPath();
    constexpr ForwardStringIndexPath( std::string_view key);

    constexpr std::size_t       size() const;
    constexpr bool              is_empty() const;
    constexpr bool              is_exhausted() const;

    constexpr bool              next_segment( char & pathSegment);

    constexpr iterator          begin();
    constexpr iterator          end();

    std::string_view            m_Key;
    std::size_t                 m_Depth;
};

constexpr
ForwardStringIndexPath::ForwardStringIndexPath()
:
    ForwardStringIndexPath( EmptyKey)
{
};

constexpr
ForwardStringIndexPath::ForwardStringIndexPath( std::string_view key)
:
    m_Key{ key}, m_Depth{ 0}
{
};

constexpr std::size_t
ForwardStringIndexPath::size() const
{
    return m_Key.size();
};

constexpr bool
ForwardStringIndexPath::is_empty() const
{
    return m_Key == EmptyKey;
};

constexpr bool
ForwardStringIndexPath::is_exhausted() const
{
    return m_Depth >= size();
};

constexpr bool
ForwardStringIndexPath::next_segment( char & pathSegment)
{
    if( is_exhausted())
        return false;

    pathSegment = m_Key[ m_Depth ++];
    return true;
};

constexpr ForwardStringIndexPath::iterator
ForwardStringIndexPath::begin()
{
    return m_Key.begin();
};

constexpr ForwardStringIndexPath::iterator
ForwardStringIndexPath::end()
{
    return m_Key.end();
};

////////////////////////////////////////////////
// ForwardStringIndexNode

class ForwardStringIndexNode
{
public:
    using IdxVector = IndexedVector< char, ForwardStringIndexNode>;

    ForwardStringIndexNode();
    ForwardStringIndexNode( std::string_view key, void * value);

    bool                        is_terminal_node();
    ForwardStringIndexNode *    find_terminal_node( ForwardStringIndexPath key);

    bool                        insert( ForwardStringIndexPath key, void * value);
    bool                        on_key_conflict( void * value);

    bool                        prune();

    std::string_view            m_Key;
    void *                      m_Value;
    IdxVector                   m_ChildNodes;
};

ForwardStringIndexNode::ForwardStringIndexNode()
:
    m_Key{ EmptyKey},
    m_Value{ nullptr}
{
};

ForwardStringIndexNode::ForwardStringIndexNode( std::string_view key, void * value)
:
    m_Key{ key},
    m_Value{ value}
{
};

bool
ForwardStringIndexNode::is_terminal_node()
{
    return m_Key != EmptyKey;
};

ForwardStringIndexNode *
ForwardStringIndexNode::find_terminal_node( ForwardStringIndexPath path)
{
    if( path.is_empty())
        return nullptr;

    std::size_t matchedSegments = 0;
    ForwardStringIndexNode * node = this;

    for( char const segment : path)
    {
        IdxVector::iterator childIter = node->m_ChildNodes.get( segment);

        if( childIter == node->m_ChildNodes.end())
            break;

        node = &*childIter;
        matchedSegments ++;
    }

    if( path.size() == matchedSegments)
        if( node->m_Key.size() == matchedSegments)
            return node;

    if( node->m_Key.size() > matchedSegments)
        if( &node->m_Key[ matchedSegments] == &path.m_Key[ matchedSegments])
            return node;

    return nullptr;
};

bool
ForwardStringIndexNode::insert( ForwardStringIndexPath path, void * value)
{
    char pathSegment = '\0';
    ForwardStringIndexNode * node = this;

    while( path.next_segment( pathSegment))
    {
        IdxVector::iterator childIter = node->m_ChildNodes.get( pathSegment);

        if( childIter == node->m_ChildNodes.end())
            node = &*node->m_ChildNodes.emplace( pathSegment);
        else
            node = &*childIter;
    }

    if( node->is_terminal_node())
        return node->on_key_conflict( value);

    node->m_Key = path.m_Key;
    node->m_Value = value;

    return true;
};

#ifdef OVERWRITE_VALUE_ON_KEY_CONFLICT
bool
ForwardStringIndexNode::on_key_conflict( void * value)
{
    m_Value = value;
    return true;
};
#else
bool
ForwardStringIndexNode::on_key_conflict( void * /*value*/)
{
    return false;
};
#endif

/*
            / (2:nil) - (3:key) - (4:nil) - (5:key)
    (1:nil)
            \ (6:nil) - (7:nil) - (8:key)

    >>

            / (2:nil) - (3:key) - (5:key)
    (1:nil)
            \ (8:key)

    Najgłębszy węzeł jest wynoszony zaraz pod drugi najgłębszy węzeł terminalny lub rozgałęznieie.
*/

bool
ForwardStringIndexNode::prune()
{
    if( m_ChildNodes.is_empty())
        return true;

    if( (m_ChildNodes.size() == 1) && (! is_terminal_node()))
    {
        ForwardStringIndexNode & onlyChild = *m_ChildNodes.get_at( 0);

        if( onlyChild.prune())
        {
            m_Key = onlyChild.m_Key;
            m_Value = onlyChild.m_Value;

            m_ChildNodes.free();

            return true;
        }
    }

    for( ForwardStringIndexNode & childIter : m_ChildNodes)
        childIter.prune();

    return false;
};

////////////////////////////////////////////////
// ForwardStringIndexBase

ForwardStringIndexBase::ForwardStringIndexBase( void * rootNode)
:
    m_RootNode{ rootNode}
{
};

ForwardStringIndexBase::~ForwardStringIndexBase()
{
    if( m_RootNode != nullptr)
    {
        ForwardStringIndexNode * rootNode = static_cast< ForwardStringIndexNode *>( m_RootNode);

        delete rootNode;
        m_RootNode = nullptr;
    }
};

void *
ForwardStringIndexBase::base_find( std::string_view key) const
{
    ForwardStringIndexNode * rootNode = static_cast< ForwardStringIndexNode *>( m_RootNode);

    if( ForwardStringIndexNode * node = rootNode->find_terminal_node( ForwardStringIndexPath( key)))
        return node->m_Value;

    return nullptr;
};

////////////////////////////////////////////////
// ForwardStringIndexFactoryBase

ForwardStringIndexFactoryBase::ForwardStringIndexFactoryBase()
:
    m_BuildRootNode{ nullptr}
{
};

ForwardStringIndexFactoryBase::~ForwardStringIndexFactoryBase()
{
    if( m_BuildRootNode != nullptr)
    {
        ForwardStringIndexNode * rootNode = static_cast< ForwardStringIndexNode *>( m_BuildRootNode);

        delete rootNode;
        m_BuildRootNode = nullptr;
    }
};

bool
ForwardStringIndexFactoryBase::base_add( std::string_view key, void * value)
{
    if( key == EmptyKey)
        return false;

    if( m_BuildRootNode == nullptr)
        m_BuildRootNode = new ForwardStringIndexNode();

    ForwardStringIndexNode * rootNode = static_cast< ForwardStringIndexNode *>( m_BuildRootNode);

    return rootNode->insert( ForwardStringIndexPath( key), value);
};

ForwardStringIndexBase *
ForwardStringIndexFactoryBase::base_emit()
{
    if( m_BuildRootNode == nullptr)
        return nullptr;

    ForwardStringIndexNode * rootNode = static_cast< ForwardStringIndexNode *>( m_BuildRootNode);

    rootNode->prune();

    return new ForwardStringIndexBase( std::exchange( m_BuildRootNode, nullptr));
};
