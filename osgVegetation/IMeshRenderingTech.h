#pragma once
#include "Common.h"
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/Node>
#include "MeshObject.h"

namespace osgVegetation
{
	class IMeshRenderingTech : public osg::Referenced
	{
	public:
		virtual ~IMeshRenderingTech(){}
		virtual osg::Node* create(const MeshVegetationObjectVector &trees, const std::string &mesh_name, const osg::BoundingBoxd &bb) = 0;
		virtual osg::StateSet* getStateSet() const = 0;
	};
}
