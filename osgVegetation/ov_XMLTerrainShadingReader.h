#pragma once
#include "ov_TerrainSplatShadingStateSet.h"
#include "ov_XMLUtils.h"
#include <osg/io_utils>
#include <osg/FrameBufferObject>
#include <osg/ImageStream>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/XmlParser>
#include <iostream>
#include <sstream>

namespace osgVegetation
{
   /* inline DetailLayer loadDetailLayer(osgDB::XmlNode* xmlNode)
    {
        std::string texture;
        if (!QueryStringAttribute(xmlNode, "Texture", texture))
            throw std::runtime_error(std::string("Serializer::loadDetailLayer - Failed to find attribute: Texture").c_str());

        DetailLayer layer(texture);
        QueryFloatAttribute(xmlNode,"Scale", layer.Scale);
        return layer;
    }

    inline std::vector<DetailLayer> loadDetailLayers(osgDB::XmlNode* xmlNode)
    {
        std::vector<osgVegetation::DetailLayer> layers;
        for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
        {
            if (xmlNode->children[i]->name == "DetailLayer")
            {
                DetailLayer dl = loadDetailLayer(xmlNode->children[i].get());
                layers.push_back(dl);
            }
        }
        return layers;
    }

    inline osg::ref_ptr<TerrainSplatShadingConfig> loadTerrainSplatShadingConfig(osgDB::XmlNode* xmlNode)
    {
        osg::ref_ptr<TerrainSplatShadingConfig> splat_config = new TerrainSplatShadingConfig();

        QueryFloatAttribute(xmlNode, "MaxDistance", splat_config->MaxDistance);
        QueryFloatAttribute(xmlNode, "ColorModulateRatio", splat_config->ColorModulateRatio);
        QueryStringAttribute(xmlNode, "ColorTexture", splat_config->ColorTexture.File);
        QueryIntAttribute(xmlNode, "ColorTextureUnit", splat_config->ColorTexture.TexUnit);
        QueryStringAttribute(xmlNode, "SplatTexture", splat_config->SplatTexture.File);
        QueryIntAttribute(xmlNode, "SplatTextureUnit", splat_config->SplatTexture.TexUnit);
        QueryStringAttribute(xmlNode, "NoiseTexture", splat_config->NoiseTexture.File);
        QueryIntAttribute(xmlNode, "NoiseTextureUnit", splat_config->NoiseTexture.TexUnit);

        osgDB::XmlNode* dlsNode = getFirstNodeByName(xmlNode, "DetailLayers");
        if (dlsNode)
            splat_config->DetailLayers = loadDetailLayers(dlsNode);
        return splat_config;
    }*/

    inline osg::ref_ptr<TerrainStateSet> loadTerrainStateSet(osgDB::XmlNode* xmlNode)
    {
        osg::ref_ptr<TerrainStateSet> tss;
        if (osgDB::XmlNode* splat_node = getFirstNodeByName(xmlNode, "TerrainSplatShading"))
        {
            TerrainSplatShadingConfig config = TerrainSplatShadingConfig::ReadXML(splat_node);
            tss = new  TerrainSplatShadingStateSet(config);
        }
        return tss;
    }
}