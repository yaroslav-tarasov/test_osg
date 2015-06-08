#pragma once

inline osg::Vec3 to_osg_vector3( cg::point_3 const& v )
{
    return osg::Vec3(v.x, v.y, v.z);
}

inline cg::point_3 from_osg_vector3( osg::Vec3 const& v )
{
    return cg::point_3(v.x(), v.y(), v.z());
}

inline osg::Quat to_osg_quat( cg::quaternion const& q )
{
    return osg::Quat(q.get_v().x, q.get_v().y, q.get_v().z, q.get_w());
}

inline cg::quaternion from_osg_quat( osg::Quat const& q )
{   
    return cg::quaternion ( q.w(), from_osg_vector3(q.asVec3()) );
}

inline osg::Matrix3 to_osg_matrix( cg::matrix_3 mm )
{                 
    osg::Matrix3 m (mm(0, 0), mm(0, 1), mm(0, 2), 
        mm(1, 0), mm(1, 1), mm(1, 2),
        mm(2, 0), mm(2, 1), mm(2, 2));
    return m;
}

inline cg::matrix_3 from_osg_matrix( osg::Matrix3 const& m )
{
    cg::matrix_3 mm;
    mm(0,0) = m(0,0),  mm(0,1) = m(0,1), mm(0,2) = m(0,2);
    mm(1,0) = m(1,0),  mm(1,1) = m(1,1), mm(1,2) = m(1,2);
    mm(2,0) = m(2,0),  mm(2,1) = m(2,1), mm(2,2) = m(2,2);

    return mm;
}

inline cg::matrix_4 from_osg_matrix( osg::Matrix const& m )
{
    cg::matrix_4 mm;
    mm(0,0) = m(0,0),  mm(0,1) = m(0,1), mm(0,2) = m(0,2), mm(0,3) = m(0,3);
    mm(1,0) = m(1,0),  mm(1,1) = m(1,1), mm(1,2) = m(1,2), mm(1,3) = m(1,3);
    mm(2,0) = m(2,0),  mm(2,1) = m(2,1), mm(2,2) = m(2,2), mm(2,3) = m(2,3);
    mm(3,0) = m(3,0),  mm(3,1) = m(3,1), mm(3,2) = m(3,2), mm(3,3) = m(3,3);

    return mm;
}

inline cg::transform_4 from_osg_transform( const osg::Matrix& m )
{
	// FIXME  Мать, мать, мать, ну чего все так не прямо
	return cg::transform_4(cg::as_translation(from_osg_vector3(m.getTrans())), cg::rotation_3(from_osg_quat(m.getRotate()).cpr()));
}

inline osg::Matrix to_osg_transform( const cg::transform_4& tr )
{
    osg::Matrix m;
    cg::transform_4::translation_t t = tr.translation();
    cg::rotation_3 r = tr.rotation();   
    m.setTrans(to_osg_vector3(t));
    m.setRotate(to_osg_quat(r.cpr()));
    return m;
}

inline osg::Matrix get_relative_transform(osg::Node* root, osg::Node* node, osg::Node* rel = NULL)
{
    osg::Matrix tr;
    osg::Node* n = node;
    while(/*n->position().is_local() &&*/ n != rel && 0 != n->getNumParents() && n != root)
    {
        if(n->asTransform())
        if(n->asTransform()->asMatrixTransform())
        tr = n->asTransform()->asMatrixTransform()->getMatrix() * tr;
        n = n->getParent(0);
    }

    if (n == rel || rel == NULL)
        return tr;

    osg::Matrix tr_rel;
    n = rel;
    while(/*n->position().is_local()*/0 != n->getNumParents() && n != root)
    {
        if(n->asTransform())
        if(n->asTransform()->asMatrixTransform())
        tr_rel = n->asTransform()->asMatrixTransform()->getMatrix() * tr_rel;
        n = n->getParent(0);
    }

    return (osg::Matrix::inverse(tr_rel)) * tr;
}

