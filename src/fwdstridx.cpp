
#include "fwdstridx.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string_view>
#include <utility>
#include <vector>
#include <variant>

// Private Classes

class ForwardStringIndexTerminalNode
{
 public:
    ForwardStringIndexTerminalNode( std::string_view key, void * value);

    inline void *       get_value( std::string_view key);

    std::string_view    m_Key;
    void *              m_Value;
};

struct ForwardStringIndexNode;

class ForwardStringIndexParentNode
{
 public:
    ForwardStringIndexNode *                    find( char key);

    bool                                        insert_sorted( ForwardStringIndexTerminalNode && tNode, size_t keyPos = 0);

    std::vector< ForwardStringIndexNode>        m_Nodes;
    std::vector< char>                          m_Keys;
};

class ForwardStringIndexNode
{
 public:
    std::variant
    <
        ForwardStringIndexTerminalNode,
        ForwardStringIndexParentNode
    >
    m_Node;
};

// Implementation

ForwardStringIndexTerminalNode::ForwardStringIndexTerminalNode( std::string_view key, void * value)
:
    m_Key{ key},
    m_Value{ value}
{
};

void *
ForwardStringIndexTerminalNode::get_value( std::string_view key)
{
    if( m_Key == key)
        return m_Value;

    return nullptr;
};


ForwardStringIndexNode *
ForwardStringIndexParentNode::find( char key)
{
    auto foundKey = std::lower_bound( m_Keys.begin(), m_Keys.end(), key);

    if( foundKey != m_Keys.end())
        if( *foundKey == key)
        {
            size_t pos = std::distance( m_Keys.begin(), foundKey);
            return &m_Nodes[ pos];
        }

    return nullptr;
};

bool
ForwardStringIndexParentNode::insert_sorted( ForwardStringIndexTerminalNode && tNode, size_t keyPos)
{
    char keyChar = tNode.m_Key[ keyPos];

    auto gtOrEqKeyIter = std::lower_bound( m_Keys.begin(), m_Keys.end(), keyChar);
    size_t keyIterOffset = std::distance( m_Keys.begin(), gtOrEqKeyIter);

    if( gtOrEqKeyIter != m_Keys.end())
        if( *gtOrEqKeyIter == keyChar)
        {
            keyPos ++;
            if( tNode.m_Key.length() <= keyPos)
                return false;

            ForwardStringIndexNode & node = m_Nodes[ keyIterOffset];

            if( std::holds_alternative< ForwardStringIndexParentNode>( node.m_Node))
            {
                auto & pNode = std::get< ForwardStringIndexParentNode>( node.m_Node);
                return pNode.insert_sorted( std::move( tNode), keyPos);
            }
            else
            {
                auto oldTNode = std::get< ForwardStringIndexTerminalNode>( node.m_Node);

                if( oldTNode.m_Key.length() <= keyPos)
                    return false;

                auto pNode  = ForwardStringIndexParentNode();

                if( ! pNode.insert_sorted( std::move( oldTNode), keyPos))
                     return false;

                if( ! pNode.insert_sorted( std::move( tNode), keyPos))
                    return false;

                node.m_Node = std::move( pNode);
                    return true;
            }
        }

    m_Keys.insert( /* before */ gtOrEqKeyIter, keyChar);
    m_Nodes.emplace( /* before */ m_Nodes.begin() + keyIterOffset, std::move( tNode));
    return true;
};

ForwardStringIndexNode *
find_terminal_node( ForwardStringIndexParentNode * pNode, std::string_view key)
{
    ForwardStringIndexNode * node = nullptr;

    for( char const keyChar : key)
    {
        if(!(node = pNode->find( keyChar)))
            break;

        if( std::holds_alternative< ForwardStringIndexTerminalNode>( node->m_Node))
            break;

        pNode = &std::get< ForwardStringIndexParentNode>( node->m_Node);
    }

    return node;
};

#define GET_ROOT_NODE( fsi) reinterpret_cast< ForwardStringIndexParentNode *>( fsi->m_RootNode)

ForwardStringIndex::ForwardStringIndex()
noexcept
:
    m_RootNode{ nullptr}
{
};

ForwardStringIndex::~ForwardStringIndex()
noexcept
{
    if( m_RootNode != nullptr)
    {
         auto * rootNode = GET_ROOT_NODE( this);
        delete rootNode;
    }
};

void *
ForwardStringIndex::find( char const * key, unsigned long keyLen)
const noexcept
{
    if( keyLen > 0)
    {
        std::string_view keyView{ key, keyLen};

        if( ForwardStringIndexNode * node = find_terminal_node( GET_ROOT_NODE( this), keyView))
        {
            auto & terminalNode = std::get< ForwardStringIndexTerminalNode>( node->m_Node);
            return terminalNode.get_value( keyView);
        }
    }

    return nullptr;
};

void *
ForwardStringIndex::find( char const * key)
const noexcept
{
    size_t keyLen = strlen( key);

    return find( key, keyLen);
};

ForwardStringIndexFactory::ForwardStringIndexFactory()
noexcept
:
    m_IndexBuild{ nullptr}
{
};

ForwardStringIndexFactory::~ForwardStringIndexFactory()
noexcept
{
    if( m_IndexBuild != nullptr)
        delete m_IndexBuild;
};

bool
ForwardStringIndexFactory::add( char const * key, void * value)
noexcept
{
    try
    {
        if( m_IndexBuild == nullptr)
        {
            m_IndexBuild = new ForwardStringIndex();
            m_IndexBuild->m_RootNode = new ForwardStringIndexParentNode();
        }

        std::string_view keyView{ key, strlen( key)};
        ForwardStringIndexTerminalNode tNode{ keyView, value};

        return GET_ROOT_NODE( m_IndexBuild)->insert_sorted( std::move( tNode));
    }
    catch( ...)
    {
        std::exception_ptr ex = std::current_exception();

        if( m_IndexBuild != nullptr)
            delete m_IndexBuild;

        return false;
    }
};

ForwardStringIndex *
ForwardStringIndexFactory::emit()
noexcept
{
    return std::exchange( m_IndexBuild, nullptr);
};
