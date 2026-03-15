
#ifndef _FORWARD_STRING_INDEX_
# define _FORWARD_STRING_INDEX_

# include <string_view>

class ForwardStringIndex
{
    friend class ForwardStringIndexFactory;

 private:
    ForwardStringIndex();

 public:
    ~ForwardStringIndex();

    void *                      find( std::string_view key) const;
    void *                      find( char const * key) const;

 private:
    void *                      m_RootNode;
};

class ForwardStringIndexFactory
{
 public:
    ForwardStringIndexFactory();
    ~ForwardStringIndexFactory();

    ForwardStringIndex *        emit();
    bool                        add( char const * key, void * value);

 private:
    ForwardStringIndex *        m_IndexBuild;
};

#endif//_FORWARD_STRING_INDEX_
