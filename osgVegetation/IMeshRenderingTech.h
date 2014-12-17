#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/Node>
#include <math.h>
#include "MeshObject.h"
#include "MeshLayer.h"

namespace osgVegetation
{
	class IMeshRenderingTech : public osg::Referenced
	{
	public:
		IMeshRenderingTech(){}
		virtual ~IMeshRenderingTech(){}
		virtual osg::Node* create(const MeshVegetationObjectVector &trees, const std::string &mesh_name, const osg::BoundingBox &bb) = 0;
		virtual osg::StateSet* createStateSet(MeshLayerVector &layers) = 0;
	protected:
	};
}