#pragma once

#include "ov_BillboardLayerConfig.h"
#include "ov_MeshLayerConfig.h"
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
    BillboardLayerConfig::Billboard loadBillboard(osgDB::XmlNode* xmlNode)
    {
        std::string texture;
        if (!QueryStringAttribute(xmlNode, "Texture", texture))
            throw std::runtime_error(std::string("Serializer::loadBillboard - Failed to find attribute: texture").c_str());

        float width = 1.0f;
        float height = 1.0f;
        float intensity = 1.0f;
        float probability = 1.0f;
        QueryFloatAttribute(xmlNode, "Width", width);
        QueryFloatAttribute(xmlNode, "Height", height);
        const osg::Vec2f size(width, height);
        QueryFloatAttribute(xmlNode, "Intensity", intensity);
        QueryFloatAttribute(xmlNode, "Probability", probability);
        BillboardLayerConfig::Billboard bb(texture, size, intensity, probability);
        return bb;
    }

    osg::ref_ptr<ILayerConfig> readBillboardLayer(osgDB::XmlNode* xmlNode)
    {
        osg::ref_ptr<BillboardLayerConfig> layer = new BillboardLayerConfig;

        std::string bbl_type;
        if (!QueryStringAttribute(xmlNode, "Type", bbl_type))
        {
            throw std::runtime_error(std::string("Serializer::loadBillboardLayer - Failed to find attribute: Type").c_str());
        }
        if (bbl_type == "BLT_CROSS_QUADS")
            layer->Type = BillboardLayerConfig::BLT_CROSS_QUADS;
        else if (bbl_type == "BLT_ROTATED_QUAD")
            layer->Type = BillboardLayerConfig::BLT_ROTATED_QUAD;
        else if (bbl_type == "BLT_GRASS")
            layer->Type = BillboardLayerConfig::BLT_GRASS;
        else
            throw std::runtime_error(std::string("Serializer::loadBillboardData - Unknown billboard type:" + bbl_type).c_str());

        QueryFloatAttribute(xmlNode, "AlphaRejectValue" , layer->AlphaRejectValue);
        QueryFloatAttribute(xmlNode, "ColorImpact", layer->ColorImpact);
        QueryFloatAttribute(xmlNode, "Density", layer->Density);
        QueryFloatAttribute(xmlNode, "MaxDistance", layer->MaxDistance);
        QueryBoolAttribute(xmlNode, "CastShadow", layer->CastShadow);
        QueryBoolAttribute(xmlNode, "RecieveShadow", layer->ReceiveShadow);
        QueryStringAttribute(xmlNode, "ColorFilter", layer->Filter.ColorFilter);
        QueryStringAttribute(xmlNode, "SplatFilter", layer->Filter.SplatFilter);
        QueryStringAttribute(xmlNode, "NormalFilter", layer->Filter.NormalFilter);

        for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
        {
            BillboardLayerConfig::Billboard bb = loadBillboard(xmlNode->children[i].get());
            layer->Billboards.push_back(bb);
        }
        return layer;
    }

    MeshTypeConfig::MeshLODConfig loadMeshLOD(osgDB::XmlNode* xmlNode)
    {
        MeshTypeConfig::MeshLODConfig mesh_lod;

        if (!QueryStringAttribute(xmlNode, "MeshFile", mesh_lod.Mesh))
            throw std::runtime_error(std::string("Serializer::loadMeshLOD - Failed to find attribute: MeshFile").c_str());


        float start_dist = 0;
        float end_dist = 0;
        float fade_in_dist = 0;
        float fade_out_dist = 0;
        QueryFloatAttribute(xmlNode, "StartDistance", start_dist);
        QueryFloatAttribute(xmlNode, "EndDistance", end_dist);
        QueryFloatAttribute(xmlNode, "FadeInDistance", fade_in_dist);
        QueryFloatAttribute(xmlNode, "FadeOutDistance", fade_out_dist);
        mesh_lod.Distance.set(start_dist - fade_in_dist, start_dist, end_dist, end_dist + fade_out_dist);
        QueryFloatAttribute(xmlNode, "Intensity", mesh_lod.Intensity);
        QueryIntAttribute(xmlNode, "Type", mesh_lod.Type);
        return mesh_lod;
    }

    MeshTypeConfig loadMesh(osgDB::XmlNode* xmlNode)
    {
        MeshTypeConfig mesh;
        QueryFloatAttribute(xmlNode, "Probability", mesh.Probability);
        QueryFloatAttribute(xmlNode, "IntensityVariation", mesh.IntensityVariation);
        QueryFloatAttribute(xmlNode, "Scale", mesh.Scale);
        QueryFloatAttribute(xmlNode, "ScaleVariation", mesh.ScaleVariation);
        QueryFloatAttribute(xmlNode, "DiffuseIntensity", mesh.DiffuseIntensity);
        for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
        {
            if (xmlNode->children[i]->name == "LOD")
            {
                MeshTypeConfig::MeshLODConfig mesh_lod = loadMeshLOD(xmlNode->children[i].get());
                mesh.MeshLODs.push_back(mesh_lod);
            }
        }
        return mesh;
    }

    osg::ref_ptr<MeshLayerConfig> readMeshLayer(osgDB::XmlNode* xmlNode)
    {
        osg::ref_ptr<MeshLayerConfig> layer = new MeshLayerConfig();
        QueryFloatAttribute(xmlNode, "Density", layer->Density);
        QueryBoolAttribute(xmlNode, "CastShadow", layer->CastShadow);
        QueryBoolAttribute(xmlNode, "ReceiveShadow", layer->ReceiveShadow);
        QueryBoolAttribute(xmlNode, "BackFaceCulling", layer->BackFaceCulling);

        QueryStringAttribute(xmlNode, "ColorFilter", layer->Filter.ColorFilter);
        QueryStringAttribute(xmlNode, "SplatFilter", layer->Filter.SplatFilter);
        QueryStringAttribute(xmlNode, "NormalFilter", layer->Filter.NormalFilter);


        for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
        {
            if (xmlNode->children[i]->name == "Mesh")
            {
                MeshTypeConfig mesh = loadMesh(xmlNode->children[i].get());
                layer->MeshTypes.push_back(mesh);
            }
        }

        //support override lod.settings
        float lod0_dist = -1;
        float lod1_dist = -1;
        float lod0_fade_dist = -1;
        float lod1_fade_dist = -1;
        float intensity = -1;
        float dist_scale = 1;

        QueryFloatAttribute(xmlNode, "DefaultDistanceLOD0", lod0_dist);
        QueryFloatAttribute(xmlNode, "DefaultFadeLOD0", lod0_fade_dist);
        QueryFloatAttribute(xmlNode, "DefaultDistanceLOD1", lod1_dist);
        QueryFloatAttribute(xmlNode, "DefaultFadeLOD0", lod1_fade_dist);
        QueryFloatAttribute(xmlNode, "DefaultIntensity", intensity);
        QueryFloatAttribute(xmlNode, "DistanceScale", dist_scale);

        for (size_t i = 0; i < layer->MeshTypes.size(); i++)
        {
            for (size_t j = 0; j < layer->MeshTypes[i].MeshLODs.size(); j++)
            {
                if (layer->MeshTypes[i].MeshLODs[j].Distance.w() == 0)
                {
                    if (j == 0 && lod0_dist > -1)
                    {
                        layer->MeshTypes[i].MeshLODs[j].Distance.set(0, 0, lod0_dist, lod0_dist + lod0_fade_dist);
                    }
                    if (j == 1 && lod0_dist > -1 && lod1_dist > -1)
                    {
                        layer->MeshTypes[i].MeshLODs[j].Distance.set(lod0_dist - lod0_fade_dist, lod0_dist, lod1_dist, lod1_dist + lod1_fade_dist);
                    }
                    layer->MeshTypes[i].MeshLODs[j].Distance = layer->MeshTypes[i].MeshLODs[j].Distance * dist_scale;
                }
                if (intensity > -1)
                    layer->MeshTypes[i].MeshLODs[j].Intensity = intensity;
            }
        }
        return layer;
    }


    std::vector<osg::ref_ptr<ILayerConfig>> readLayers(osgDB::XmlNode* xmlNOde)
    {
        std::vector<osg::ref_ptr<ILayerConfig>> layers;
        for (unsigned int i = 0; i < xmlNOde->children.size(); ++i)
        {
            if (xmlNOde->children[i]->name == "BillboardLayer")
            {
                layers.push_back(readBillboardLayer(xmlNOde->children[i].get()));
            }
            else if (xmlNOde->children[i]->name == "MeshLayer")
            {
                layers.push_back(readMeshLayer(xmlNOde->children[i].get()));
            }
        }
        return layers;
    }

    std::vector<osg::ref_ptr<ILayerConfig>> readLayers(const std::string& filename, const osgDB::Options* options)
    {
        std::vector<osg::ref_ptr<ILayerConfig>> layers;
        osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlFile(filename, options);
        if (xmlRoot.valid())
        {
            osgDB::FilePathList& filePaths = osgDB::getDataFilePathList();
            filePaths.push_back(osgDB::getFilePath(filename));
            if(osgDB::XmlNode* layer_node = getFirstNodeByName(xmlRoot.get(), "Layers"))
            {
                layers = readLayers(layer_node);
            }
            filePaths.pop_back();
        }
        return layers;
    } 
}