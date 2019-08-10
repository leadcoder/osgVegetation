#pragma once
#include "ov_BillboardLayer.h"
#include "ov_BillboardLayerStateSet.h"

namespace osgVegetation
{
	/*class BillboardMultiLayerEffectConfig
	{
	public:
		BillboardMultiLayerEffectConfig(const std::vector<BillboardLayer> &layers, int tex_unit) : Layers(layers),
			TextureUnit(tex_unit)
		{

		}
		std::vector<BillboardLayer> Layers;
		int TextureUnit;
	};*/

	class BillboardMultiLayerEffect : public osg::Group
	{
	public:
		BillboardMultiLayerEffect(const std::vector<BillboardLayer> &layers/*, int tex_unit*/)
		{
			for (size_t i = 0; i < layers.size(); i++)
			{
				osg::ref_ptr<BillboardLayerEffect> layer = new BillboardLayerEffect(layers[i]/*, tex_unit*/);
				if(layers[i].Type == BillboardLayer::BLT_GRASS)
					layer->setNodeMask(0x1);
				else
					layer->setNodeMask(0x1 | 0x2);

				//layer->setNodeMask(0x1);
				//layer_node->setNodeMask(0x1 | m_Config.CastShadowTraversalMask);
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