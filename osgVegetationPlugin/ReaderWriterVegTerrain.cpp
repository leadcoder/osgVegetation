#include "ReaderWriterVegTerrain.h"
#include <osg/Node>
#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osg/PagedLOD>
#include "ov_Utils.h"
#include "ov_VPBTerrainReader.h"
#include "ov_T3DBTerrainReader.h"
#include "ov_SimpleTerrainReader.h"
#include "ov_XMLRegisterReader.h"

using namespace osg;
using namespace osgDB;

namespace osgVegetation
{
	GlobalRegister Register;
}

class OVTConfig
{
public:
	osg::ref_ptr<osgDB::ReaderWriter> TerrainReader;
	std::string TerrainFilename;
};

OVTConfig readOVTConfig(const std::string& filename, const ReaderWriter::Options* options)
{
	OVTConfig ovtconfig;
	osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlFile(filename, options);
	if (xmlRoot.valid())
	{
		osgDB::FilePathList& filePaths = osgDB::getDataFilePathList();
		filePaths.push_back(osgDB::getFilePath(filename));
		if (osgDB::XmlNode* terrain_node = osgVegetation::getFirstNodeByName(xmlRoot.get(), "OVT"))
		{
			osgVegetation::QueryStringAttribute(terrain_node, "TerrainFile", ovtconfig.TerrainFilename);
			
			if (osgDB::XmlNode* register_node = osgVegetation::getFirstNodeByName(terrain_node, "Register"))
				osgVegetation::loadRegister(register_node);

			if (osgDB::XmlNode* t_node = osgVegetation::getFirstNodeByName(terrain_node, "T3DBTerrainReader"))
			{
				ovtconfig.TerrainReader = osgVegetation::T3DBTerrainReader::ReadXML(t_node);
			}
			else if (osgDB::XmlNode* vpb_node = osgVegetation::getFirstNodeByName(terrain_node, "VPBTerrainReader"))
			{
				ovtconfig.TerrainReader = osgVegetation::VPBTerrainReader::ReadXML(vpb_node);
			}
			else if (osgDB::XmlNode* str_node = osgVegetation::getFirstNodeByName(terrain_node, "SimpleTerrainReader"))
			{
				ovtconfig.TerrainReader = osgVegetation::SimpleTerrainReader::readXML(str_node);
			}
			else
				throw std::runtime_error(std::string("readOVTConfig : Failed to find Terrain Reader").c_str());

			if(!ovtconfig.TerrainReader)
				throw std::runtime_error(std::string("readOVTConfig : Failed Load Terrain Reader").c_str());
		}
		else
		{
			throw std::runtime_error(std::string("readOVTConfig : Failed to find OVT node").c_str());
		}
		filePaths.pop_back();
	}
	return ovtconfig;
}

ReaderWriterOVT::ReaderWriterOVT()
{

}

const char* ReaderWriterOVT::className() const
{
    // Return a description of this class
    return "OVT File Reader";
}

bool ReaderWriterOVT::acceptsExtension(const std::string& extension) const
{
    return osgDB::equalCaseInsensitive(extension, "ovt");
}

ReaderWriter::ReadResult ReaderWriterOVT::_readOVT(const std::string& file, const ReaderWriter::Options* options) const
{
	// See if we can find the requested file
	std::string fileName = osgDB::findDataFile(file, options);
	if (fileName.empty())
		return ReadResult::FILE_NOT_FOUND;

	const std::string file_path = osgDB::getFilePath(fileName);
	osgDB::Registry::instance()->getDataFilePathList().push_back(file_path);
	OVTConfig config = readOVTConfig(fileName, options);
	//osgDB::Registry::instance()->getDataFilePathList().push_back(file_path + "/shaders");
	osgDB::Registry::instance()->addReaderWriter(config.TerrainReader);

	const osgDB::ReaderWriter::FormatDescriptionMap& formats = config.TerrainReader->supportedExtensions();

	if(formats.empty())
		throw std::runtime_error(std::string("ReaderWriterOVT::_readOVT : Terrain Reader don't provide any extensions").c_str());

	const std::string reader_ext = formats.cbegin()->first;

	const std::string file_no_ext = osgDB::getNameLessExtension(file);
	const std::string terrain_file = config.TerrainFilename.empty() ? file_no_ext : config.TerrainFilename;

	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile(terrain_file + "." + reader_ext);
	
	if (!terrain_node)
		return ReadResult::ERROR_IN_READING_FILE;
	
	return terrain_node;
}

ReaderWriter::ReadResult ReaderWriterOVT::readNode(const std::string& file, const ReaderWriter::Options* options) const
{
    // See if we handle this kind of file
    if (!acceptsExtension(osgDB::getFileExtension(file)))
        return ReadResult::FILE_NOT_HANDLED;
	return _readOVT(file, options);
}

REGISTER_OSGPLUGIN(ovt, ReaderWriterOVT)