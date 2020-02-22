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
#include "ov_CustomVegetationInjection.h"

using namespace osg;
using namespace osgDB;

#define OV_PSEUDO_EXT "ovt_injection"


namespace osgVegetation
{
	GlobalRegister Register;
}


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

	osgVegetation::OVTConfig terrain_config = osgVegetation::XMLSerializer::ReadOVTConfig(fileName);

	const std::string file_path = osgDB::getFilePath(fileName);
	osgDB::Registry::instance()->getDataFilePathList().push_back(file_path);

	//Control texture slots
	/*osgVegetation::Register.TexUnits.AddUnit(0, OV_TERRAIN_COLOR_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(1, OV_TERRAIN_SPLAT_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(2, OV_TERRAIN_DETAIL_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(3, OV_BILLBOARD_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(6, OV_SHADOW_TEXTURE0_ID);
	osgVegetation::Register.TexUnits.AddUnit(7, OV_SHADOW_TEXTURE1_ID);*/

	osg::ref_ptr <osgVegetation::TerrainSplatShadingStateSet> terrain_shading_ss = new osgVegetation::TerrainSplatShadingStateSet(terrain_config.SplatConfig);

	const bool custom_terrain = terrain_config.TerrainType == "custom" ? true : false;
	const bool inject_terrain_state_set = custom_terrain;

	m_VPBInjection = custom_terrain ?  new osgVegetation::CustomVegetationInjection(terrain_config.VPBConfig) :
		new osgVegetation::VPBVegetationInjection(terrain_config.VPBConfig);

	m_VPBInjection->SetPseudoLoaderExt(OV_PSEUDO_EXT);
	m_VPBInjection->SetTerrainCastShadow(terrain_config.TerrainCastShadow);
	m_VPBInjection->SetObjectsCastShadow(terrain_config.ObjectsCastShadow);

	if (inject_terrain_state_set)
	{
		m_VPBInjection->SetTerrainStateSet(terrain_shading_ss);
	}

	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile(terrain_config.Filename + "." + OV_PSEUDO_EXT);

	if (!terrain_node)
		return ReadResult::ERROR_IN_READING_FILE;

	if (!inject_terrain_state_set)
	{
		osg::ref_ptr<osg::Group> root_node = new osg::Group();
		root_node->setStateSet(terrain_shading_ss);
		root_node->addChild(terrain_node);
		return root_node;
	}
	else
	{
		return terrain_node;
	}
}

ReaderWriter::ReadResult ReaderWriterOVT::_readPseudo(const std::string& file, const ReaderWriter::Options* options) const
{
	if (!m_VPBInjection)
		return ReadResult::FILE_NOT_HANDLED;

	//remove pseudo extention
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
	return is_ovt ? _readOVT(file, options) : _readPseudo(file,options);
}

REGISTER_OSGPLUGIN(ovt, ReaderWriterOVT)

