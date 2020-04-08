#pragma once
#include "ov_VPBVegetationInjectionConfig.h"
#include "ov_XMLTerrainShadingReader.h"
#include "ov_XMLLayerReader.h"

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
    void loadRegister(osgDB::XmlNode* xmlNode)
    {
        QueryUnsignedAttribute(xmlNode, "CastsShadowTraversalMask", osgVegetation::Register.CastsShadowTraversalMask);
        QueryUnsignedAttribute(xmlNode, "ReceivesShadowTraversalMask", Register.ReceivesShadowTraversalMask);
     
        //check texture units
        int tu_color = 0;
        QueryIntAttribute(xmlNode,"TerrainColorTextureUnit", tu_color);
        if (tu_color > -1)
            osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_color, OV_TERRAIN_COLOR_TEXTURE_ID);

        int tu_splat = 1;
        QueryIntAttribute(xmlNode,"TerrainSplatTextureUnit", tu_splat);
        if (tu_splat > -1)
            osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_splat, OV_TERRAIN_SPLAT_TEXTURE_ID);

        int tu_normal = -1;
        QueryIntAttribute(xmlNode,"TerrainNormalTextureUnit", tu_normal);
        if (tu_normal > -1)
            osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_normal, OV_TERRAIN_NORMAL_TEXTURE_ID);

        int tu_elevation = -1;
        QueryIntAttribute(xmlNode, "TerrainElevationTextureUnit", tu_elevation);
        if (tu_elevation > -1)
            osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_elevation, OV_TERRAIN_ELEVATION_TEXTURE_ID);

        int tu_shadow = 6;
        QueryIntAttribute(xmlNode, "ShadowTextureUnit", tu_shadow);
        if (tu_shadow > -1)
        {
            osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_shadow, OV_SHADOW_TEXTURE0_ID);
            osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_shadow + 1, OV_SHADOW_TEXTURE1_ID);
        }
    }

    VPBInjectionLODConfig loadLODConfig(osgDB::XmlNode* xmlNode)
    {
        VPBInjectionLODConfig config;
        QueryIntAttribute(xmlNode, "TargetLevel", config.TargetLevel);
        if (osgDB::XmlNode* layers_node = getFirstNodeByName(xmlNode, "Layers"))
        {
            config.Layers = readLayers(layers_node);
        }
        return config;
    }

    VPBVegetationInjectionConfig readVPBConfig(osgDB::XmlNode* vpb_node)
    {
        VPBVegetationInjectionConfig config;

        if (osgDB::XmlNode* register_node = getFirstNodeByName(vpb_node, "Register"))
            loadRegister(register_node);

        if (osgDB::XmlNode* splat_node = getFirstNodeByName(vpb_node, "SplatShading"))
            config.SplatConfig = loadSplatShading(splat_node);

        QueryBoolAttribute(vpb_node, "TerrainCastShadow", config.TerrainCastShadow);
        QueryBoolAttribute(vpb_node, "ObjectsCastShadow", config.ObjectsCastShadow);

        for (unsigned int i = 0; i < vpb_node->children.size(); ++i)
        {
            if (vpb_node->children[i]->name == "VPBInjectionLOD")
            {
                VPBInjectionLODConfig lod_config = loadLODConfig(vpb_node->children[i].get());
                config.TerrainLODs.push_back(lod_config);
            }
        }
        return config;
    }

    VPBVegetationInjectionConfig readVPBConfig(const std::string& filename, const osgDB::Options* options)
    {
        VPBVegetationInjectionConfig config;
        osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlFile(filename, options);
        if (xmlRoot.valid())
        {
            osgDB::FilePathList& filePaths = osgDB::getDataFilePathList();
            filePaths.push_back(osgDB::getFilePath(filename));
            if (osgDB::XmlNode* vpb_node = getFirstNodeByName(xmlRoot.get(), "VPBVegetationInjectionConfig"))
            {
                config = readVPBConfig(vpb_node);
            }
            filePaths.pop_back();
        }
        return config;
    }
}