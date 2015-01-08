#include "VegetationUtils.h"
#include "BillboardData.h"
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>

namespace osgVegetation
{
	osg::ref_ptr<osg::Texture2DArray> Utils::loadTextureArray(BillboardData &data)
	{
		int tex_width = 0;
		int tex_height = 0;
		//Load textures
		const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options(); 
		options->setOptionString("dds_flip");
		std::map<std::string, osg::Image*> image_map;
		std::map<std::string, int> index_map;
		int num_textures = 0;
		for(size_t i = 0; i < data.Layers.size();i++)
		{
			if(image_map.find(data.Layers[i].TextureName) == image_map.end() )
			{
				osg::Image* image = osgDB::readImageFile(data.Layers[i].TextureName,options);
				if(image && tex_width == 0) // first image decide array size
				{
					tex_width = image->s();
					tex_height = image->t();
				}
				image_map[data.Layers[i].TextureName] = image;
				index_map[data.Layers[i].TextureName] = num_textures;
				data.Layers[i]._TextureIndex = num_textures;
				num_textures++;
			}
			else
				data.Layers[i]._TextureIndex = index_map[data.Layers[i].TextureName];
		}

		osg::ref_ptr<osg::Texture2DArray> tex = new osg::Texture2DArray;
		tex->setTextureSize(tex_width, tex_height, num_textures);
		tex->setUseHardwareMipMapGeneration(true);   

		for(size_t i = 0; i < data.Layers.size();i++)
		{
			tex->setImage(index_map[data.Layers[i].TextureName], image_map[data.Layers[i].TextureName]);
		}
		return tex;
	}
}