#pragma once
#include "ov_Common.h"
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
}