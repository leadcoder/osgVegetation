#pragma once
#include "Common.h"
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "IMeshRenderingTech.h"
#include "MeshData.h"
namespace osgVegetation
{

	class osgvExport MRTShaderInstancing :  public IMeshRenderingTech
	{
	public:
		MRTShaderInstancing(MeshData &data);
		osg::Node* create(const MeshVegetationObjectVector &trees, const std::string &mesh_name, const osg::BoundingBoxd &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}
	protected:
		osg::StateSet* _createStateSet(MeshData &data);
		osg::StateSet* m_StateSet; 
		std::map<std::string, osg::ref_ptr<osg::Node>  > m_MeshNodeMap;
		std::vector<osg::Geometry*> m_Geometries;
	};
}