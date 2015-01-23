#pragma once


inline osg::Matrix get_relative_transform(osg::Node* root, osg::Node* node, osg::Node* rel = NULL)
{
    osg::Matrix tr;
    osg::Node* n = node;
    while(/*n->position().is_local() &&*/ n != rel && 0 != n->getNumParents() && n != root)
    {
        tr = n->asTransform()->asMatrixTransform()->getMatrix() * tr;
        n = n->getParent(0);
    }

    if (n == rel || rel == NULL)
        return tr;

    osg::Matrix tr_rel;
    n = rel;
    while(/*n->position().is_local()*/0 != n->getNumParents() && n != root)
    {                  
        tr_rel = n->asTransform()->asMatrixTransform()->getMatrix() * tr_rel;
        n = n->getParent(0);
    }

    return (osg::Matrix::inverse(tr_rel)) * tr;
}

