
#include "forward_string_index.h"
#include "indexd_vector.h"

#include <cstring>
#include <string_view>
#include <utility>

////////////////////////////////////////////////
// ForwardStringIndexNode

class ForwardStringIndexNode
{
public:
    using IdxVector = IndexedVector< char, ForwardStringIndexNode>;

    ForwardStringIndexNode( std::string_view key, void * value);

    ForwardStringIndexNode *    find_child_node( char key);
    ForwardStringIndexNode *    find_terminal_node( std::string_view key);

    bool                        insert( std::string_view key, std::size_t charPos, void * value);
    bool                        insert_resolve_conflict( ForwardStringIndexNode & conflictingNode, std::string_view key, std::size_t charPos, void * value);
    bool                        on_key_overwrite( void * value);

    std::string_view            m_Key;
    void *                      m_Value;
    IdxVector                   m_ChildNodes;
};

ForwardStringIndexNode::ForwardStringIndexNode( std::string_view key, void * value)
:
    m_Key{ key},
    m_Value{ value}
{
};

ForwardStringIndexNode *
ForwardStringIndexNode::find_child_node( char keyChar)
{
    IdxVector::iterator childIter = m_ChildNodes.get( keyChar);

    if( childIter != m_ChildNodes.end())
        return &*childIter;

    return nullptr;
};

ForwardStringIndexNode *
ForwardStringIndexNode::find_terminal_node( std::string_view key)
{
    ForwardStringIndexNode * node = this;

    for( char const keyChar : key)
        if( ! (node = node->find_child_node( keyChar)))
            return nullptr;

    if( node->m_Key == key)
        return node;

    return nullptr;
};

bool
ForwardStringIndexNode::insert( std::string_view key, std::size_t charPos, void * value)
{
    char keyChar = key[ charPos];

    IdxVector::iterator conflictingNodeIter = m_ChildNodes.get( keyChar);

    if( conflictingNodeIter == m_ChildNodes.end())
    {
        m_ChildNodes.emplace( keyChar, key, value);
        return true;
    }

    return insert_resolve_conflict( *conflictingNodeIter, key, charPos, value);
};

bool
ForwardStringIndexNode::insert_resolve_conflict( ForwardStringIndexNode & conflictingNode, std::string_view key, std::size_t charPos, void * value)
{
    charPos ++;

    std::size_t newKeyLen = key.length();
    std::size_t conflictingKeyLen = conflictingNode.m_Key.length();

    if( ! (conflictingKeyLen > charPos)) // W starym kluczu już nic nie ma ...
    {
        if( newKeyLen > charPos) // ... ale w nowym jeszcze mamy znaki - więc nowy zawiera całkowicie stary w przedrostku.
        {
            return conflictingNode.insert( key, charPos, value);
        }
        else // ... i w nowym też, a mamy konflikt - więc są takie same i wpisujemy wartość do isteniejącego klucza.
        {
            return conflictingNode.on_key_overwrite( value);
        }
    }
    else // Stary klucz ma więcej znaków ...
    {
        if( ! (newKeyLen > charPos)) // ... ale w nowym się skończyły - czyli nowy zawiera się całkowicie w przedrostku starego.
        {
            if( ! conflictingNode.insert( conflictingNode.m_Key, charPos, conflictingNode.m_Value))
                return false;

            conflictingNode.m_Key = key;
            conflictingNode.m_Value = value;

            return true;
        }
        else // ... i w nowym też - czyli wspólny przedrostek nie prowadzi do unikalnej wartości.
        {
            if( ! conflictingNode.insert( conflictingNode.m_Key, charPos, conflictingNode.m_Value))
                return false;

            conflictingNode.m_Key = "";
            conflictingNode.m_Value = nullptr;

            return conflictingNode.insert( key, charPos, value);
        }
    }
};

#ifdef OVERWRITE_VALUE_ON_KEY_CONFLICT
bool
ForwardStringIndexNode::on_key_overwrite( void * value)
{
    m_Value = value;
    return true;
};
#else
bool
ForwardStringIndexNode::on_key_overwrite( void * /*value*/)
{
    return false;
};
#endif

////////////////////////////////////////////////
// ForwardStringIndex

#define GET_ROOT_NODE( fsi) reinterpret_cast< ForwardStringIndexNode *>( fsi->m_RootNode)

ForwardStringIndex::ForwardStringIndex()
:
    m_RootNode{ nullptr}
{
};

ForwardStringIndex::~ForwardStringIndex()
{
    if( m_RootNode != nullptr)
    {
        ForwardStringIndexNode * rootNode = GET_ROOT_NODE( this);
        delete rootNode;
    }
};

void *
ForwardStringIndex::find( std::string_view key)
const
{
    if( ForwardStringIndexNode * node = GET_ROOT_NODE( this)->find_terminal_node( key))
        return node->m_Value;

    return nullptr;
};

void *
ForwardStringIndex::find( char const * key)
const
{
    size_t keyLen = strlen( key);

    return find( std::string_view( key, keyLen));
};

////////////////////////////////////////////////
// ForwardStringIndexFactory

ForwardStringIndexFactory::ForwardStringIndexFactory()
:
    m_IndexBuild{ nullptr}
{
};

ForwardStringIndexFactory::~ForwardStringIndexFactory()
{
    if( m_IndexBuild != nullptr)
        delete m_IndexBuild;
};

bool
ForwardStringIndexFactory::add( char const * key, void * value)
{
    std::size_t keyLen = strlen( key);

    if( keyLen == 0)
        return false;

    if( m_IndexBuild == nullptr)
    {
        m_IndexBuild = new ForwardStringIndex();
        m_IndexBuild->m_RootNode = new ForwardStringIndexNode( "", nullptr);
    }

    std::string_view keyView{ key, keyLen};

    return GET_ROOT_NODE( m_IndexBuild)->insert( keyView, 0, value);
};

ForwardStringIndex *
ForwardStringIndexFactory::emit()
{
    return std::exchange( m_IndexBuild, nullptr);
};
