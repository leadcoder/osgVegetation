#pragma once
#include "ov_BillboardMultiLayerEffect.h"
#include "ov_MeshLayerGenerator.h"

namespace osgVegetation
{
	class LayerGenerator
	{
	public:
		LayerGenerator(std::vector<osg::ref_ptr<ILayerConfig>> layers)
		{
			//Sort layers
			std::vector<MeshLayerConfig> mesh_layers;
			std::vector<BillboardLayerConfig> billboard_layers;
			for (size_t i = 0; i < layers.size(); i++)
			{
				if (MeshLayerConfig* mesh_layer = dynamic_cast<MeshLayerConfig*>(layers[i].get()))
					mesh_layers.push_back(*mesh_layer);
				if (BillboardLayerConfig* bb_layer = dynamic_cast<BillboardLayerConfig*>(layers[i].get()))
					billboard_layers.push_back(*bb_layer);
			}
			m_BBEffect = new BillboardMultiLayerEffect(billboard_layers);
			m_MeshGenerator = new MeshMultiLayerGenerator(mesh_layers);
		}

		osg::ref_ptr<osg::Group> CreateVegetationNode(osg::ref_ptr<osg::Node> terrain_geometry)
		{
			osg::ref_ptr<osg::Group> root = new osg::Group();
			osg::ref_ptr<BillboardMultiLayerEffect> bb_layers = m_BBEffect->createInstance(terrain_geometry);
			root->addChild(bb_layers);
			osg::ref_ptr<osg::Group> mesh_layers = m_MeshGenerator->CreateMeshNode(terrain_geometry);
			root->addChild(mesh_layers);
			return root;
		}
	private:
		osg::ref_ptr<BillboardMultiLayerEffect> m_BBEffect;
		MeshMultiLayerGenerator* m_MeshGenerator;
	};
}