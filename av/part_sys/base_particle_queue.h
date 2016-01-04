#pragma once

namespace av
{

namespace part_sys
{

    template<class TParticle, unsigned blocksize = 1024U>
    struct base_particle_queue
    {
        typedef TParticle particle_type_t;

    #pragma pack(push, 1)
        struct link_t
        {
            particle_type_t particle;
            link_t * next;
        };
    #pragma pack(pop)

        struct iterator
        {
            iterator();

            iterator & operator++();
            bool operator == ( const iterator & other ) const;
            bool operator != ( const iterator & other ) const;
            TParticle * operator -> ();
            TParticle & operator * ();

        private:
            friend struct base_particle_queue;

            iterator( link_t * const * pLink );
            link_t * const * ptr_;
        };

        base_particle_queue();
        ~base_particle_queue();

        iterator begin() const;
        iterator end  () const;

        void push_front( const particle_type_t & particle );
        TParticle *push_front();

        iterator erase( iterator iter );
        void clear();

        unsigned size() const;

    private:

        link_t * head_;
        unsigned size_;

        void erase( link_t ** pLink );
        TParticle * reserve_front();

        static __forceinline unsigned & ref_counter()
        {
            static unsigned counter = 0;
            return counter;
        }

        static __forceinline link_t *& freelist_head()
        {
            static link_t * head = nullptr;
            return head;
        }

        typedef std::vector<void *> block_list_t;
        static __forceinline block_list_t & block_list()
        {
            static block_list_t list;
            return list;
        }
    };


    // iterator

    template <class P, unsigned B>
    base_particle_queue<P,B>::iterator::iterator()
        : ptr_(nullptr)
    {}

    template <class P, unsigned B>
    __forceinline typename base_particle_queue<P,B>::iterator & base_particle_queue<P,B>::iterator::operator++ ()
    {
        Assert(ptr_);
        ptr_ = &((*ptr_)->next);
        return *this;
    }

    template <class P, unsigned B>
    __forceinline bool base_particle_queue<P,B>::iterator::operator== ( iterator const& other ) const
    {
        return other.ptr_ == ptr_ || (!other.ptr_ && !(*ptr_)) || (!ptr_ && !(*other.ptr_));
    }

    template <class P, unsigned B>
    __forceinline bool base_particle_queue<P,B>::iterator::operator!= ( iterator const& other ) const
    {
        return !(*this == other) ;
    }

    template <class P, unsigned B>
    __forceinline P * base_particle_queue<P,B>::iterator::operator-> ()
    {
        Assert(ptr_);
        return &((*ptr_)->particle);
    }

    template <class P, unsigned B>
    __forceinline P & base_particle_queue<P,B>::iterator::operator* ()
    {
        Assert(ptr_);
        return ((*ptr_)->particle);
    }

    template <class P, unsigned B>
    base_particle_queue<P,B>::iterator::iterator( link_t * const * ptr )
        : ptr_(ptr)
    {
    }

    // queue

    template <class P, unsigned B>
    base_particle_queue<P,B>::base_particle_queue()
        : head_(nullptr)
        , size_(0)
    {
        ++ref_counter();
    }

    template <class P, unsigned B>
    base_particle_queue<P,B>::~base_particle_queue()
    {
        clear();
        --ref_counter();

        if (!ref_counter())
        {
            for (unsigned i = 0; i != block_list().size(); i++)
                free(block_list()[i]);
            block_list().clear();
            freelist_head() = nullptr;
        }
    }

    template <class P, unsigned B>
    __forceinline typename base_particle_queue<P,B>::iterator base_particle_queue<P,B>::begin() const
    {
        return iterator(&head_);
    }

    template <class P, unsigned B>
    __forceinline typename base_particle_queue<P,B>::iterator base_particle_queue<P,B>::end() const
    {
        return iterator();
    }

    template <class P, unsigned B>
    __forceinline P * base_particle_queue<P,B>::reserve_front()
    {
        if (!freelist_head())
        {
            link_t * block = (link_t *)calloc(B, sizeof(link_t));
            for (unsigned i = 0; i < B - 1; i++)
                block[i].next = &block[i + 1];
            //block[B - 1].next = nullptr;

            block_list().emplace_back(block);
            freelist_head() = block;
        }

        link_t * newLink = freelist_head();
        freelist_head() = freelist_head()->next;

        newLink->next = head_;
        head_ = newLink;

        ++size_;
        return &newLink->particle;
    }

    template <class P, unsigned B>
    __forceinline void base_particle_queue<P,B>::push_front( P const & particle )
    {
        new (reserve_front()) P(particle);
    }

    template <class P, unsigned B>
    __forceinline P * base_particle_queue<P,B>::push_front()
    {
        return (new (reserve_front()) P());
    }

    template <class P, unsigned B>
    __forceinline typename base_particle_queue<P,B>::iterator base_particle_queue<P,B>::erase( iterator iter )
    {
        erase(const_cast<link_t **>(iter.ptr_));
        return iter;
    }

    template <class P, unsigned B>
    __forceinline void base_particle_queue<P,B>::clear()
    {
        while (head_)
            erase(&head_);
    }

    template <class P, unsigned B>
    __forceinline unsigned base_particle_queue<P,B>::size() const
    {
        return size_;
    }

    template <class P, unsigned B>
    __forceinline void base_particle_queue<P,B>::erase( link_t ** plink )
    {
        Assert(plink && *plink);

        link_t *delLink = *plink;
        (*plink) = (*plink)->next;
        delLink->particle.~P();

        --size_;

        delLink->next = freelist_head();
        freelist_head() = delLink;
    }

} // namespace part_sys

} // namespace av
