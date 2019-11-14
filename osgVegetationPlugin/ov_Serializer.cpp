#include "ov_Serializer.h"
#include "tinyxml.h"
#include <sstream>
#include <iterator>

namespace osgVegetation
{
	BillboardLayerConfig::Billboard loadBillboard(TiXmlElement *bb_elem) 
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
		BillboardLayerConfig::Billboard bb(texture,size,intensity,probability);
		return bb;
	}

	BillboardLayerConfig loadBillboardLayer(TiXmlElement *bbl_elem)
	{
		BillboardLayerConfig layer;

		if (!bbl_elem->Attribute("Type"))
			throw std::runtime_error(std::string("Serializer::loadBillboardLayer - Failed to find attribute: Type").c_str());

		const std::string bbl_type = bbl_elem->Attribute("Type");

		if (bbl_type == "BLT_CROSS_QUADS")
			layer.Type = BillboardLayerConfig::BLT_CROSS_QUADS;
		else if (bbl_type == "BLT_ROTATED_QUAD")
			layer.Type = BillboardLayerConfig::BLT_ROTATED_QUAD;
		else if (bbl_type == "BLT_GRASS")
			layer.Type = BillboardLayerConfig::BLT_GRASS;
		else
			throw std::runtime_error(std::string("Serializer::loadBillboardData - Unknown billboard type:" + bbl_type).c_str());
		
		bbl_elem->QueryFloatAttribute("AlphaRejectValue", &layer.AlphaRejectValue);
		bbl_elem->QueryFloatAttribute("ColorImpact", &layer.ColorImpact);
		bbl_elem->QueryFloatAttribute("Density", &layer.Density);
		bbl_elem->QueryFloatAttribute("MaxDistance", &layer.MaxDistance);
		bbl_elem->QueryBoolAttribute("CastShadow", &layer.CastShadow);
		bbl_elem->QueryBoolAttribute("RecieveShadow", &layer.ReceiveShadow);

	
		if(bbl_elem->Attribute("ColorFilter"))
			layer.Filter.ColorFilter = bbl_elem->Attribute("ColorFilter");

		if (bbl_elem->Attribute("SplatFilter"))
			layer.Filter.SplatFilter = bbl_elem->Attribute("SplatFilter");

		if (bbl_elem->Attribute("NormalFilter"))
			layer.Filter.NormalFilter = bbl_elem->Attribute("NormalFilter");
		
		TiXmlElement *bb_elem = bbl_elem->FirstChildElement("Billboard");
		while (bb_elem)
		{
			BillboardLayerConfig::Billboard bb = loadBillboard(bb_elem);
			layer.Billboards.push_back(bb);
			bb_elem = bb_elem->NextSiblingElement("Billboard");
		}
		return layer;
	}

	MeshTypeConfig::MeshLODConfig loadMeshLOD(TiXmlElement *mesh_lod_elem)
	{
		MeshTypeConfig::MeshLODConfig mesh_lod;

		if (!mesh_lod_elem->Attribute("MeshFile"))
			throw std::runtime_error(std::string("Serializer::loadMeshLOD - Failed to find attribute: MeshFile").c_str());

		mesh_lod.Mesh = mesh_lod_elem->Attribute("MeshFile");

		float start_dist = 0;
		float end_dist = 0;
		float fade_in_dist = 0;
		float fade_out_dist = 0;
		mesh_lod_elem->QueryFloatAttribute("StartDistance", &start_dist);
		mesh_lod_elem->QueryFloatAttribute("EndDistance", &end_dist);
		mesh_lod_elem->QueryFloatAttribute("FadeInDistance", &fade_in_dist);
		mesh_lod_elem->QueryFloatAttribute("FadeOutDistance", &fade_out_dist);
		mesh_lod.Distance.set(start_dist - fade_in_dist, start_dist, end_dist , end_dist + fade_out_dist);
		mesh_lod_elem->QueryFloatAttribute("Intensity", &mesh_lod.Intensity);
		mesh_lod_elem->QueryIntAttribute("Type", &mesh_lod.Type);
		return mesh_lod;
	}

	MeshTypeConfig loadMesh(TiXmlElement *mesh_elem)
	{
		MeshTypeConfig mesh;

		mesh_elem->QueryFloatAttribute("Probability", &mesh.Probability);
		mesh_elem->QueryFloatAttribute("IntensityVariation", &mesh.IntensityVariation);
		mesh_elem->QueryFloatAttribute("Scale", &mesh.Scale);
		mesh_elem->QueryFloatAttribute("ScaleVariation", &mesh.ScaleVariation);
		mesh_elem->QueryFloatAttribute("DiffuseIntensity", &mesh.DiffuseIntensity);
		TiXmlElement *mesh_lod_elem = mesh_elem->FirstChildElement("LOD");

		while (mesh_lod_elem)
		{
			MeshTypeConfig::MeshLODConfig mesh_lod = loadMeshLOD(mesh_lod_elem);
			mesh.MeshLODs.push_back(mesh_lod);
			mesh_lod_elem = mesh_lod_elem->NextSiblingElement("LOD");
		}

		return mesh;
	}

	MeshLayerConfig loadMeshLayer(TiXmlElement *mesh_layer_elem)
	{
		MeshLayerConfig layer;
		mesh_layer_elem->QueryFloatAttribute("Density", &layer.Density);
		mesh_layer_elem->QueryBoolAttribute("CastShadow", &layer.CastShadow);
		mesh_layer_elem->QueryBoolAttribute("ReceiveShadow", &layer.ReceiveShadow);

		if (mesh_layer_elem->Attribute("ColorFilter"))
			layer.Filter.ColorFilter = mesh_layer_elem->Attribute("ColorFilter");

		if (mesh_layer_elem->Attribute("SplatFilter"))
			layer.Filter.SplatFilter = mesh_layer_elem->Attribute("SplatFilter");

		if (mesh_layer_elem->Attribute("NormalFilter"))
			layer.Filter.NormalFilter = mesh_layer_elem->Attribute("NormalFilter");

		TiXmlElement *mesh_elem = mesh_layer_elem->FirstChildElement("Mesh");
		while (mesh_elem)
		{
			MeshTypeConfig mesh = loadMesh(mesh_elem);
			layer.MeshTypes.push_back(mesh);
			mesh_elem = mesh_elem->NextSiblingElement("Mesh");
		}


		//support override lod.settings
		float lod0_dist = -1;
		float lod1_dist = -1;
		float lod0_fade_dist = -1;
		float lod1_fade_dist = -1;
		float intensity = -1;
		mesh_layer_elem->QueryFloatAttribute("DefaultDistanceLOD0", &lod0_dist);
		mesh_layer_elem->QueryFloatAttribute("DefaultFadeLOD0", &lod0_fade_dist);
		mesh_layer_elem->QueryFloatAttribute("DefaultDistanceLOD1", &lod1_dist);
		mesh_layer_elem->QueryFloatAttribute("DefaultFadeLOD0", &lod1_fade_dist);
		mesh_layer_elem->QueryFloatAttribute("DefaultIntensity", &intensity);
		
		for (size_t i = 0; i < layer.MeshTypes.size(); i++)
		{
			for (size_t j = 0; j < layer.MeshTypes[i].MeshLODs.size(); j++)
			{
				if (layer.MeshTypes[i].MeshLODs[j].Distance.w() == 0)
				{
					if (j == 0 && lod0_dist > -1)
					{
						layer.MeshTypes[i].MeshLODs[j].Distance.set(0, 0, lod0_dist, lod0_dist + lod0_fade_dist);
					}
					if (j == 1 && lod0_dist > -1 && lod1_dist > -1)
					{
						layer.MeshTypes[i].MeshLODs[j].Distance.set(lod0_dist - lod0_fade_dist, lod0_dist, lod1_dist, lod1_dist + lod1_fade_dist);
					}
				}
				if(intensity > -1)
					layer.MeshTypes[i].MeshLODs[j].Intensity = intensity;
			}
		}

		return layer;
	}

	VPBInjectionLODConfig loadLODConfig(TiXmlElement *lod_elem)
	{
		VPBInjectionLODConfig config;
		lod_elem->QueryIntAttribute("TargetLevel", &config.TargetLevel);
	
		TiXmlElement *bblayers_elem = lod_elem->FirstChildElement("BillboardLayer");
		while (bblayers_elem)
		{
			BillboardLayerConfig bbl = loadBillboardLayer(bblayers_elem);
			config.Layers.push_back(new BillboardLayerConfig(bbl));
			bblayers_elem = bblayers_elem->NextSiblingElement("BillboardLayer");
		}
		TiXmlElement *mesh_layers_elem = lod_elem->FirstChildElement("MeshLayer");
		while (mesh_layers_elem)
		{
			MeshLayerConfig mesh_layer = loadMeshLayer(mesh_layers_elem);
			config.Layers.push_back(new MeshLayerConfig(mesh_layer));
			mesh_layers_elem = mesh_layers_elem->NextSiblingElement("MeshLayer");
		}
		return config;
	}

	VPBVegetationInjectionConfig loadInjectionConfig(TiXmlElement *injection_elem)
	{
		VPBVegetationInjectionConfig config;
		
		TiXmlElement *terrain_lod_elem = injection_elem->FirstChildElement("VPBInjectionLOD");
		while (terrain_lod_elem)
		{
			VPBInjectionLODConfig lod_config = loadLODConfig(terrain_lod_elem);
			config.TerrainLODs.push_back(lod_config);
			terrain_lod_elem = terrain_lod_elem->NextSiblingElement("VPBInjectionLOD");
		}
		return config;
	}

	void loadRegister(TiXmlElement *register_elem)
	{
		register_elem->QueryUnsignedAttribute("CastsShadowTraversalMask", &osgVegetation::Register.Scene.Shadow.CastsShadowTraversalMask);
		register_elem->QueryUnsignedAttribute("ReceivesShadowTraversalMask", &Register.Scene.Shadow.ReceivesShadowTraversalMask);
		if (register_elem->Attribute("ShadowMode"))
		{
			const std::string sm_str = register_elem->Attribute("ShadowMode");
			Register.Scene.Shadow.Mode = Register.Scene.Shadow.StringToMode(sm_str);
		}

		if (register_elem->Attribute("FogMode"))
		{
			const std::string fm_str = register_elem->Attribute("FogMode");
			Register.Scene.Fog.Mode = Register.Scene.Fog.StringToMode(fm_str);
		}

		//check texture units
		int tu_color = 0;
		register_elem->QueryIntAttribute("TerrainColorTextureUnit", &tu_color);
		if(tu_color > -1)
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_color, OV_TERRAIN_COLOR_TEXTURE_ID);

		int tu_splat = 1;
		register_elem->QueryIntAttribute("TerrainSplatTextureUnit", &tu_splat);
		if (tu_splat > -1)
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_splat, OV_TERRAIN_SPLAT_TEXTURE_ID);

		int tu_normal = -1;
		register_elem->QueryIntAttribute("TerrainNormalTextureUnit", &tu_normal);
		if (tu_normal > -1)
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_normal, OV_TERRAIN_NORMAL_TEXTURE_ID);

		int tu_elevation = -1;
		register_elem->QueryIntAttribute("TerrainElevationTextureUnit", &tu_elevation);
		if (tu_elevation > -1)
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_elevation, OV_TERRAIN_ELEVATION_TEXTURE_ID);
		
		int tu_shadow = 6;
		register_elem->QueryIntAttribute("ShadowTextureUnit", &tu_shadow);
		if (tu_shadow > -1)
		{
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_shadow, OV_SHADOW_TEXTURE0_ID);
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(tu_shadow + 1, OV_SHADOW_TEXTURE1_ID);
		}
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


	TerrainSplatShadingConfig loadSplatShading(TiXmlElement *splat_shading_elem)
	{
		TerrainSplatShadingConfig splat_config;

		splat_shading_elem->QueryFloatAttribute("MaxDistance", &splat_config.MaxDistance);
		splat_shading_elem->QueryFloatAttribute("ColorModulateRatio", &splat_config.ColorModulateRatio);

		//splat_config.ColorTexture.TexUnit = -1;
		//splat_config.SplatTexture.TexUnit = -1;
		//splat_config.DetailTextureUnit = -1;

		//splat_shading_elem->QueryIntAttribute("ColorTextureUnit", &splat_config.ColorTexture.TexUnit);
		//splat_shading_elem->QueryIntAttribute("SplatTextureUnit", &splat_config.SplatTexture.TexUnit);
		//splat_shading_elem->QueryIntAttribute("DetailTextureUnit", &splat_config.DetailTextureUnit);
		
		/*if (splat_config.ColorTexture.TexUnit > -1)
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(splat_config.ColorTexture.TexUnit, OV_TERRAIN_COLOR_TEXTURE_ID);
		if (splat_config.SplatTexture.TexUnit > -1)
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(splat_config.SplatTexture.TexUnit, OV_TERRAIN_SPLAT_TEXTURE_ID);
		if (splat_config.DetailTextureUnit > -1)
			osgVegetation::Register.TexUnits.AddUnitIfNotPresent(splat_config.DetailTextureUnit, OV_TERRAIN_DETAIL_TEXTURE_ID);
			*/

		TiXmlElement *dlv_elem = splat_shading_elem->FirstChildElement("DetailLayers");
		if (dlv_elem)
			splat_config.DetailLayers = loadDetailLayers(dlv_elem);
		return splat_config;
	}

	osgVegetation::OVTConfig  XMLSerializer::ReadOVTConfig(const std::string &filename)
	{
		OVTConfig terrain;
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
		terrain.Filename = terrain_elem->Attribute("File");

		if (terrain_elem->Attribute("Type"))
			terrain.TerrainType = terrain_elem->Attribute("Type");

		terrain_elem->QueryBoolAttribute("TerrainCastShadow", &terrain.TerrainCastShadow);
		terrain_elem->QueryBoolAttribute("ObjectsCastShadow", &terrain.ObjectsCastShadow);

		TiXmlElement *register_elem = terrain_elem->FirstChildElement("Register");
		if (register_elem)
			loadRegister(register_elem);
		
		TiXmlElement *sc_elem = terrain_elem->FirstChildElement("SplatShading");
		if (sc_elem)
			terrain.SplatConfig = loadSplatShading(sc_elem);

		TiXmlElement *injection_elem = terrain_elem->FirstChildElement("VPBVegetationInjectionConfig");
		if(injection_elem)
			terrain.VPBConfig = loadInjectionConfig(injection_elem);

		xmlDoc->Clear();
		// Delete our allocated document and return data
		delete xmlDoc;
		return terrain;
	}
}
