#pragma once

namespace Database
{
    struct fpl_wrap 
    {
        fpl_wrap(const std::string& name)
        {
            fpl_.push_back(cfg().path.data + "/models/" + name + "/");
            fpl_.push_back(cfg().path.data + "/areas/" + name + "/");
            fpl_.push_back(cfg().path.data + "/areas/misc/" + name + "/");
        };
        
        const osgDB::FilePathList& get_file_list() const {return fpl_;}
    private:
        osgDB::FilePathList fpl_;
    };

} // ns Database

