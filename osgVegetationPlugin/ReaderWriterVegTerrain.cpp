#include "ReaderWriterVegTerrain.h"
#include <osg/Node>
#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osg/PagedLOD>
#include "ov_Utils.h"
#include "ov_Serializer.h"
#include "ov_VPBVegetationInjection.h"

using namespace osg;
using namespace osgDB;

#define OV_PSEUDO_EXT "ovt_injection"


ReaderWriterOVT::ReaderWriterOVT() :m_VPBInjection(NULL)
{

}

const char* ReaderWriterOVT::className() const
{
    // Return a description of this class
    return "OVT File Reader";
}

bool ReaderWriterOVT::acceptsExtension(const std::string& extension) const
{
    return osgDB::equalCaseInsensitive(extension, "ovt") || osgDB::equalCaseInsensitive(extension, OV_PSEUDO_EXT);
}



ReaderWriter::ReadResult ReaderWriterOVT::_readOVT(const std::string& file, const ReaderWriter::Options* options) const
{
	// See if we can find the requested file
	std::string fileName = osgDB::findDataFile(file, options);
	if (fileName.empty())
		return ReadResult::FILE_NOT_FOUND;

	osgVegetation::TerrainConfig terrain_config = osgVegetation::XMLSerializer::ReadTerrainData(fileName);


	const std::string file_path = osgDB::getFilePath(fileName);
	osgDB::Registry::instance()->getDataFilePathList().push_back(file_path);


	//Control texture slots
	osgVegetation::Register.TexUnits.AddUnit(0, OV_TERRAIN_COLOR_TEXTURE_ID);
	//osgVegetation::Register.TexUnits.AddUnit(2, OV_TERRAIN_NORMAL_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(1, OV_TERRAIN_SPLAT_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(2, OV_TERRAIN_DETAIL_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(3, OV_BILLBOARD_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(6, OV_SHADOW_TEXTURE0_ID);
	osgVegetation::Register.TexUnits.AddUnit(7, OV_SHADOW_TEXTURE1_ID);

	m_VPBInjection = new osgVegetation::VPBVegetationInjection(terrain_config.BillboardConfig);
	m_VPBInjection->SetPseudoLoaderExt(OV_PSEUDO_EXT);

	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile(terrain_config.Filename + "." + OV_PSEUDO_EXT);
	if (!terrain_node)
		return ReadResult::ERROR_IN_READING_FILE;

	osg::ref_ptr <osgVegetation::TerrainSplatShadingStateSet> terrain_shading_ss = new osgVegetation::TerrainSplatShadingStateSet(terrain_config.SplatConfig);
	osg::ref_ptr<osg::Group> root_node = new osg::Group();
	root_node->setStateSet(terrain_shading_ss);
	root_node->addChild(terrain_node);
	return root_node;
}

ReaderWriter::ReadResult ReaderWriterOVT::_readOVTInjection(const std::string& file, const ReaderWriter::Options* options) const
{
	//remove .ovt_injection
	const std::string filename = osgDB::getNameLessExtension(file);
	return m_VPBInjection->readNode(filename, options);
}

ReaderWriter::ReadResult ReaderWriterOVT::readNode(const std::string& file, const ReaderWriter::Options* options) const
{
    ref_ptr<Node>              result;
    osgDB::ifstream            stream;

    // See if we handle this kind of file
    if (!acceptsExtension(osgDB::getFileExtension(file)))
        return ReadResult::FILE_NOT_HANDLED;

	const bool is_ovt = osgDB::equalCaseInsensitive(osgDB::getFileExtension(file), "ovt");
	return is_ovt ? _readOVT(file, options) : _readOVTInjection(file,options);

#if 0	
	if (is_ovt)
	{
	
		// See if we can find the requested file
		std::string fileName = osgDB::findDataFile(file, options);
		if (fileName.empty())
			return ReadResult::FILE_NOT_FOUND;

		osgVegetation::TerrainConfig terrain_config = osgVegetation::XMLSerializer::ReadTerrainData(fileName);


		const std::string file_path = osgDB::getFilePath(fileName);
		osgDB::Registry::instance()->getDataFilePathList().push_back(file_path);


		//Control texture slots
		osgVegetation::Register.TexUnits.AddUnit(0, OV_TERRAIN_COLOR_TEXTURE_ID);
		//osgVegetation::Register.TexUnits.AddUnit(2, OV_TERRAIN_NORMAL_TEXTURE_ID);
		osgVegetation::Register.TexUnits.AddUnit(1, OV_TERRAIN_SPLAT_TEXTURE_ID);
		osgVegetation::Register.TexUnits.AddUnit(2, OV_TERRAIN_DETAIL_TEXTURE_ID);
		osgVegetation::Register.TexUnits.AddUnit(3, OV_BILLBOARD_TEXTURE_ID);
		osgVegetation::Register.TexUnits.AddUnit(6, OV_SHADOW_TEXTURE0_ID);
		osgVegetation::Register.TexUnits.AddUnit(7, OV_SHADOW_TEXTURE1_ID);


		VPBInjection = new osgVegetation::VPBVegetationInjection(terrain_config.BillboardConfig);
		//osgDB::Registry::instance()->setReadFileCallback(new osgVegetation::VPBVegetationInjection(terrain_config.BillboardConfig));

		//if(terrain_data.Type == osgVegetation::Terrain::TT_PLOD_TERRAIN)
		//	osgDB::Registry::instance()->setReadFileCallback(new osgVegetation::PLODTerrainTileInjection(terrain_data));
		//else if (terrain_data.Type == osgVegetation::Terrain::TT_PLOD_GEODE)
		//	osgDB::Registry::instance()->setReadFileCallback(new osgVegetation::GeodePLODInjection(terrain_data));

		osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile(terrain_config.Filename + ".ovt_injection");
		if(!terrain_node)
			return ReadResult::ERROR_IN_READING_FILE;
		osg::ref_ptr <osgVegetation::TerrainSplatShadingStateSet> terrain_shading_ss = new osgVegetation::TerrainSplatShadingStateSet(terrain_config.SplatConfig);
		osg::ref_ptr<osg::Group> root_node = new osg::Group();
		root_node->setStateSet(terrain_shading_ss);
		root_node->addChild(terrain_node);


		//Setup shadow defines
		osg::StateSet::DefineList& defineList = terrain_node->getOrCreateStateSet()->getDefineList();
		/*if(terrain_data.ShadowMode == osgVegetation::Terrain::SM_LISPSM)
		{
			defineList["SM_LISPSM"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		}
		else if (terrain_data.ShadowMode == osgVegetation::Terrain::SM_VDSM1)
		{
			defineList["SM_VDSM1"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		}
		else if (terrain_data.ShadowMode == osgVegetation::Terrain::SM_VDSM2)
		{
			defineList["SM_VDSM2"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		}

		int shadowTexUnit = 6;
		osg::Uniform* shadowTextureUnit = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit");
		shadowTextureUnit->set(shadowTexUnit);
		terrain_node->getOrCreateStateSet()->addUniform(shadowTextureUnit);

		defineList["FM_EXP2"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		*/
		return root_node;

	}
	else //ovt_injection
	{
		//remove .ovt_injection
		const std::string filename = osgDB::getNameLessExtension(file);
		return m_VPBInjection->readNode(filename,options);
	}
#endif
}

REGISTER_OSGPLUGIN(ovt, ReaderWriterOVT)

