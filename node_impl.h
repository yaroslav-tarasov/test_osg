#pragma once

namespace nodes_management
{

class node_impl : public node_control
{
public:
    node_impl( osg::Node* n )
        :node_(n)
    {
        node_->getUserValue("id",node_id_);
    }
    
    virtual ~node_impl(){childs_callbacks_.clear();}

public:
    // void save( binary::output_stream& stream ) const;
    virtual void pre_update (double time);
    virtual void post_update(double time);

public:
    virtual void extrapolated_position_reseted();


//  node_control
private:
    void play_animation  (std::string const& seq, double len, double from, double size) override;
    void set_texture     (std::string const& texture) override;
    void set_visibility  (bool visible) override;
    void set_position    (node_position const& pos) override;

//  node_info
    node_position /*const&*/    position        () /*const*/ override;
    uint32_t                    node_id         () const;
    cg::sphere_3                get_bound       ()         override;
    std::string const&          name            ()   const override;
    node_info_ptr               root_node       ()   const override;
    cg::geo_point_3             get_global_pos  ()   const override;
    cg::quaternion              get_global_orien()   const override;
    cg::transform_4             get_root_transform() const override;

public:
//  node_impl
    osg::Node*                  as_osg_node() {return node_.get();}
private:
    uint32_t    node_id_;

    optional<double>                               time_;

    osg::ref_ptr<osg::Node>                       node_;
    node_position                                 position_;
    node_position                                 extrapolated_position_;
    std::vector<osg::ref_ptr<osg::NodeCallback>>  childs_callbacks_;
    mutable std::string                           name_;
};

typedef polymorph_ptr<node_impl> node_impl_ptr;

}