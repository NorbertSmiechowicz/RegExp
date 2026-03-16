
#ifndef _INDEXED_VECTOR_
# define _INDEXED_VECTOR_

# include <algorithm>
# include <vector>

template< typename KeyT, typename DataT>
class IndexedVector
{
    using KeyVector     = std::vector< KeyT>;
    using DataVector    = std::vector< DataT>;

    KeyVector   m_Keys;
    DataVector  m_Data;

public:
    using iterator = DataVector::iterator;
    using reverse_iterator = DataVector::reverse_iterator;

    constexpr iterator
    begin() noexcept
    {   return m_Data.begin(); };

    constexpr iterator
    end() noexcept
    {   return m_Data.end(); };

    constexpr reverse_iterator
    rbegin() noexcept
    {   return m_Data.rbegin(); };

    constexpr reverse_iterator
    rend() noexcept
    {   return m_Data.rend(); };

    constexpr iterator
    get( std::size_t pos) noexcept
    {
        if( pos < m_Data.size())
            return m_Data.begin() + pos;

        return m_Data.end();
    };

    constexpr iterator
    get( KeyVector::iterator key) noexcept
    {   return m_Data.begin() + std::distance( m_Keys.begin(), key); };

    constexpr iterator
    get( KeyT key) noexcept
    {
        auto greaterOrEqualKeyIter = std::lower_bound( m_Keys.begin(), m_Keys.end(), key);

        if( greaterOrEqualKeyIter != m_Keys.end())
            if( *greaterOrEqualKeyIter == key)
                return get( greaterOrEqualKeyIter);

        return m_Data.end();
    };

    constexpr iterator
    insert( KeyT key, DataT const & data) noexcept
    {
        auto greaterOrEqualKeyIter = std::lower_bound( m_Keys.begin(), m_Keys.end(), key);

        // Kolejność jest ważna z uwagi na unieważnienie iteratora

        auto inserted = m_Data.insert( get( greaterOrEqualKeyIter), data);
        m_Keys.insert( greaterOrEqualKeyIter, key);

        return inserted;
    };

    constexpr iterator
    insert( KeyT key, DataT && data) noexcept
    {
        auto greaterOrEqualKeyIter = std::lower_bound( m_Keys.begin(), m_Keys.end(), key);

        auto inserted = m_Data.insert( get( greaterOrEqualKeyIter), std::move( data));
        m_Keys.insert( greaterOrEqualKeyIter, key);

        return inserted;
    };

    template< typename ... _ConstructorArgs>
    constexpr iterator
    emplace( KeyT key, _ConstructorArgs && ... args) noexcept
    {
        auto greaterOrEqualKeyIter = std::lower_bound( m_Keys.begin(), m_Keys.end(), key);

        auto inserted = m_Data.emplace( get( greaterOrEqualKeyIter), std::forward< _ConstructorArgs>( args)...);
        m_Keys.insert( greaterOrEqualKeyIter, key);

        return inserted;
    };

    constexpr iterator
    remove( KeyT key) noexcept
    {
        auto greaterOrEqualKeyIter = std::lower_bound( m_Keys.begin(), m_Keys.end(), key);

        if( greaterOrEqualKeyIter != m_Keys.end())
            if( *greaterOrEqualKeyIter == key)
            {
                auto removed = m_Data.erase( get( greaterOrEqualKeyIter));
                m_Keys.erase( greaterOrEqualKeyIter);

                return removed;
            }

        return m_Data.end();
    };
};

#endif//_INDEXED_VECTOR_
