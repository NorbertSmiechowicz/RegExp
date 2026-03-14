
#ifndef _FWDSTRIDX_H_
# define _FWDSTRIDX_H_

class ForwardStringIndex
{
    friend class ForwardStringIndexFactory;

 private:
    ForwardStringIndex() noexcept;

 public:
    ~ForwardStringIndex() noexcept;

    void * find( char const * key, unsigned long keyLen) const noexcept;
    void * find( char const * key) const noexcept;

 private:
    void * m_RootNode;
};

class ForwardStringIndexFactory
{
 public:
    ForwardStringIndexFactory() noexcept;
    ~ForwardStringIndexFactory() noexcept;

    ForwardStringIndex * emit() noexcept;
    bool add( char const * key, void * value) noexcept;

 private:
    ForwardStringIndex * m_IndexBuild;
};

#endif//_FWDSTRIDX_H_
