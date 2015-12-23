#pragma once    

template<typename T>
std::ostream &operator <<(std::ostream &os, const std::vector<T> &v) {
    using namespace std;
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
    return os;
}

template<typename T>
std::ostream &operator <<(std::ostream &os, const std::vector<std::vector<T>> &v) {
    using namespace std;

    //NOTE: for some reason std::copy doesn't work here, so I use manual loop
    //copy(v.begin(), v.end(), ostream_iterator<std::vector<T>>(os, "\n"));

    for(size_t i = 0 ; i < v.size(); ++i)
        os << v[i] << "  ";
    return os;
}

class heilVisitor : public osg::NodeVisitor
    {  

    public:
        heilVisitor(std::ofstream& filelogic, const cg::point_3& offset)  
            :_level(0)
            , osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
            , root_visited(false)
            , lod_visited (false)
			, got_lod(false)
            , ostream_nodes(filelogic)
            , damned_offset(offset)
            , damned_offset_added(false)
        {
            
            using namespace binary;
            // fill default
            //write(ostream_nodes, 1);
            auto d = wrap(binary::size_type(1));
            ostream_nodes.write(raw_ptr(d), size(d));//1 transform before root
        }

        std::string spaces()
        { return std::string(_level*2, ' '); }
        
        size_t num_children(osg::Group& node)
        {
            size_t num = 0;

            for (unsigned i = 0; i < node.getNumChildren(); i++) //lod level
            {
                if (node.getChild(i)->asTransform() || node.getChild(i)->asGroup())
                    ++num;
            }
            return num;
        }

        void apply( osg::Node& node )
        {   
            using namespace binary;
            std::cout << spaces() << node.libraryName() << "::" << node.className() << " : " << node.getName() << std::endl;
            std::string node_name = boost::to_lower_copy(node.getName());
            std::string name_cut = node_name.substr(0, node_name.find("_lod"));
            std::string name_first_part = name_cut.substr(0, name_cut.find("_")); 
            cg::point_3 offset;

            if(node_name == "root")
            {
               root_visited = true;
               offset = damned_offset;
            }
            FIXME("XXX")
            
            //if(boost::starts_with(node_name,"lod") && name_cut == name_first_part && name_first_part != "lod0")
            //   return;
            
            if(boost::starts_with(node_name,"lod") && lod_visited)
                return;
            else
                if(boost::starts_with(node_name,"lod0"))
                {
                    lod_visited = true;
                }
				else if(boost::starts_with(node_name,"lod"))
                {
                    got_lod = true;
                    if(node.asGroup())
                    {
                        for(unsigned i = 0; i<node.asGroup()->getNumChildren();++i)
                        {
                            lods_.push_back(node.asGroup()->getChild(i)->getName());
                        }
                    }
                }


            if (root_visited && (node.asTransform() || node.asGroup() ))
            {

                model_structure::node_data new_node;
            
                binary::size_type  children_count = got_lod?1:(node.asGroup()?num_children(*node.asGroup()):0);
                
                if(node_name == "root")
                    children_count++;  // For damned_offset
				
                if(got_lod)
                {
                    got_lod=false;
                }
                
                bool mt = node.asTransform();
				
                new_node.pos   = from_osg_vector3(mt?node.asTransform()->asMatrixTransform()->getMatrix().getTrans():osg::Vec3(0,0,0)) + offset;
                new_node.orien = from_osg_quat(mt?node.asTransform()->asMatrixTransform()->getMatrix().getRotate():osg::Quat());
                //const osg::BoundingSphere& bs = node.getBound();
                //new_node.bound = cg::sphere_3(cg::sphere_3::point_t(bs.center().x(),bs.center().y(),bs.center().z()),bs.radius());
                
                osg::ComputeBoundsVisitor cbvs;
                node.accept( cbvs );
                const osg::BoundingBox bb = cbvs.getBoundingBox();
                new_node.bound = cg::rectangle_3(cg::rectangle_3::point_t(bb.xMin(),bb.yMin(),bb.zMin()),cg::rectangle_3::point_t(bb.xMax(),bb.yMax(),bb.zMax()));


                if (lod_visited && node_name.find("_lod")!= std::string::npos)
                    std::for_each(lods_.begin(),lods_.end(),[&new_node,&name_cut](const std::string name){new_node.victory_nodes.push_back(name_cut + "_" + name);});
                else
                    new_node.victory_nodes.push_back(name_cut);

                new_node.name = name_cut; // логическое имя. Общее для все лодов

                {
                    auto d = wrap(new_node);
                    ostream_nodes.write(raw_ptr(d), size(d));
                }

                {
                    const binary::size_type  cc = children_count;
                    auto d = wrap(cc);// print root
                    ostream_nodes.write(raw_ptr(d), size(d));
                }

                print_node(new_node, children_count);

                
                if(!damned_offset_added)
                {
                    model_structure::node_data do_node;
                    do_node.pos   = from_osg_vector3(mt?node.asTransform()->asMatrixTransform()->getMatrix().getTrans():osg::Vec3(0,0,0)) + offset;
                    do_node.name =  "damned_offset";

                    damned_offset_added = true;

                    {
                        auto d = wrap(do_node);
                        ostream_nodes.write(raw_ptr(d), size(d));
                    }

                    {
                        auto d = wrap(0);// print root
                        ostream_nodes.write(raw_ptr(d), size(d));
                    }

                    print_node(do_node,0);
                }

            }

            _level++;
            traverse( node );
            _level--;
        }


        void apply( osg::Geode& geode )
        {
            std::cout << spaces() << geode.libraryName() << "::" << geode.className() << " : " << geode.getName() << std::endl;
            
            std::stringstream cstr;

            //cstr << std::setprecision(8) 
            //    << "physics x:  "         << cur_pos.pos.x
            //    << "\n" ;

            //OutputDebugString(cstr.str().c_str());

            _level++;
            for ( unsigned int i=0; i<geode.getNumDrawables(); ++i )
            {
                osg::Drawable* drawable = geode.getDrawable(i);
                std::cout << spaces() << drawable->libraryName() << "::" << drawable->className() << " : " << geode.getName() << std::endl;
            }

            traverse( geode );
            _level--;
        }

        void inline print_node( model_structure::node_data &new_node, const binary::size_type children_count ) 
        {
            std::stringstream cstr;

            cstr << std::setprecision(8) 
                << "--------------------------------------- \n"
                << spaces() << "   Node /Name=   "          << new_node.name << "\n"
                << spaces() << "   Translate =   ("         << new_node.pos.x  << " ," << new_node.pos.y << " ,"  << new_node.pos.z << " )" << "\n"
                << spaces() << "   Rotate    =   ("         << new_node.orien.get_course()  << " ," << new_node.orien.get_pitch() << " ,"  << new_node.orien.get_roll() << " )" << "\n"
                << spaces() << "   Logic children number: " << children_count  
                << spaces() << "   visual nodes: "           << new_node.victory_nodes  << "\n";

            OutputDebugStringA(cstr.str().c_str());
        }

    protected:
        unsigned int             _level;
        std::ofstream&           ostream_nodes;
        bool                     root_visited;
        bool                     lod_visited;
		bool                     got_lod;
        std::vector<std::string> lods_;
        cg::point_3              damned_offset;
        bool                     damned_offset_added;   
    };