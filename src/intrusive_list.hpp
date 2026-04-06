
#include <utility>
#include <cassert>

#define _LIST_EMPTY( list) ((list)->m_Head == nullptr && (list)->m_Tail == nullptr)
#define _VALID_LIST_HEAD( list) ((list)->m_Head && ((list)->m_Head->*linkMember).prev == nullptr)
#define _VALID_LIST_TAIL( list) ((list)->m_Tail && ((list)->m_Tail->*linkMember).next == nullptr)
#define _LIST_IN_VALID_STATE( list) (_VALID_LIST_HEAD( list) && _VALID_LIST_TAIL( list)) || _LIST_EMPTY( list)

template< typename T>
class ListLink
{
public:
    constexpr
    ListLink()
    :
        prev{ nullptr},
        next{ nullptr}
    {
    }

    T* prev;
    T* next;
};

template< typename T, ListLink< T> T::*linkMember, bool isOwner = false>
class IntrusiveList
{
private:
    T * m_Head;
    T * m_Tail;

    template< bool isForward, bool isConst = false>
    class iterator_base
    {
    public:
        using value_type = std::conditional< isConst, const T, T>::type;

        value_type * m_Curr;
        value_type * m_Next;

        explicit constexpr
        iterator_base( T * ptr)
        :
            m_Curr{ ptr}
        {
            set_next();
        }

        constexpr void
        set_next()
        {
            if( m_Curr)
            {
                if constexpr( isForward)
                    m_Next = (m_Curr->*linkMember).next;
                else
                    m_Next = (m_Curr->*linkMember).prev;
            }
        }

        constexpr iterator_base&
        operator++()
        {
            m_Curr = m_Next;
            set_next();
            return *this;
        }

        constexpr iterator_base
        operator++( int)
        {
            iterator_base out = *this;
            ++(*this);
            return out;
        }

        constexpr bool
        operator==( iterator_base other) const
        {
            return m_Curr == other.m_Curr;
        }

        constexpr bool
        operator!=( iterator_base other) const
        {
            return m_Curr != other.m_Curr;
        }

        constexpr value_type &
        operator*()
        {
            return *m_Curr;
        }

        constexpr value_type *
        operator->()
        {
            return m_Curr;
        }

        constexpr
        operator value_type*()
        {
            return m_Curr;
        }
    };

public:
    using iterator = iterator_base< true>;
    using reverse_iterator = iterator_base< false>;

    constexpr
    IntrusiveList()
    :
        m_Head{ nullptr},
        m_Tail{ nullptr}
    {
    }

    constexpr
    IntrusiveList( IntrusiveList const & copiedFrom)
    = delete;

    constexpr IntrusiveList &
    operator=( IntrusiveList const & copiedFrom)
    = delete;

    constexpr
    IntrusiveList( IntrusiveList && movedFrom)
    {
        if constexpr( isOwner)
            clear();

        m_Head = std::exchange( movedFrom.m_Head, nullptr);
        m_Tail = std::exchange( movedFrom.m_Tail, nullptr);

        assert( _LIST_IN_VALID_STATE( this));
    }

    constexpr IntrusiveList &
    operator=( IntrusiveList && movedFrom)
    {
        if constexpr( isOwner)
            clear();

        m_Head = std::exchange( movedFrom.m_Head, nullptr);
        m_Tail = std::exchange( movedFrom.m_Tail, nullptr);

        assert( _LIST_IN_VALID_STATE( this));

        return *this;
    }

    constexpr
    ~IntrusiveList()
    requires isOwner
    {
        clear();
    }

    constexpr
    ~IntrusiveList()
    = default;

    constexpr void
    clear()
    {
        for( T & item : *this)
            delete &item;

        m_Head = nullptr;
        m_Tail = nullptr;
    }

    constexpr void
    unlink( T & item)
    {
        assert( _LIST_IN_VALID_STATE( this));

        if( m_Head == &item)
            m_Head = (item.*linkMember).next;

        if( m_Tail == &item)
            m_Tail = (item.*linkMember).prev;

        if( T * prevItem = (item.*linkMember).prev)
            (prevItem->*linkMember).next = std::exchange( (item.*linkMember).next, nullptr);

        if( T * nextItem = (item.*linkMember).next)
            (nextItem->*linkMember).prev = std::exchange( (item.*linkMember).prev, nullptr);

        assert( _LIST_IN_VALID_STATE( this));
    }

    constexpr void
    link_front( T & item)
    {
        if( m_Head != nullptr)
            return link_before( item, *m_Head);

        assert( _LIST_EMPTY( this));

        m_Head = &item;
        m_Tail = &item;
    }

    constexpr void
    link_back( T & item)
    {
        if( m_Tail != nullptr)
            return link_after( *m_Tail, item);

        assert( _LIST_EMPTY( this));

        m_Head = &item;
        m_Tail = &item;
    }

    constexpr void
    link_before( T & item, T & beforeItem)
    {
        assert( _VALID_LIST_TAIL( this));

        if( m_Head == &beforeItem)
            m_Head = &item;

        (item.*linkMember).prev = std::exchange( (beforeItem.*linkMember).prev, &item);
        (item.*linkMember).next = &beforeItem;
    }

    constexpr void
    link_after( T & afterItem, T & item)
    {
        assert( _VALID_LIST_HEAD( this));

        if( m_Tail == &afterItem)
            m_Tail = &item;

        (item.*linkMember).prev = &afterItem;
        (item.*linkMember).next = std::exchange( (afterItem.*linkMember).next, &item);
    }

    constexpr void
    merge_front( IntrusiveList && other)
    {
        assert( _LIST_IN_VALID_STATE( this));
        assert( _LIST_IN_VALID_STATE( &other));

        if( other.m_Head == nullptr)
            return;

        if( m_Head == nullptr)
        {
            *this = other;
            return;
        }

        (m_Head->*linkMember).prev = other.m_Tail;
        (other.m_Tail->*linkMember).next = m_Head;

        m_Head = other.m_Head;

        assert( m_Head != m_Tail);

        other.m_Tail = nullptr;
        other.m_Head = nullptr;

        assert( _LIST_IN_VALID_STATE( this));
    }

    constexpr void
    merge_back( IntrusiveList && other)
    {
        assert( _LIST_IN_VALID_STATE( this));
        assert( _LIST_IN_VALID_STATE( &other));

        if( other.m_Tail == nullptr)
            return;

        if( m_Tail == nullptr)
        {
            *this = other;
            return;
        }

        (m_Tail->*linkMember).next = other.m_Head;
        (other.m_Head->*linkMember).prev = m_Tail;

        m_Tail = other.m_Tail;

        other.m_Tail = nullptr;
        other.m_Head = nullptr;

        assert( _LIST_IN_VALID_STATE( this));
    }

    template< typename ... ConstructorArgs>
    constexpr T &
    emplace_front( ConstructorArgs && ... args)
    {
        T * item = new T( args...);
        link_front( *item);
        return *item;
    }

    template< typename ... ConstructorArgs>
    constexpr T &
    emplace_back( ConstructorArgs && ... args)
    {
        T * item = new T( args...);
        link_back( *item);
        return *item;
    }

    constexpr iterator
    begin()
    {
        return m_Head ? iterator{ m_Head} : iterator{ nullptr};
    }

    constexpr iterator
    end()
    {
        return iterator{ nullptr};
    }

    constexpr reverse_iterator
    rbegin()
    {
        return m_Tail ? reverse_iterator{ m_Tail} : reverse_iterator{ nullptr};
    }

    constexpr reverse_iterator
    rend()
    {
        return reverse_iterator{ nullptr};
    }
};

#undef _LIST_IN_VALID_STATE
#undef _VALID_LIST_TAIL
#undef _VALID_LIST_HEAD
#undef _LIST_EMPTY
