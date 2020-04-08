#include "ReaderWriterVegTerrain.h"
#include <osg/Node>
#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osg/PagedLOD>
#include "ov_Utils.h"
#include "ov_VPBVegetationInjection.h"
#include "ov_CustomVegetationInjection.h"
#include "ov_XMLVPBReader.h"

using namespace osg;
using namespace osgDB;

namespace osgVegetation
{
	GlobalRegister Register;
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


class OVTConfig
{
public:
	osgVegetation::VPBVegetationInjectionConfig VPBConfig;
	std::string Filename;
	std::string VPBType;

};

OVTConfig readOVTConfig(const std::string& filename, const ReaderWriter::Options* options)
{
	OVTConfig ovtconfig;
	osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlFile(filename, options);
	if (xmlRoot.valid())
	{
		osgDB::FilePathList& filePaths = osgDB::getDataFilePathList();
		filePaths.push_back(osgDB::getFilePath(filename));
		if (osgDB::XmlNode* terrain_node = osgVegetation::getFirstNodeByName(xmlRoot.get(), "Terrain"))
		{
			if (!osgVegetation::QueryStringAttribute(terrain_node, "File", ovtconfig.Filename))
				throw std::runtime_error(std::string("Failed to find attribute: File").c_str());

			osgVegetation::QueryStringAttribute(terrain_node, "Type", ovtconfig.VPBType);
			if (osgDB::XmlNode* register_node = osgVegetation::getFirstNodeByName(terrain_node, "Register"))
				osgVegetation::loadRegister(register_node);

			if (osgDB::XmlNode* vpb_node = osgVegetation::getFirstNodeByName(terrain_node, "VPBVegetationInjectionConfig"))
			{
				ovtconfig.VPBConfig = osgVegetation::readVPBConfig(vpb_node);

			}
		}
		filePaths.pop_back();
	}
	return ovtconfig;
}


ReaderWriter::ReadResult ReaderWriterOVT::_readOVT(const std::string& file, const ReaderWriter::Options* options) const
{
	// See if we can find the requested file
	std::string fileName = osgDB::findDataFile(file, options);
	if (fileName.empty())
		return ReadResult::FILE_NOT_FOUND;

	OVTConfig config = readOVTConfig(fileName, options);
	const std::string file_path = osgDB::getFilePath(fileName);
	osgDB::Registry::instance()->getDataFilePathList().push_back(file_path);
	osgDB::Registry::instance()->getDataFilePathList().push_back(file_path + "/shaders");
	

	const bool custom_vpb = config.VPBType == "custom" ? true : false;
	osgVegetation::VPBVegetationInjection* vegetation_reader = custom_vpb ?  new osgVegetation::CustomVegetationInjection(config.VPBConfig) :
		new osgVegetation::VPBVegetationInjection(config.VPBConfig);
	
	osgDB::Registry::instance()->addReaderWriter(vegetation_reader);
	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile(config.Filename + "." + OV_PLOD_READER_EXT);

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

