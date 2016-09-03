#include "scn_lights.h"

using namespace cg;

struct value_getter
{
    typedef std::vector<std::string> values_t;

    value_getter(std::string const& line)
    {
        boost::split(values_, line, boost::is_any_of(" \t"), boost::token_compress_on);
    }

    template <class T>
    T get(size_t index)
    {
        return boost::lexical_cast<T>(values_[index]);
    }

    bool valid()
    {
        return values_.size()>0;
    }

    values_t::size_type get_size()
    {
        return values_.size();
    }

private:
    values_t       values_;
};


void fill_navids(std::string file,   point_3f const& offset)
{
    //     if (!boost::filesystem::is_regular_file(file))
    //         LogWarn("No lights for airport found: " << file.string());


    std::ifstream ifs(file);

    //for (auto it = cur_lamps.begin(); it != cur_lamps.end(); ++it)
    //    parent->removeChild(it->get());
    //cur_lamps.clear();

    //navaid_group_node_ptr navid_node_ptr = nullptr;

    bool group_ready = false;

    while (ifs.good())
    {
        char buf[0x400] = {};
        ifs.getline(buf, 0x400);

        std::string line = buf;
        boost::trim(line);

        // skip comments

        if (line.size()==0) continue;

        if (boost::starts_with(line, "//"))
        {
            boost::erase_all(line," ");
            if (boost::starts_with(line, "//#"))
            {
                boost::trim_if(line, boost::is_any_of("/#"));
                
                //navid_node_ptr.release();
                //navid_node_ptr = new osgSim::LightPointNode;// new osg::Group();
                //navid_node_ptr->setName(line);
                //

                group_ready = true;
            }
            else
                continue;
        }

        value_getter items(line);

        if (items.get<std::string>(0) == "FireLine:" || items.get<std::string>(0) == "FireLineHa:")
        {
            size_t ofs = items.get<std::string>(0) == "FireLineHa:" ? 3 : 0;

            point_3f  pos (items.get<float>(ofs + 2), items.get<float>(ofs + 4), items.get<float>(ofs + 3));
            point_3f  dir (polar_point_2f(1, items.get<float>(ofs + 5)));
            float     len   = items.get<float>(ofs + 6);
            float     step  = items.get<float>(ofs + 7);
            point_4f  clr (items.get<float>(ofs + 8), items.get<float>(ofs + 9), items.get<float>(ofs + 10),1.0f); 

            size_t count = 0;
            if (!cg::eq_zero(step) && !cg::eq_zero(len))
            {
                count = size_t(cg::ceil(len / step));

                if (step > 2)
                    ++count;
            }

            for (size_t i = 0; i < count; ++i)
            {
                point_3f p = pos + dir * step * i  + offset;

                //victory::navaid_group_node::LightData lamp = {p, clr, .1,40000,/*.01f, 4000.f,*/ cg::range_2f(), cg::range_2f(), 1, 0, 0};
                //navid_group->AddLight(lamp);

                //osgSim::LightPoint pnt;

                //pnt._position.set(p.x(),p.y(),p.z());
                //pnt._color = clr;
                //if (!usePointSprites)
                //    pnt._radius = 0.3f;
                //else
                //    pnt._radius = 0.6f;

                //navid_node_ptr->addLightPoint(pnt);

            }
        }

        if(/*navid_node_ptr &&*/ group_ready)
        {
            //cur_lamps.push_back(navid_node_ptr);
            //parent->addChild(navid_node_ptr);
            group_ready = false;
        }

    }


    //cur_lamps.push_back(navid_group);
    //parent->add(navid_group);
}
