#pragma once

#include "ov_Utils.h"
#include <osg/PositionAttitudeTransform>
#include <osg/Texture2D>
#include <osgTerrain/TerrainTile>
#include <osg/ShapeDrawable>

namespace osgVegetation
{
	class TerrainHelper
	{
	public:
		static osg::Node* CreateTerrainNodeFromTerrainTile(osgTerrain::TerrainTile* tile)
		{
			osg::PositionAttitudeTransform* ret_node = NULL;
			osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
			if (layer)
			{
				osg::HeightField* hf = layer->getHeightField();
				if (hf)
				{
					osg::Geometry* hf_geom = _CreateGeometryFromHeightField(hf);
					ret_node = new osg::PositionAttitudeTransform();
					ret_node->setPosition(hf->getOrigin());
					ret_node->addChild(hf_geom);
				
					//Add color texture
					osgTerrain::Layer* colorLayer = tile->getColorLayer(0);
					if (colorLayer)
					{
						osg::Image* image = colorLayer->getImage();
						osg::Texture2D* texture = new osg::Texture2D(image);
						ret_node->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
					}

					//Add land cover texture
					osgTerrain::Layer* landCoverLayer = tile->getColorLayer(1);
					if (landCoverLayer)
					{
						osg::Image* image = landCoverLayer->getImage();
						osg::Texture2D* texture = new osg::Texture2D(image);
						ret_node->getOrCreateStateSet()->setTextureAttributeAndModes(1, texture, osg::StateAttribute::ON);
					}
				}
			}
			return ret_node;
		}
	private:
		static osg::Geometry* _CreateGeometryFromHeightField(osg::HeightField* hf)
		{
			unsigned int numColumns = hf->getNumColumns();
			unsigned int numRows = hf->getNumRows();
			float columnCoordDelta = hf->getXInterval();
			float rowCoordDelta = hf->getYInterval();

			osg::Geometry* geometry = new osg::Geometry;

			osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
			osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
			osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));
			color[0].set(255, 255, 255, 255);
			float rowTexDelta = 1.0f / (float)(numRows - 1);
			float columnTexDelta = 1.0f / (float)(numColumns - 1);
			osg::Vec3 local_origin(0, 0, 0);

			osg::Vec3 pos(local_origin.x(), local_origin.y(), local_origin.z());
			osg::Vec2 tex(0.0f, 0.0f);
			int vi = 0;
			for (unsigned int r = 0; r < numRows; ++r)
			{
				pos.x() = local_origin.x();
				tex.x() = 0.0f;
				for (unsigned int c = 0; c < numColumns; ++c)
				{
					float h = hf->getHeight(c, r);
					v[vi].set(pos.x(), pos.y(), h);
					t[vi].set(tex.x(), tex.y());
					pos.x() += columnCoordDelta;
					tex.x() += columnTexDelta;
					++vi;
				}
				pos.y() += rowCoordDelta;
				tex.y() += rowTexDelta;
			}

			geometry->setVertexArray(&v);
			geometry->setTexCoordArray(0, &t);
			geometry->setColorArray(&color, osg::Array::BIND_OVERALL);

			osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 2 * 3 * (numColumns*numRows)));
			geometry->addPrimitiveSet(&drawElements);
			int ei = 0;
			for (unsigned int r = 0; r < numRows - 1; ++r)
			{
				for (unsigned int c = 0; c < numColumns - 1; ++c)
				{
					// Try to imitate how GeometryTechnique::generateGeometry optimize 
					// which way to put the diagonal by choosing to
					// place it between the two corners that have the least curvature
					// relative to each other.
					// Due to how normals are calculated we don't get exact match...fix this by using same normal calulations

					osg::Vec3 n00 = hf->getNormal(c, r);
					osg::Vec3 n01 = hf->getNormal(c, r + 1);
					osg::Vec3 n10 = hf->getNormal(c + 1, r);
					osg::Vec3 n11 = hf->getNormal(c + 1, r + 1);
					float dot_00_11 = n00 * n11;
					float dot_01_10 = n01 * n10;
					if (dot_00_11 > dot_01_10)
					{
						drawElements[ei++] = (r)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c + 1;

						drawElements[ei++] = (r + 1)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c;
					}
					else
					{
						drawElements[ei++] = (r)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;

						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;
					}
				}
			}
			geometry->setUseDisplayList(false);
			return geometry;
		}
	};
}
