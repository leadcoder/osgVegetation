#include "Serializer.h"
#include "tinyxml.h"
#include "BillboardLayer.h"
#include "CoverageData.h"
#include "TerrainQuery.h"
#include <sstream>
#include <iterator>

namespace osgVegetation
{
	std::vector<BillboardData> Serializer::loadBillboardData(const std::string &filename) const
	{
		TiXmlDocument *xmlDoc = new TiXmlDocument(filename.c_str());
		if (!xmlDoc->LoadFile())
		{
			OSGV_EXCEPT(std::string("Serializer::loadBillboardData - Failed to load file:" + filename).c_str());
		}
		TiXmlElement *vd_elem = xmlDoc->FirstChildElement("VegetationData");
		if (vd_elem == NULL)
		{
			OSGV_EXCEPT(std::string("Serializer::loadBillboardData - Failed to find tag: VegetationData").c_str());
		}

		std::vector<BillboardData> bb_vector;

		TiXmlElement *bd_elem = vd_elem->FirstChildElement("BillboardData");
		while (bd_elem)
		{
			BillboardData bb_data = loadBillboardData(bd_elem);
			bb_vector.push_back(bb_data);
			bd_elem = bd_elem->NextSiblingElement("BillboardData");
		}

		xmlDoc->Clear();
		// Delete our allocated document and return data
		delete xmlDoc;
		return bb_vector;
	}

	BillboardData Serializer::loadBillboardData(TiXmlElement *bd_elem) const
	{
		BillboardLayerVector layers;
		TiXmlElement *elem = bd_elem->FirstChildElement("BillboardLayers");
		if (elem)
		{
			TiXmlElement *bl_elem = elem->FirstChildElement("BillboardLayer");
			while (bl_elem)
			{
				BillboardLayer layer("", 0);
				if (!bl_elem->Attribute("TextureName"))
					OSGV_EXCEPT(std::string("Serializer::loadBillboardData - Failed to find attribute: TextureName").c_str());

				layer.TextureName = bl_elem->Attribute("TextureName");

				if (!bl_elem->Attribute("MinTileSize"))
					OSGV_EXCEPT(std::string("Serializer::loadBillboardData - Failed to find attribute: MinTileSize").c_str());

				bl_elem->QueryDoubleAttribute("MinTileSize", &layer.MinTileSize);
				bl_elem->QueryDoubleAttribute("Density", &layer.Density);
				double _min = 1.0;
				double _max = 1.0;

				bl_elem->QueryDoubleAttribute("MinScale", &_min);
				bl_elem->QueryDoubleAttribute("MaxScale", &_max);
				layer.Scale.set(_min, _max);

				bl_elem->QueryDoubleAttribute("MinWidth", &_min);
				bl_elem->QueryDoubleAttribute("MaxWidth", &_max);
				layer.Width.set(_min, _max);

				bl_elem->QueryDoubleAttribute("MinHeight", &_min);
				bl_elem->QueryDoubleAttribute("MaxHeight", &_max);
				layer.Height.set(_min, _max);

				bl_elem->QueryDoubleAttribute("MinColorIntensity", &_min);
				bl_elem->QueryDoubleAttribute("MaxColorIntensity", &_max);
				layer.ColorIntensity.set(_min, _max);

				bl_elem->QueryBoolAttribute("UseTerrainIntensity", &layer.UseTerrainIntensity);
				bl_elem->QueryDoubleAttribute("TerrainColorRatio", &layer.TerrainColorRatio);


				if (!bl_elem->Attribute("CoverageMaterials"))
					OSGV_EXCEPT(std::string("Serializer::loadBillboardData - Failed to find material attribute").c_str());

				const std::string materials = bl_elem->Attribute("CoverageMaterials");
				//split to vector
				std::stringstream ss(materials);
				std::istream_iterator<std::string> begin(ss);
				std::istream_iterator<std::string> end;
				layer.CoverageMaterials = std::vector<std::string>(begin, end);
				layers.push_back(layer);
				bl_elem = bl_elem->NextSiblingElement("BillboardLayer");
			}
		}
		BillboardData bb_data(layers, false, 0, false);

		bd_elem->QueryBoolAttribute("UseAlphaBlend", &bb_data.UseAlphaBlend);
		bd_elem->QueryBoolAttribute("UseMultiSample", &bb_data.UseMultiSample);
		
		bd_elem->QueryFloatAttribute("AlphaRefValue", &bb_data.AlphaRefValue);
		bd_elem->QueryBoolAttribute("ReceiveShadows", &bb_data.ReceiveShadows);
		bd_elem->QueryBoolAttribute("CastShadows", &bb_data.CastShadows);
		bd_elem->QueryBoolAttribute("TerrainNormal", &bb_data.TerrainNormal);
		//bd_elem->QueryBoolAttribute("UseFog", &bb_data.UseFog);
		bd_elem->QueryIntAttribute("TilePixelSize", &bb_data.TilePixelSize);

		const std::string bb_type = bd_elem->Attribute("Type");

		if (bb_type == "BT_CROSS_QUADS")
			bb_data.Type = BT_CROSS_QUADS;
		else if (bb_type == "BT_ROTATED_QUAD")
			bb_data.Type = BT_ROTATED_QUAD;
		else if (bb_type == "BT_GRASS")
			bb_data.Type = BT_GRASS;
		else
			OSGV_EXCEPT(std::string("Serializer::loadBillboardData - Unknown billboard type:" + bb_type).c_str());


		if(bd_elem->Attribute("Technique"))
		{ 
		const std::string technique = bd_elem->Attribute("Technique");

		if (technique == "BRT_GEOMETRY_SHADER")
			bb_data.Technique = BRT_GEOMETRY_SHADER;
		else if (technique == "BRT_SHADER_INSTANCING")
			bb_data.Technique = BRT_SHADER_INSTANCING;
		else
			OSGV_EXCEPT(std::string("Serializer::loadBillboardData - Unknown billboard type:" + bb_type).c_str());
		}
		return bb_data;
	}

	EnvironmentSettings Serializer::loadEnvironmentSettings(const std::string &filename) const
	{
		TiXmlDocument *xmlDoc = new TiXmlDocument(filename.c_str());
		if (!xmlDoc->LoadFile())
		{
			OSGV_EXCEPT(std::string("Serializer::loadEnvironmentSettings - Failed to load file:" + filename).c_str());
		}
		TiXmlElement *es_elem = xmlDoc->FirstChildElement("EnvironmentSettings");
		if (es_elem  == NULL)
		{
			OSGV_EXCEPT(std::string("Serializer::loadEnvironmentSettings - Failed to find tag: EnvironmentSettings").c_str());
		}
		EnvironmentSettings settings = loadEnvironmentSettingsImpl(es_elem);
		
		xmlDoc->Clear();
		// Delete our allocated document and return data
		delete xmlDoc;
		return settings;
	}

	EnvironmentSettings Serializer::loadEnvironmentSettingsImpl(TiXmlElement *es_elem) const
	{
		EnvironmentSettings settings;
		if (!es_elem->Attribute("FogMode"))
			OSGV_EXCEPT(std::string("Serializer::loadEnvironmentSettings - Failed to find attribute: FogMode").c_str());

		const std::string fog_mode = es_elem->Attribute("FogMode");
		settings.UseFog = true;
		if (fog_mode == "LINEAR")
			settings.FogMode = osg::Fog::LINEAR;
		else if (fog_mode == "EXP")
			settings.FogMode = osg::Fog::EXP;
		else if (fog_mode == "EXP2")
			settings.FogMode = osg::Fog::EXP2;
		else if (fog_mode == "DISABLED")
			settings.UseFog = false;
		else
			OSGV_EXCEPT(std::string("Serializer::loadEnvironmentSettings  - Unknown FogMode:" + fog_mode).c_str());

		if (es_elem->Attribute("ShadowMode"))
		{
			const std::string shadow_mode = es_elem->Attribute("ShadowMode");
			if (shadow_mode == "LISPSM")
				settings.ShadowMode = SM_LISPSM;
			else if (shadow_mode == "VDSM1")
				settings.ShadowMode = SM_VDSM1;
			else if (shadow_mode == "VDSM2")
				settings.ShadowMode = SM_VDSM2;
			else if (shadow_mode == "DISABLED")
				settings.ShadowMode = SM_DISABLED;
			else
				OSGV_EXCEPT(std::string("Serializer::loadEnvironmentSettings - Unknown Shadow mode:" + shadow_mode).c_str());
		}
		es_elem->QueryIntAttribute("BaseShadowTextureUnit", &settings.BaseShadowTextureUnit);
		return settings;
	}

	osg::ref_ptr<ITerrainQuery> Serializer::loadTerrainQuery(osg::Node* terrain, const std::string &filename) const
	{
		TiXmlDocument *xmlDoc = new TiXmlDocument(filename.c_str());
		if (!xmlDoc->LoadFile())
		{
			OSGV_EXCEPT(std::string("Serializer::loadTerrainQuery - Failed to load file:" + filename).c_str());
		}
		TiXmlElement *tq_elem = xmlDoc->FirstChildElement("TerrainQuery");
		if (tq_elem == NULL)
		{
			OSGV_EXCEPT(std::string("Serializer::loadTerrainQuery - Failed to find tag: TerrainQuery").c_str());
		}

		TiXmlElement *cd_elem = tq_elem->FirstChildElement("CoverageData");
		if (cd_elem == NULL)
		{
			OSGV_EXCEPT(std::string("Serializer::loadBillboardData - Failed to find tag: CoverageData").c_str());
		}
		CoverageData cd = loadCoverageData(cd_elem);

		//Here we can add option to load other terrain query implementations
		TerrainQuery* tq = new TerrainQuery(terrain, cd);

		if (tq_elem->Attribute("CoverageTextureSuffix"))
		{
			const std::string suffix = tq_elem->Attribute("CoverageTextureSuffix");
			tq->setCoverageTextureSuffix(suffix);
		}

		if (tq_elem->Attribute("FlipCoverageCoordinates"))
		{
			bool flip = false;
			tq_elem->QueryBoolAttribute("FlipCoverageCoordinates", &flip);
			tq->setFlipCoverageCoordinates(flip);
		}


		if (tq_elem->Attribute("ColorTextureSuffix"))
		{
			const std::string suffix = tq_elem->Attribute("ColorTextureSuffix");
			tq->setColorTextureSuffix(suffix);
		}

		if (tq_elem->Attribute("FlipColorCoordinates"))
		{
			bool flip = false;
			tq_elem->QueryBoolAttribute("FlipColorCoordinates", &flip);
			tq->setFlipColorCoordinates(flip);
		}

		xmlDoc->Clear();
		// Delete our allocated document and return data
		delete xmlDoc;
		return tq;
	}


	CoverageData Serializer::loadCoverageData(TiXmlElement *cd_elem) const
	{
		CoverageData data;
		TiXmlElement *mat_elem = cd_elem->FirstChildElement("CoverageMaterial");
		while (mat_elem)
		{
			if (!mat_elem->Attribute("MatName"))
				OSGV_EXCEPT(std::string("Serializer::loadMaterialMapping - Failed to find attribute: MatName").c_str());
			const std::string name = mat_elem->Attribute("MatName");
			int r = 0, g = 0, b = 0, a = 0;
			int tr = 0, tg = 0, tb = 0, ta = 0;
			mat_elem->QueryIntAttribute("r", &r);
			mat_elem->QueryIntAttribute("g", &g);
			mat_elem->QueryIntAttribute("b", &b);
			mat_elem->QueryIntAttribute("a", &a);
			
			mat_elem->QueryIntAttribute("tr", &tr);
			mat_elem->QueryIntAttribute("tg", &tg);
			mat_elem->QueryIntAttribute("tb", &tb);
			mat_elem->QueryIntAttribute("ta", &ta);

			const CoverageColor color(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0, float(a) / 255.0);
			const CoverageColor tolerance(float(tr) / 255.0, float(tg) / 255.0, float(tb) / 255.0, float(ta) / 255.0);
			data.CoverageMaterials.push_back(CoverageData::CoverageMaterial(name, color, tolerance));
			mat_elem = mat_elem->NextSiblingElement("CoverageMaterial");
		}
		return data;
	}
}
