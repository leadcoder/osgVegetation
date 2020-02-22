#pragma once
#include "ov_BillboardLayerConfig.h"
#include "ov_BillboardLayerStateSet.h"

namespace osgVegetation
{
	class BillboardMultiLayerEffect : public osg::Group
	{
	public:
		BillboardMultiLayerEffect(const std::vector<BillboardLayerConfig> &layers)
		{
			for (size_t i = 0; i < layers.size(); i++)
			{
				osg::ref_ptr<BillboardLayerEffect> layer = new BillboardLayerEffect(layers[i]);
				addChild(layer);
			}
		}

		BillboardMultiLayerEffect(const BillboardMultiLayerEffect& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) : osg::Group(rhs, copyop)
		{

		}

		virtual Object* cloneType() const { return new Group(); }
		virtual Object* clone(const osg::CopyOp& copyop) const { return new BillboardMultiLayerEffect(*this, copyop); }

		osg::ref_ptr<BillboardMultiLayerEffect> createInstance(osg::ref_ptr<osg::Node> terrain_geometry) const
		{
			osg::ref_ptr<BillboardMultiLayerEffect>  effect = dynamic_cast<BillboardMultiLayerEffect*>(clone(osg::CopyOp::DEEP_COPY_NODES));
			effect->insertTerrain(terrain_geometry);
			return effect;
		}
	
		void insertTerrain(osg::ref_ptr<osg::Node> terrain_geometry)
		{
			for (unsigned int i = 0; i < getNumChildren(); i++)
			{
				getChild(i)->asGroup()->addChild(terrain_geometry);
			}
		}
	};
}