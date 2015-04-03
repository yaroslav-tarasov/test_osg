#pragma once

//#include "nfi/func_collection.h"

#if defined _WIN32 || defined __CYGWIN__
#   include "win_lib_loader.h"
#elif defined __GNUC__
#   include "unix_lib_loader.h"
#endif

#define DEFINE_FUNC_POINTER_TYPE(func_name,type_name)  \
    auto pointer_##func_name = &func_name;             \
    typedef decltype(pointer_##func_name) type_name;     

struct lib_loader_t
{
    lib_loader_t();
    ~lib_loader_t();

    void        dispose();
    void        load_lib(std::string const& lib_name);
    std::string find_full_name(std::string const& lib_name, std::string const& short_name);
    sym_descriptor_t get_symbol(std::string const& lib_name, std::string const& short_name);
    //nfi::details::func_collection_t const& collection(string const& lib_name) const;

//private:
//    void on_lib(fs::path const& lib_path) const;

private:
    //mutable unordered_map<
    //    string,
    //    nfi::details::func_collection_t const*> collections_;

    mutable std::unordered_map<
        std::string,
         std::list<std::string>> collections_;

    mutable std::unordered_map<
        std::string,
        lib_descriptor_t>        libs_;
};

lib_loader_t& lib_loader();

