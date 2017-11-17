#pragma once
#include "ov_Common.h"
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
namespace osgVegetation
{
	class BillboardLayer
	{
	public:
		class Billboard
		{
		public:
			Billboard(const std::string &texture, const osg::Vec2f &size, float intensity) : TextureName(texture),
				Size(size),
				Intensity(intensity)
			{

			}

			~Billboard()
			{

			}
			std::string TextureName;
			osg::Vec2f Size;
			float Intensity;
		private:
		};

		BillboardLayer(float max_dist = 150, float density = 8, float color_threshold = 0.2, float color_impact = 1.0, float lc_id = -1, int lod_level = -1) : MaxDistance(max_dist),
			Density(density), 
			ColorThreshold(color_threshold),
			ColorImpact(color_impact),
			LandCoverID(lc_id),
			LODLevel(lod_level)
		{

			//m_TexArray = osgVegetation::Utils::loadTextureArray(tex_names);
		}

		~BillboardLayer()
		{

		}

		float MaxDistance;
		float Density;
		float ColorThreshold;
		float ColorImpact;
		float LandCoverID;
		int LODLevel;
		std::vector<Billboard> Billboards;

		osg::ref_ptr<osg::Texture2DArray> _BillboardTextures;
		osg::ref_ptr<osg::Texture2DArray> GetOrCreateTexArray()
		{
			if (_BillboardTextures.valid())
				return _BillboardTextures;
			_BillboardTextures = CreateTextureArray();
			return _BillboardTextures;
		}

		osg::ref_ptr<osg::Texture2DArray> CreateTextureArray()
		{
			//Load textures
			const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options();
			//options->setOptionString("dds_flip");
			osg::ref_ptr<osg::Texture2DArray> tex = new osg::Texture2DArray;

			for (size_t i = 0; i < Billboards.size(); i++)
			{
				const std::string texture_name = Billboards[i].TextureName;
				osg::Image* image = osgDB::readImageFile(texture_name, options);
				if (image == NULL)
					OSGV_EXCEPT(std::string("BillboardLayer::CreateTextureArray - Failed to load texture:" + texture_name).c_str());
				if (i == 0) // first image decide array size
				{
					tex->setTextureSize(image->s(), image->t(), Billboards.size());
					tex->setUseHardwareMipMapGeneration(true);
				}
				tex->setImage(i, image);
			}
			return tex;
		}
	private:
	};
}
