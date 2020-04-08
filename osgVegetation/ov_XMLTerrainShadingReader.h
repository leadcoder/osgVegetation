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
    DetailLayer loadDetailLayer(osgDB::XmlNode* xmlNode)
    {
        std::string texture;
        if (!QueryStringAttribute(xmlNode, "Texture", texture))
            throw std::runtime_error(std::string("Serializer::loadDetailLayer - Failed to find attribute: Texture").c_str());

        DetailLayer layer(texture);
        QueryFloatAttribute(xmlNode,"Scale", layer.Scale);
        return layer;
    }

    std::vector<DetailLayer> loadDetailLayers(osgDB::XmlNode* xmlNode)
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

    TerrainSplatShadingConfig loadSplatShading(osgDB::XmlNode* xmlNode)
    {
        TerrainSplatShadingConfig splat_config;

        QueryFloatAttribute(xmlNode,"MaxDistance", splat_config.MaxDistance);
        QueryFloatAttribute(xmlNode,"ColorModulateRatio", splat_config.ColorModulateRatio);

        osgDB::XmlNode* dlsNode = getFirstNodeByName(xmlNode,"DetailLayers");
        if (dlsNode)
            splat_config.DetailLayers = loadDetailLayers(dlsNode);
        return splat_config;
    }
}