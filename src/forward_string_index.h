
#ifndef _FORWARD_STRING_INDEX_
# define _FORWARD_STRING_INDEX_

# include <memory>
# include <string_view>


class ForwardStringIndexBase
{
    friend class ForwardStringIndexFactoryBase;

protected:
    ForwardStringIndexBase( void * rootNode);
    ~ForwardStringIndexBase();

    void * base_find( std::string_view key) const;

    void * m_RootNode;
};

class ForwardStringIndexFactoryBase
{
protected:
    ForwardStringIndexFactoryBase();
    ~ForwardStringIndexFactoryBase();

    bool base_add( std::string_view key, void * value);
    ForwardStringIndexBase * base_emit();

    void * m_BuildRootNode;
};

template< typename ValueT>
class ForwardStringIndex : public ForwardStringIndexBase
{
public:

    ValueT *
    find( std::string_view key) const
    {   return reinterpret_cast< ValueT *>( base_find( key)); };

};

template< typename ValueT>
class ForwardStringIndexFactory : public ForwardStringIndexFactoryBase
{
public:

    bool
    add( std::string_view key, ValueT * value)
    {   return base_add( key, reinterpret_cast< void *>( value)); };

    ForwardStringIndex< ValueT> *
    emit()
    {   return static_cast< ForwardStringIndex< ValueT> *>( base_emit()); };

};

#endif//_FORWARD_STRING_INDEX_
