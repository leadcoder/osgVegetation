#include "Serializer.h"
#include "tinyxml.h"
#include "BillboardLayer.h"

namespace osgVegetation
{
	BillboardData Serializer::loadBillboardData(const std::string &filename)
	{
		TiXmlDocument *xmlDoc = new TiXmlDocument(filename.c_str());
		if (!xmlDoc->LoadFile())
		{
			throw std::exception(std::string("Serializer::loadBillboardData - Failed to load file:" + filename).c_str());
		}
		TiXmlElement *bd_elem = xmlDoc->FirstChildElement("BillboardData");
		if(bd_elem == NULL) 
		{
			throw std::exception(std::string("Serializer::loadBillboardData - Failed to find tag: BillboardData").c_str());
		}

		BillboardLayerVector layers;
		TiXmlElement *elem = bd_elem->FirstChildElement("BillboardLayers");
		if(elem)
		{
			TiXmlElement *bl_elem = elem->FirstChildElement("BillboardLayer");
			while(bl_elem)
			{
				BillboardLayer layer("",0);
				layer.TextureName = bl_elem->Attribute("TextureName");
				bl_elem->QueryDoubleAttribute("ViewDistance",&layer.ViewDistance);
				bl_elem->QueryDoubleAttribute("Density",&layer.Density);
				double _min = 1.0; 
				double _max = 1.0;
				
				bl_elem->QueryDoubleAttribute("MinScale",&_min);
				bl_elem->QueryDoubleAttribute("MaxScale",&_max);
				layer.Scale.set(_min,_max);

				bl_elem->QueryDoubleAttribute("MinWidth",&_min);
				bl_elem->QueryDoubleAttribute("MaxWidth",&_max);
				layer.Width.set(_min,_max);
				
				bl_elem->QueryDoubleAttribute("MinHeight",&_min);
				bl_elem->QueryDoubleAttribute("MaxHeight",&_max);
				layer.Height.set(_min,_max);
				
				bl_elem->QueryDoubleAttribute("MinColorIntensity",&_min);
				bl_elem->QueryDoubleAttribute("MaxColorIntensity",&_max);
				layer.ColorIntensity.set(_min,_max);
				
				bl_elem->QueryBoolAttribute("MixInIntensity",&layer.MixInIntensity);
				bl_elem->QueryDoubleAttribute("MixInColorRatio",&layer.MixInColorRatio);
				
				layers.push_back(layer);
				bl_elem  = bl_elem->NextSiblingElement("BillboardLayer");
			}
		}
		BillboardData bb_data(layers,false,0,false);
		
		bd_elem->QueryBoolAttribute("UseAlphaBlend",&bb_data.UseAlphaBlend);
		bd_elem->QueryFloatAttribute("AlphaRefValue",&bb_data.AlphaRefValue);
		bd_elem->QueryBoolAttribute("ReceiveShadows",&bb_data.ReceiveShadows);
		bd_elem->QueryBoolAttribute("CastShadows",&bb_data.CastShadows);
		bd_elem->QueryBoolAttribute("TerrainNormal",&bb_data.TerrainNormal);
		bd_elem->QueryBoolAttribute("UseFog",&bb_data.UseFog);
		const std::string fog_mode = bd_elem->Attribute("FogMode");
		if(fog_mode == "LINEAR")
			bb_data.FogMode = osg::Fog::LINEAR;
		else if(fog_mode == "EXP")
			bb_data.FogMode = osg::Fog::EXP;
		else if(fog_mode == "EXP2")
			bb_data.FogMode = osg::Fog::EXP2;

		bd_elem->QueryBoolAttribute("UseFog",&bb_data.UseFog);
		const std::string bb_type = bd_elem->Attribute("Type");
		
		if(bb_type == "BT_CROSS_QUADS")
			bb_data.Type = BT_CROSS_QUADS;
		else if(bb_type == "BT_SCREEN_ALIGNED")
			bb_data.Type = BT_SCREEN_ALIGNED;
		
		xmlDoc->Clear();
		// Delete our allocated document and return success ;)
		delete xmlDoc;
		return bb_data;
	}
	
}