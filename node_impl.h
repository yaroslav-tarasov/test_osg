#pragma once

namespace nodes_management
{

class node_impl : public node_control
{
public:
    node_impl( osg::Node* n )
        :node_(n)
    {}

//  node_control
    void play_animation  (std::string const& seq, double len, double from, double size) override;
    void set_texture     (std::string const& texture) override;
    void set_visibility  (bool visible) override;

//  node_info
    cg::sphere_3        get_bound();
    std::string const&  name  () const override;

//  node_impl
    osg::Node*          as_osg_node() {return node_.get();}
private:
    osg::ref_ptr<osg::Node> node_;
};

typedef polymorph_ptr<node_impl> node_impl_ptr;

}