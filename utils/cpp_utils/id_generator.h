#pragma once

#include "alloc/pool_stl.h"


namespace utils
{

template <class T, unsigned MIN = 0>
struct fixed_id_generator
{
    fixed_id_generator() : last_id(MIN){}

    // get new id
    unsigned create()
    {
        unsigned int id = last_id;
        if (!free_ids.empty())
        {
            auto it = free_ids.begin();
            id = *it;
            free_ids.erase(it);
        }
        else
            ++last_id;
        return id;
    }
    // release used id
    void release( unsigned int id )
    {
        free_ids.insert(id);
        _tidy();
    }

    // really used ids
    unsigned size() const
    {
        return last_id - free_ids.size();
    }
    // allocated ids
    unsigned capacity() const
    {
        return last_id;
    }

private:

    void _tidy()
    {
        auto it = free_ids.rbegin();
        for (auto rend = free_ids.rend(); it != rend && *it == last_id - 1; ++it)
            --last_id;
        free_ids.erase(it.base(), free_ids.end());
    }

private:

    unsigned last_id;
    ph_set<unsigned>::set_t free_ids;
};

}
