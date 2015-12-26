#pragma once

namespace utils
{

template <class T>
class singleton
{
public:
    static T& instance()
    {
        static T instance_;
        return instance_;
    }

private:
    singleton();
    ~singleton();
    singleton(const singleton &);
    singleton& operator=(const singleton &);
};

}