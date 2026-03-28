
#ifndef _UNIQUE_PTR_STACK_H_
# define _UNIQUE_PTR_STACK_H_

# include <memory>
# include <list>

template< typename _DataT>
class UniquePtrStack
{
    using _ContainerT = std::list< std::unique_ptr< _DataT>>;
    using iterator = _ContainerT::iterator;

    _ContainerT m_Data;

 public:

    constexpr iterator
    begin()
    {   return m_Data.rbegin(); };

    constexpr iterator
    end()
    {   return m_Data.rend(); };

    constexpr void
    push( _DataT && data)
    {   m_Data.emplace_back( std::make_unique< _DataT>( std::move( data))); };

    template< typename ... _ConstructorArgs>
    constexpr void
    push( _ConstructorArgs && ... args)
    {   m_Data.emplace_back( std::make_unique< _DataT>( std::forward< _ConstructorArgs>( args) ...)); };

    constexpr std::unique_ptr< _DataT>
    pop()
    {
        std::unique_ptr< _DataT> & topElementRef = m_Data.back();

        if( topElementRef == nullptr)
            return nullptr;

        std::unique_ptr< _DataT> topElement{ std::move( topElementRef)};
        m_Data.pop_back();

        return topElement;
    };
 };

#endif//_UNIQUE_PTR_STACK_H_
