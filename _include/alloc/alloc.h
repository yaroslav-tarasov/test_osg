#pragma once
#include "common/dyn_lib.h"

#ifdef ALLOC_LIB
# define ALLOC_API __HELPER_DL_EXPORT
#else
# define ALLOC_API __HELPER_DL_IMPORT
#endif


// single-element pool (set, heap, map, list, ...)
struct pool_alloc_t
{
    virtual ~pool_alloc_t() {}

    virtual void * allocate() = 0;
    virtual void deallocate( void * ) = 0;
};
typedef boost::shared_ptr<pool_alloc_t> pool_alloc_ptr;
ALLOC_API pool_alloc_ptr get_pool_allocator( unsigned item_size );


// multiple-elements segregated storage (vector, deque)
struct seg_storage_alloc_t
{
    virtual ~seg_storage_alloc_t() {}

    virtual void * allocate( unsigned ) = 0;
    virtual void deallocate( void * , unsigned ) = 0;
};
typedef boost::shared_ptr<seg_storage_alloc_t> seg_storage_alloc_ptr;
ALLOC_API seg_storage_alloc_ptr get_seg_storage_allocator( unsigned item_size );


// tiny strings storage pool (for strings less than 64 chars - pools, else - malloc/free)
struct tiny_string_alloc_t
{
    virtual ~tiny_string_alloc_t() {}

    virtual void * allocate( unsigned ) = 0;
    virtual void deallocate( void * , unsigned ) = 0;
};
typedef boost::shared_ptr<tiny_string_alloc_t> tiny_string_alloc_ptr;
ALLOC_API tiny_string_alloc_ptr get_tiny_string_allocator();
