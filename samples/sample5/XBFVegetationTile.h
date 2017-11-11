#pragma once

#include <osg/PositionAttitudeTransform>
#include "XBFInstanceGenerator.h"
#define XFB_SLOT 7

class XBFVegetationTile : public osg::PositionAttitudeTransform
{
public:
	XBFVegetationTile(osgTerrain::TerrainTile* tile, const XBFVegetationData& data)
	{
		if (data.MeshLODs.size() == 0)
			throw std::runtime_error("VegetationGroup::VegetationGroup No Mesh LOD added");
		if (data.MeshLODs.size() != 2)
			throw std::runtime_error("VegetationGroup::VegetationGroup VegetationGroup only support two mesh LOD for now");

		osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
		if (layer)
		{
			osg::HeightField* hf = layer->getHeightField();
			if (hf)
			{
				setPosition(hf->getOrigin());
				const osg::BoundingBox bbox = _GetBounds(hf);
				std::vector<XBFInstance*> instances;
				const int maxNumInstances = hf->getNumColumns()*hf->getNumRows() * 2 * (data.Density*data.Density);
				for (size_t i = 0; i < data.MeshLODs.size(); i++)
				{
					osg::Node* instancedModel = osgDB::readNodeFile(data.MeshLODs[i].Mesh);
					if (!instancedModel)
						throw std::runtime_error(std::string("VegetationGroup::VegetationGroup - Clould not load model:" + data.MeshLODs[i].Mesh).c_str());
					XBFInstance* instance = new XBFInstance(maxNumInstances, XFB_SLOT, instancedModel, bbox);
					instances.push_back(instance);
					addChild(instance);
				}
				osg::Geometry* geom = new XBFInstanceGenerator(instances, *tile, data);
				osg::Geode* genGeode = new osg::Geode();
				genGeode->addDrawable(geom);
				addChild(genGeode);
			}
		}
	}
private:
	osg::BoundingBox _GetBounds(osg::HeightField* hf)
	{
		osg::BoundingBox bbox;
		unsigned int numColumns = hf->getNumColumns();
		unsigned int numRows = hf->getNumRows();
		float columnCoordDelta = hf->getXInterval();
		float rowCoordDelta = hf->getYInterval();

		osg::Vec3 pos(0, 0, 0);
		for (unsigned int r = 0; r < numRows; ++r)
		{
			pos.x() = 0;
			for (unsigned int c = 0; c < numColumns; ++c)
			{
				float h = hf->getHeight(c, r);
				bbox.expandBy(osg::Vec3(pos.x(), pos.y(), h));
				pos.x() += columnCoordDelta;
			}
			pos.y() += rowCoordDelta;
		}
		return bbox;
	}
};