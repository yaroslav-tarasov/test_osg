#pragma once

#include "alloc.h"

#include <limits>


//
// STL-compatible private-heap based single-element allocator
//

template<class T>
struct type_size_base 
{
protected:
    type_size_base()                        : type_size_of(sizeof(T)) {}
    type_size_base(type_size_base const&)   : type_size_of(sizeof(T)) {}

    type_size_base& operator=(type_size_base const&) { return *this; }

    const unsigned type_size_of;
};

template<typename T>
struct private_heap_allocator
    : type_size_base<T>
{
public: // typedefs
    typedef T                   value_type;
    typedef value_type *        pointer;
    typedef const value_type *  const_pointer;
    typedef value_type &        reference;
    typedef const value_type &  const_reference;
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;

public: // convert an allocator<T> to allocator<U>
    template<typename U> struct rebind { typedef private_heap_allocator<U> other; };

private: // heap ref
    pool_alloc_ptr heap;

protected:
    using type_size_base<T>::type_size_of;

public: // ctors
    __forceinline private_heap_allocator() : heap(get_pool_allocator(type_size_of)) {}
    __forceinline private_heap_allocator( const private_heap_allocator & other ) : type_size_base<T>(), heap(other.heap) { }

    template<typename U> 
    __forceinline private_heap_allocator( const private_heap_allocator<U> & ) : heap(get_pool_allocator(type_size_of)) { }

public: // address
    __forceinline pointer address( reference r ) const { return &r; }
    __forceinline const_pointer address( const_reference r ) { return &r; }

public: // memory allocation
    __forceinline pointer allocate( size_type count, typename std::allocator<void>::const_pointer hint = nullptr )
    {
        (void)hint;
        Assert(count == 1);
        return (pointer)heap->allocate();
    }
    __forceinline void deallocate( pointer p, size_type count )
    {
        Assert(count == 1);
        heap->deallocate(p);
    }

public: // max_size
    __forceinline size_type max_size() const
    {
        return std::numeric_limits<size_type>::max() / type_size_of;
    }

public: // objects construction/destruction
    template<class U, class Args>
    __forceinline void construct( U * p, Args && args ) { new(p) U(std::forward<Args>(args)); }

    template<class U>
    __forceinline void destroy( U * p ) { (void)p; p->~U(); }
};

template<class T1, class T2>
bool operator==(const private_heap_allocator<T1> &, const private_heap_allocator<T2> &)
{
    return true;
}
template<class T1, class T2>
bool operator!=(const private_heap_allocator<T1> &, const private_heap_allocator<T2> &)
{
    return false;
}

//! хелперы для коллекций с использованием аллокатора private_heap_allocator

// helpers for set
template <typename T, typename Pred = std::less<T>>
    struct ph_set
{
    typedef std::set     <T, Pred, private_heap_allocator<T>>   set_t;
    typedef std::multiset<T, Pred, private_heap_allocator<T>>   multiset_t;
};
// helpers for map
template <typename K, typename T, typename Pred = std::less<K>>
    struct ph_map
{
    typedef std::map     <K, T, Pred, private_heap_allocator<std::pair<const K, T>>>   map_t;
    typedef std::multimap<K, T, Pred, private_heap_allocator<std::pair<const K, T>>>   multimap_t;
};
// helpers for list
template <typename T>
    struct ph_list
{
    typedef std::list<T, private_heap_allocator<T>> list_t;
};


//
// segregated storage
//

template<typename T>
struct ss_pool_allocator
{
public: // typedefs
    typedef T                   value_type;
    typedef value_type *        pointer;
    typedef const value_type *  const_pointer;
    typedef value_type &        reference;
    typedef const value_type &  const_reference;
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;

public: // convert an allocator<T> to allocator<U>
    template<typename U> struct rebind { typedef ss_pool_allocator<U> other; };

private: // heap ref
    static const unsigned type_size_of = sizeof(T);
    seg_storage_alloc_ptr ss_pool;

public: // ctors
    inline ss_pool_allocator() : ss_pool(get_seg_storage_allocator(type_size_of)) {}
    inline ss_pool_allocator( const ss_pool_allocator & other ) : ss_pool(other.ss_pool) { }

    template<typename U> 
    inline ss_pool_allocator( const ss_pool_allocator<U> & ) : ss_pool(get_seg_storage_allocator(type_size_of)) { }

public: // address
    inline pointer address( reference r ) const { return &r; }
    inline const_pointer address( const_reference r ) { return &r; }

public: // memory allocation
    inline pointer allocate( size_type count, typename std::allocator<void>::const_pointer hint = nullptr )
    {
        (void)hint;
        return (pointer)ss_pool->allocate(count);
    }
    inline void deallocate( pointer p, size_type count )
    {
        ss_pool->deallocate(p, count);
    }

public: // max_size
    inline size_type max_size() const
    {
        return std::numeric_limits<size_type>::max() / type_size_of;
    }

public: // objects construction/destruction
    inline void construct( pointer p, const_reference val ) { new(p) value_type(val); }
    inline void destroy( pointer p ) { (void)p; p->~value_type(); }
};

template<class T1, class T2>
bool operator==(const ss_pool_allocator<T1> &, const ss_pool_allocator<T2> &)
{
    return true;
}
template<class T1, class T2>
bool operator!=(const ss_pool_allocator<T1> &, const ss_pool_allocator<T2> &)
{
    return false;
}

//////////////////////////////////////////////////////////////////////////
//
// works slowler than usual std::allocator
//


// helpers for vector
// template <typename T>
//     struct ph_vector
// {
//     typedef std::vector<T, ss_pool_allocator<T>> vector_t;
// };
// helpers for deque
// template <typename T>
//     struct ph_deque
// {
//     typedef std::deque<T, ss_pool_allocator<T>> deque_t;
// };


//
// std::string allocator for small string
//

struct tiny_string_allocator
{
public: // typedefs
    typedef char                value_type;
    typedef value_type *        pointer;
    typedef const value_type *  const_pointer;
    typedef value_type &        reference;
    typedef const value_type &  const_reference;
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;

public: // convert
    template<typename U> struct rebind { typedef tiny_string_allocator other; };

private: // heap refs
    tiny_string_alloc_ptr heap;

public: // ctors
    __forceinline tiny_string_allocator() : heap(get_tiny_string_allocator()) {}
    __forceinline tiny_string_allocator( const tiny_string_allocator & other ) : heap(other.heap) { }

public: // address
    __forceinline pointer address( reference r ) const { return &r; }
    __forceinline const_pointer address( const_reference r ) { return &r; }

public: // memory allocation
    __forceinline pointer allocate( size_type count, const void * hint = nullptr )
    {
        (void)hint;
        return (pointer)heap->allocate(count);
    }
    __forceinline void deallocate( pointer p, size_type count )
    {
        heap->deallocate(p, count);
    }

public: // max_size
    __forceinline size_type max_size() const
    {
        return std::numeric_limits<size_type>::max();
    }

public: // objects construction/destruction
    template<class U, class Args>
    __forceinline void construct( U * p, Args && args ) { new(p) U(std::forward<Args>(args)); }

    template<class U>
    __forceinline void destroy( U * p ) { (void)p; p->~U(); }
};

namespace std
{
    typedef basic_string<char, char_traits<char>, tiny_string_allocator> tiny_string;
}
