

#include <stdafx.h>
#include "av/precompiled.h"

#include "NodeTracker.h"

using namespace utils;


void NodeTracker::setTrackNode(osg::Node* node)
{
	if (!node)
	{
		OSG_NOTICE<<"NodeTrackerCallback::setTrackNode(Node*):  Unable to set tracked node due to null Node*"<<std::endl;
		return;
	}


	osg::NodePathList parentNodePaths = node->getParentalNodePaths(/*avScene::GetScene()*/);

	if (!parentNodePaths.empty())
	{
		OSG_INFO<<"NodeTrackerCallback::setTrackNode(Node*): Path set"<<std::endl;
		setTrackNodePath(parentNodePaths[0]);
	}
	else
	{
		OSG_NOTICE<<"NodeTrackerCallback::setTrackNode(Node*): Unable to set tracked node due to empty parental path."<<std::endl;
	}
}

osg::Node* NodeTracker::getTrackNode()
{
	osg::NodePath nodePath;
	if (_trackNodePath.getNodePath(nodePath)) return nodePath.back();
	else return 0;
}

const osg::Node* NodeTracker::getTrackNode() const
{
	osg::NodePath nodePath;
	if (_trackNodePath.getNodePath(nodePath)) return nodePath.back();
	else return 0;
}


const osg::Matrix&  NodeTracker::update()
{
	osg::NodePath nodePath;
	if (_trackNodePath.getNodePath(nodePath))
	{
		_matrix = computeLocalToWorld(nodePath);

		osg::Matrixd trMatrix;
		trMatrix.setTrans(-_matrix.getTrans());
		osg::Matrixd trMatrix2;
		trMatrix2.setTrans(_matrix.getTrans());
		osg::Matrixd rotateMatrix;
		rotateMatrix.setRotate(_matrix.getRotate());

		_rot_matrix = trMatrix*rotateMatrix*trMatrix2;
	}

	return _matrix;
}

