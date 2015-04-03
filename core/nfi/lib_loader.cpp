#include "stdafx.h"
#include "lib_loader.h"
//#include "lib_iteration.h"

//using namespace nfi;
//using namespace nfi::details;

lib_loader_t::lib_loader_t()
{
    //-- it's not right to preload all the libs in working dir. Some of them could create dangerous static objects on load
    //iterate_libs(lib_extension(), bind(&lib_loader_t::on_lib, this, _1), libs_path());
}


lib_loader_t::~lib_loader_t()
{
    dispose();
}

//func_collection_t const& lib_loader_t::collection(string const& lib_name) const
//{
//    auto it = collections_.find(lib_name);
//
//    if (it == collections_.end()) // let's try to reload it 
//        load_lib(lib_name + "." + lib_extension(), bind(&lib_loader_t::on_lib, this, _1), libs_path());
//
//    it = collections_.find(lib_name);
//    VerifyMsg(it != collections_.end(), "nfi error, symbols collection not found for: " << lib_name);
//
//    return *(it->second);
//}

sym_descriptor_t lib_loader_t::get_symbol(std::string const& lib_name, std::string const& short_name)
{
    load_lib(lib_name);

    std::string full_name = find_full_name(lib_name, short_name); 

    if(!full_name.empty())
        return ::get_symbol(libs_[lib_name],full_name);

    return nullptr;
}

std::string lib_loader_t::find_full_name(std::string const& lib_name, std::string const& short_name)
{
    auto it = collections_.find(lib_name);

    if (it != collections_.end()) 
    {
         auto col = it->second;
         for (auto it = col.begin();it!=col.end();++it)
         {
             if(it->find(short_name)!=std::string::npos)
                 return *it;
         }
    }

    return "";
}

void lib_loader_t::load_lib(std::string const& lib_name)
{
        auto it = collections_.find(lib_name);
    
        if (it == collections_.end()) // let's try to reload it 
        {
              lib_descriptor_t lib  = ::load_library(lib_name + "." + lib_extension());
              if (lib != 0)
              {   
                  libs_[lib_name]  = lib;
                  collections_[lib_name]= sym_enum(lib);
              }
        }


}

void lib_loader_t::dispose()
{
    for (auto it = libs_.begin(); it != libs_.end(); ++it)
        unload_library(it->second);

    libs_.clear();
    collections_.clear();
}

//void lib_loader_t::on_lib(fs::path const& lib_path) const // sorry, need in collection func
//{
//    lib_descriptor_t lib  = load_library(lib_path.string());
//    sym_descriptor_t symb = get_symbol  (lib, LIB_COLLECTION_FUNC_NAME_STR);
//
//    if (symb != 0)
//    {
//        libs_.push_back(lib);
//
//        auto extract_collection_function = reinterpret_cast<empty_f(*)()>(symb);
//        auto func_collection             = reinterpret_cast<func_collection_t const* (*)()>(extract_collection_function());
//
//        string lib_name = extract_libname(lib_path);
//        collections_[lib_name] = func_collection();
//    }
//}

/////////////////////////////////

lib_loader_t& lib_loader()
{
    static lib_loader_t loader;
    return loader;
}
