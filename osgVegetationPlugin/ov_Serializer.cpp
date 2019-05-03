#include "ov_Serializer.h"
#include "tinyxml.h"
#include <sstream>
#include <iterator>

namespace osgVegetation
{
	BillboardLayer::Billboard loadBillboard(TiXmlElement *bb_elem) 
	{
		if (!bb_elem->Attribute("Texture"))
			throw std::runtime_error(std::string("Serializer::loadBillboard - Failed to find attribute: texture").c_str());

		const std::string texture = bb_elem->Attribute("Texture");

		float width = 1.0f;
		float height = 1.0f;
		float intensity = 1.0f;
		float probability = 1.0f;
		bb_elem->QueryFloatAttribute("Width", &width);
		bb_elem->QueryFloatAttribute("Height", &height);
		const osg::Vec2f size(width, height);
		bb_elem->QueryFloatAttribute("Intensity", &intensity);
		bb_elem->QueryFloatAttribute("Probability", &probability);
		BillboardLayer::Billboard bb(texture,size,intensity,probability);
		return bb;
	}

	BillboardLayer loadBillboardLayer(TiXmlElement *bbl_elem)
	{
		BillboardLayer layer;

		if (!bbl_elem->Attribute("Type"))
			throw std::runtime_error(std::string("Serializer::loadBillboardLayer - Failed to find attribute: Type").c_str());

		const std::string bbl_type = bbl_elem->Attribute("Type");

		if (bbl_type == "BLT_CROSS_QUADS")
			layer.Type = BillboardLayer::BLT_CROSS_QUADS;
		else if (bbl_type == "BLT_ROTATED_QUAD")
			layer.Type = BillboardLayer::BLT_ROTATED_QUAD;
		else if (bbl_type == "BLT_GRASS")
			layer.Type = BillboardLayer::BLT_GRASS;
		else
			throw std::runtime_error(std::string("Serializer::loadBillboardData - Unknown billboard type:" + bbl_type).c_str());
		
		bbl_elem->QueryFloatAttribute("AlphaRejectValue", &layer.AlphaRejectValue);
		bbl_elem->QueryFloatAttribute("ColorImpact", &layer.ColorImpact);
		bbl_elem->QueryFloatAttribute("ColorThreshold", &layer.ColorThreshold);
		bbl_elem->QueryFloatAttribute("Density", &layer.Density);
		bbl_elem->QueryFloatAttribute("LandCoverID", &layer.LandCoverID);
		bbl_elem->QueryIntAttribute("LODLevel", &layer.LODLevel);
		bbl_elem->QueryFloatAttribute("MaxDistance", &layer.MaxDistance);
		
		TiXmlElement *bb_elem = bbl_elem->FirstChildElement("Billboard");
		while (bb_elem)
		{
			BillboardLayer::Billboard bb = loadBillboard(bb_elem);
			layer.Billboards.push_back(bb);
			bb_elem = bb_elem->NextSiblingElement("Billboard");
		}
		return layer;
	}

	std::vector<BillboardLayer> loadBillboardLayers(TiXmlElement *bblv_elem)
	{
		std::vector<osgVegetation::BillboardLayer> layers;
		TiXmlElement *bbl_elem = bblv_elem->FirstChildElement("BillboardLayer");
		while (bbl_elem)
		{
			BillboardLayer bbl = loadBillboardLayer(bbl_elem);
			layers.push_back(bbl);
			bbl_elem = bbl_elem->NextSiblingElement("BillboardLayer");
		}
		return layers;
	}


	DetailLayer loadDetailLayer(TiXmlElement *dl_elem)
	{
		if (!dl_elem->Attribute("Texture"))
			throw std::runtime_error(std::string("Serializer::loadDetailLayer - Failed to find attribute: Texture").c_str());

		const std::string texture = dl_elem->Attribute("Texture");
		float scale=1.0;
		dl_elem->QueryFloatAttribute("Scale", &scale);
		DetailLayer layer(texture);
		layer.Scale = scale;
		return layer;
	}

	std::vector<DetailLayer> loadDetailLayers(TiXmlElement *dlv_elem)
	{
		std::vector<osgVegetation::DetailLayer> layers;
		TiXmlElement *dl_elem = dlv_elem->FirstChildElement("DetailLayer");
		while (dl_elem)
		{
			DetailLayer dl = loadDetailLayer(dl_elem);
			layers.push_back(dl);
			dl_elem = dl_elem->NextSiblingElement("DetailLayer");
		}
		return layers;
	}

	TerrainShadingConfiguration XMLSerializer::ReadTerrainData(const std::string &filename)
	{
		TiXmlDocument *xmlDoc = new TiXmlDocument(filename.c_str());
		if (!xmlDoc->LoadFile())
		{
			throw std::runtime_error(std::string("Failed to load file:" + filename).c_str());
		}
		TiXmlElement *terrain_elem = xmlDoc->FirstChildElement("Terrain");
		if (terrain_elem == NULL)
		{
			throw std::runtime_error(std::string("Failed to find tag: Terrain").c_str());
		}

		if (!terrain_elem->Attribute("File"))
			throw std::runtime_error(std::string("Failed to find attribute: File").c_str());
		//terrain.Filename = terrain_elem->Attribute("File");

		/*if (terrain_elem->Attribute("Type"))
		{
			const std::string tt_str = terrain_elem->Attribute("Type");
			if (tt_str == "TT_PLOD_TERRAIN")
				terrain.Type = Terrain::TT_PLOD_TERRAIN;
			else if (tt_str == "TT_PLOD_GEODE")
				terrain.Type = Terrain::TT_PLOD_GEODE;
		}

		terrain.ShadowMode = Terrain::SM_DISABLED;
		if (terrain_elem->Attribute("ShadowMode"))
		{
			const std::string sm_str = terrain_elem->Attribute("ShadowMode");
			if (sm_str == "SM_LISPSM")
				terrain.ShadowMode = Terrain::SM_LISPSM;
			else if (sm_str == "SM_VDSM1")
				terrain.ShadowMode = Terrain::SM_VDSM1;
			else if (sm_str == "SM_VDSM2")
				terrain.ShadowMode = Terrain::SM_VDSM2;
			else if (sm_str == "SM_DISABLED")
				terrain.ShadowMode = Terrain::SM_DISABLED;
			else if (sm_str == "SM_UNDEFINED")
				terrain.ShadowMode = Terrain::SM_UNDEFINED;
			else
				throw std::runtime_error(std::string("Serializer::GetTerrainData - Unknown ShadowMode:" + sm_str).c_str());
		}*/

		TerrainTextureUnitSettings tex_units;
		TerrainShadingConfiguration config(tex_units);
		TiXmlElement *dlv_elem = terrain_elem->FirstChildElement("DetailLayers");
		if (dlv_elem)
			config.DetailLayers = loadDetailLayers(dlv_elem);

		TiXmlElement *bblv_elem = terrain_elem->FirstChildElement("BillboardLayers");
		//if(bblv_elem)
			//terrain.BillboardLayers = loadBillboardLayers(bblv_elem);
		xmlDoc->Clear();
		// Delete our allocated document and return data
		delete xmlDoc;
		return config;
	}
}
