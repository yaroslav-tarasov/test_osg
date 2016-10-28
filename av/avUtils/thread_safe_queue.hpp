#pragma once


namespace Utils
{

template <typename T>
class thread_safe_queue
{
    typedef  std::queue< osg::ref_ptr<T> > ts_queue_t;

    mutable OpenThreads::Mutex  mutex_;
    ts_queue_t               the_queue;

public:
    template<typename T>
    void push(T* p)
    {        
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock_(mutex_);
        //std::lock_guard<std::mutex> lk(m);
        // the_queue.push(std::make_shared<wrapped_message<T> >(msg));
        the_queue.push(p);
        //c.notify_all();
    }

    T* wait_and_pop()
    {        
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock_(mutex_);
        //std::unique_lock<std::mutex> lk(m);
        //c.wait(lk,[&]{return !q.empty();});
        T* res=the_queue.front().release();
        the_queue.pop();
        return res;
    }

    typename ts_queue_t::size_type size() const
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock_(mutex_);
        return the_queue.size();
    }

    bool empty() const
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock_(mutex_);
        return the_queue.empty();
    }

    T* try_pop()
    {
        // std::lock_guard<std::mutex> lk(mut);
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock_(mutex_);
        if(the_queue.empty())
            return nullptr;
        osg::ref_ptr<T> res=the_queue.front();
        the_queue.pop();
        return res.release();
    }

};


}