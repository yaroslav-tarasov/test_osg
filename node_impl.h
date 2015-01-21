#pragma once

namespace nodes_management
{

class node_impl : public node_control
{
public:
    node_impl( osg::Node* n )
        :node_(n)
    {}

    void play_animation  (std::string const& seq, double len, double from, double size) override;
    void set_texture     (std::string const& texture) override;
    void set_visibility  (bool visible) override;

    std::string const&  name  () const override;
private:
    osg::ref_ptr<osg::Node> node_;
};

typedef polymorph_ptr<node_impl> node_impl_ptr;

}