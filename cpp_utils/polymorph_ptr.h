#pragma once

template<class T> struct weak_polymorph_ptr;

template<class T>
struct polymorph_ptr
{
public:
    typedef polymorph_ptr           this_type;
    typedef boost::shared_ptr<T>    underlied_ptr;

public:
    polymorph_ptr(std::nullptr_t)
    {
    }

    explicit polymorph_ptr(T* p = 0)
        : underlied_ptr_(p)
    {
    }

    template<class U>
    explicit polymorph_ptr(U* p)
        : underlied_ptr_(boost::dynamic_pointer_cast<T>(boost::shared_ptr<U>(p)))
    {
    }

    explicit polymorph_ptr(boost::weak_ptr<T> const& p)
        : underlied_ptr_(p)
    {
    }

    polymorph_ptr(polymorph_ptr const& other)
        : underlied_ptr_(other.underlied())
    {
    }

    template<class U>
    polymorph_ptr(boost::shared_ptr<U> const& other)
        : underlied_ptr_(boost::dynamic_pointer_cast<T>(other))
    {
    }

    template<class U>
    polymorph_ptr(polymorph_ptr<U> const& other)
        : underlied_ptr_(boost::dynamic_pointer_cast<T>(other.underlied()))
    {
    }

    operator boost::weak_ptr<T>() const
    {
        return boost::weak_ptr<T>(underlied_ptr_);
    }

    polymorph_ptr& operator=(polymorph_ptr const& other)
    {
        underlied_ptr_ = other.underlied_ptr_;
        return *this;
    }

    template<class U>
    polymorph_ptr& operator=(polymorph_ptr<U> const& other)
    {
        underlied_ptr_ = boost::dynamic_pointer_cast<T>(other.underlied());
        return *this;
    }

    ~polymorph_ptr()
    {
        //static_assert(boost::has_virtual_destructor<T>::value,"Base class must have virtual destructor");
    }

    void reset()
    {
        underlied_ptr_.reset();
    }

    template<class U>
    void reset(U* other = 0)
    {
        underlied_ptr_.reset(other);
    }

    T* get() const
    {
        return underlied_ptr_.get();
    }

    T* operator->() const
    {
        return underlied_ptr_.operator->();
    }

    T& operator* () const
    {
        return underlied_ptr_.operator*();
    }

    underlied_ptr const& underlied() const
    {
        return underlied_ptr_;
    }

    // safe bool operator
    typedef T*(this_type::*unspecified_bool_type)() const;
    operator unspecified_bool_type() const
    {
        return underlied_ptr_ ? &polymorph_ptr::get : 0;
    }

    bool unique() const
    {
        return underlied_ptr_.unique();
    }

    void swap(polymorph_ptr& other)
    {
        underlied_ptr_.swap(other.underlied_ptr_);
    }

private:
    underlied_ptr underlied_ptr_;
};

template<class T, class U>
   bool operator==(polymorph_ptr<T> const & a, polymorph_ptr<U> const & b)
   {
       auto b_casted = boost::dynamic_pointer_cast<T>(b.underlied());
       if (b.underlied() && b_casted == 0)
           return false;
       return a.underlied() == b_casted;
   }

 template<class T, class U>
   bool operator!=(polymorph_ptr<T> const & a, polymorph_ptr<U> const & b)
   {
       return !(a == b);
   }

 template<class T, class U>
   bool operator<(polymorph_ptr<T> const & a, polymorph_ptr<U> const & b)
   {
       return (a != b) && a.underlied() < b.underlied();
   }

 template<class T> void swap(polymorph_ptr<T>& a, polymorph_ptr<T>& b)
   {
       a.swap(b);
   }

 template<class T> T * get_pointer(polymorph_ptr<T> const & p)
   {
       return get_pointer(p.underlied());
   }
