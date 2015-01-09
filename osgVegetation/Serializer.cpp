#include "Serializer.h"
#include "tinyxml.h"
#include "BillboardLayer.h"
#include <sstream>
#include <iterator>

namespace osgVegetation
{

	BillboardData Serializer::loadBillboardData(const std::string &filename) const
	{
		TiXmlDocument *xmlDoc = new TiXmlDocument(filename.c_str());
		if (!xmlDoc->LoadFile())
		{
			throw std::exception(std::string("Serializer::loadBillboardData - Failed to load file:" + filename).c_str());
		}
		TiXmlElement *vd_elem = xmlDoc->FirstChildElement("VegetationData");
		if(vd_elem == NULL) 
		{
			throw std::exception(std::string("Serializer::loadBillboardData - Failed to find tag: VegetationData").c_str());
		}

		TiXmlElement *mm_elem = vd_elem->FirstChildElement("MaterialMapping");
		if(mm_elem == NULL) 
		{
			throw std::exception(std::string("Serializer::loadBillboardData - Failed to find tag: MaterialMapping").c_str());
		}
		MaterialMapping mapping = loadMaterialMapping(mm_elem);
		
		TiXmlElement *bd_elem = vd_elem->FirstChildElement("BillboardData");
		if(bd_elem == NULL) 
		{
			throw std::exception(std::string("Serializer::loadBillboardData - Failed to find tag: BillboardData").c_str());
		}
		
		BillboardData bb_data= loadBillboardData(bd_elem,mapping);

		xmlDoc->Clear();
		// Delete our allocated document and return data
		delete xmlDoc;
		return bb_data;
	}

	Serializer::MaterialMapping Serializer::loadMaterialMapping(TiXmlElement *mm_elem) const
	{
		MaterialMapping mapping;
		TiXmlElement *mat_elem = mm_elem->FirstChildElement("Material");
		while(mat_elem)
		{
			if(!mat_elem->Attribute("MatName"))
				throw std::exception(std::string("Serializer::loadMaterialMapping - Failed to find attribute: MatName").c_str());
			const std::string name = mat_elem->Attribute("MatName");
			int r,g,b,a;
			mat_elem->QueryIntAttribute("r",&r);
			mat_elem->QueryIntAttribute("g",&g);
			mat_elem->QueryIntAttribute("b",&b);
			mat_elem->QueryIntAttribute("a",&a);
			const MaterialColor color (float(r)/255.0,float(g)/255.0,float(b)/255.0,float(a)/255.0);
			mapping[name] = color;
			mat_elem  = mat_elem->NextSiblingElement("Material");
		}
		return mapping;
	}

	BillboardData Serializer::loadBillboardData(TiXmlElement *bd_elem, const MaterialMapping& mapping) const
	{
		BillboardLayerVector layers;
		TiXmlElement *elem = bd_elem->FirstChildElement("BillboardLayers");
		if(elem)
		{
			TiXmlElement *bl_elem = elem->FirstChildElement("BillboardLayer");
			while(bl_elem)
			{
				BillboardLayer layer("",0);
				if(!bl_elem->Attribute("TextureName"))
					throw std::exception(std::string("Serializer::loadBillboardData - Failed to find attribute: TextureName").c_str());

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


				if(!bl_elem->Attribute("Materials"))
					throw std::exception(std::string("Serializer::loadBillboardData - Failed to find material attribute").c_str());

				const std::string materials = bl_elem->Attribute("Materials");
				//split to vector
				std::stringstream ss(materials);
				std::istream_iterator<std::string> begin(ss);
				std::istream_iterator<std::string> end;
				std::vector<std::string> mat_vector(begin, end);
				for(size_t i = 0 ; i < mat_vector.size();i++)
				{
					const std::string mat_name = mat_vector[i];
					MaterialMapping::const_iterator iter = mapping.find(mat_name);
					if(iter == mapping.end())
						throw std::exception(std::string("Serializer::loadBillboardData - Failed to find material:"+ mat_name).c_str());
					const MaterialColor color = iter->second;
					layer.Materials.push_back(color);
				}
				
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
		
		if(!bd_elem->Attribute("FogMode"))
			throw std::exception(std::string("Serializer::loadBillboardData - Failed to find attribute: FogMode").c_str());

		const std::string fog_mode = bd_elem->Attribute("FogMode");

		if(fog_mode == "LINEAR")
			bb_data.FogMode = osg::Fog::LINEAR;
		else if(fog_mode == "EXP")
			bb_data.FogMode = osg::Fog::EXP;
		else if(fog_mode == "EXP2")
			bb_data.FogMode = osg::Fog::EXP2;
		else
			throw std::exception(std::string("Serializer::loadBillboardData - Unknown FogMode:" + fog_mode).c_str());

		bd_elem->QueryBoolAttribute("UseFog",&bb_data.UseFog);
		const std::string bb_type = bd_elem->Attribute("Type");
		
		if(bb_type == "BT_CROSS_QUADS")
			bb_data.Type = BT_CROSS_QUADS;
		else if(bb_type == "BT_SCREEN_ALIGNED")
			bb_data.Type = BT_SCREEN_ALIGNED;
		else
			throw std::exception(std::string("Serializer::loadBillboardData - Unknown billboard type:" + bb_type).c_str());

		return bb_data;
	}
	
	
}