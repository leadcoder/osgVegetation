#pragma once

#include "ov_Utils.h"
#include <osg/MatrixTransform>
#include <osg/NodeVisitor>
#include <osg/Texture2D>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/Locator>
#include <osg/ShapeDrawable>
#include <osgUtil/Optimizer>

namespace osgVegetation
{
	class TerrainHelper
	{
	private:
		class TerrainTileVisitor : public osg::NodeVisitor
		{
		public:
			TerrainTileVisitor(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Node& node)
			{
				osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
				if (tile)
				{
					TerrainTiles.push_back(tile);
				}
				else
				{
					traverse(node);
				}
			}
			std::vector<osgTerrain::TerrainTile*> TerrainTiles;
		};
	public:
		static std::vector<osgTerrain::TerrainTile*> GetTerrainTiles(osg::Node* node)
		{
			TerrainTileVisitor visitor;
			node->accept(visitor);
			return visitor.TerrainTiles;
		}

		static osg::Node* CreateTerrainNodeFromTerrainTile(osgTerrain::TerrainTile* tile)
		{
			osg::MatrixTransform* ret_node = NULL;
			osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
			if (layer)
			{
				osg::HeightField* hf = layer->getHeightField();
				if (hf)
				{
					osgTerrain::Locator* locator = tile->getLocator();
					osg::EllipsoidModel* em = locator->getEllipsoidModel();
					
					ret_node = new osg::MatrixTransform();
					
					osg::Matrixd matrix = locator->getTransform();
					osg::Vec3d center = osg::Vec3d(0.5, 0.5, 0.0) * matrix;
					osg::Matrixd localToWorldTransform;

					if (locator->getCoordinateSystemType() == osgTerrain::Locator::GEOCENTRIC)
					{
						em->computeLocalToWorldTransformFromLatLongHeight(center.y(), center.x(), center.z(), localToWorldTransform);
					}
					else
					{
						localToWorldTransform = locator->getTransform();
					}
#if 1
					//unscale to allow VDSM-shadowing...and avoid inverse scaling in geometry shader
					osg::Vec3d scale = localToWorldTransform.getScale();
					localToWorldTransform.preMultScale(osg::Vec3d(1.0/ scale.x(), 1.0 / scale.y(), 1.0 / scale.z()));
					ret_node->setMatrix(localToWorldTransform);
#endif
					osg::Matrixd worldToLocalTransform;
					worldToLocalTransform.invert(ret_node->getMatrix());
					osg::Geometry* hf_geom = _CreateGeometryFromHeightField(hf, locator, worldToLocalTransform);

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
		static osg::Geometry* _CreateGeometryFromHeightField(const osg::HeightField* hf, const osgTerrain::Locator* locator ,const osg::Matrixd& worldToLocalTransform)
		{
			const unsigned int numColumns = hf->getNumColumns();
			const unsigned int numRows = hf->getNumRows();
			//float columnCoordDelta = hf->getXInterval();
			//float rowCoordDelta = hf->getYInterval();

			const float rowCoordDelta = 1.0f / (float)(numRows - 1);
			const float columnCoordDelta = 1.0f / (float)(numColumns - 1);
			const float rowTexDelta = 1.0f / (float)(numRows - 1);
			const float columnTexDelta = 1.0f / (float)(numColumns - 1);

			osg::Geometry* geometry = new osg::Geometry;

			osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
			osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
			osg::Vec3Array& n = *(new osg::Vec3Array(numColumns*numRows));
			osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));
			color[0].set(255, 255, 255, 255);
			osg::Vec3d local_origin(0, 0, 0);
			osg::Vec3d local_pos = local_origin;
			osg::Vec2 tex(0.0f, 0.0f);
			int vi = 0;

			for (unsigned int r = 0; r < numRows; ++r)
			{
				local_pos.x() = local_origin.x();
				tex.x() = 0.0f;
				for (unsigned int c = 0; c < numColumns; ++c)
				{
					local_pos.z() = static_cast<double>(hf->getHeight(c, r));

					osg::Vec3d world_pos;
					locator->convertLocalToModel(local_pos, world_pos);
					osg::Vec3d new_local_pos = world_pos * worldToLocalTransform;

					v[vi].set(new_local_pos.x(), new_local_pos.y(), new_local_pos.z());
					t[vi].set(tex.x(), tex.y());
					
					local_pos.x() += columnCoordDelta;
					tex.x() += columnTexDelta;
					++vi;
				}
				local_pos.y() += rowCoordDelta;
				tex.y() += rowTexDelta;
			}

			for (unsigned int r = 0; r < numRows; ++r)
			{
				for (unsigned int c = 0; c < numColumns; ++c)
				{
					int center = (r)*numColumns + c;
					osg::Vec3 center_p = v[center];
					osg::Vec3 dx(0.0f, 0.0f, 0.0f);
					osg::Vec3 dy(0.0f, 0.0f, 0.0f);
					osg::Vec3 zero(0.0f, 0.0f, 0.0f);

					if (c < numColumns - 1)
					{
						int right = (r)*numColumns + c + 1;
						dx += v[right] - v[center];
					}
					if (c > 0)
					{
						int left = (r)*numColumns + c - 1;
						dx += v[center] - v[left];
					}
					if (r > 0)
					{
						int down = (r - 1)*numColumns + c;
						dy += v[center] - v[down];
					}
					if (r < numRows-1)
					{
						int up = (r + 1)*numColumns + c;
						dy += v[up] - v[center];
					}

					if (dx != zero && dy != zero)
					{
						osg::Vec3 normal = dx ^ dy;
						normal.normalize();
						n[center].set(normal.x(), normal.y(), normal.z());
					}
				}
			}

			geometry->setVertexArray(&v);
			geometry->setNormalArray(&n, osg::Array::BIND_PER_VERTEX);
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
					
					osg::Vec3 n00 = n[r*numColumns + c];
					osg::Vec3 n01 = n[(r+1)*numColumns + c];
					osg::Vec3 n10 = n[r*numColumns + c +1];
					osg::Vec3 n11 = n[(r + 1)*numColumns + c+1];
					
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
