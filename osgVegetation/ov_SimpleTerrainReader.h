#pragma once
#include "ov_Common.h"
#include "ov_XMLTerrainShadingReader.h"
#include "ov_XMLLayerReader.h"
#include <vector>

#define OV_SIMPLE_TERRAIN_READER_EXT "str"

namespace osgVegetation
{
	class SimpleTerrainReaderConfig
	{
	public:
		SimpleTerrainReaderConfig() {}
		std::vector<osg::ref_ptr<ILayerConfig> > Layers;
		osg::ref_ptr<osg::StateSet> TerrainStateSet;
	};

	class SimpleTerrainReader : public osgDB::ReaderWriter
	{
	public:
		
		SimpleTerrainReader(const SimpleTerrainReaderConfig& config) : m_Generator(config.Layers),
			m_TerrainStateSet(config.TerrainStateSet)
		{
			supportsExtension(OV_SIMPLE_TERRAIN_READER_EXT, "SimpleTerrainReader");
		}

		virtual const char* className() const
		{
			// Return a description of this class
			return "SimpleTerrainReader";
		}

		virtual bool acceptsExtension(const std::string& extension) const
		{
			return osgDB::equalCaseInsensitive(extension, OV_SIMPLE_TERRAIN_READER_EXT);
		}

		virtual ReadResult readNode(const std::string& file,
			const osgDB::Options* options) const
		{
			if (!acceptsExtension(osgDB::getFileExtension(file)))
				return ReadResult::FILE_NOT_HANDLED;

			const std::string ext = osgDB::getFileExtension(file);

			const std::string filename = osgDB::getNameLessExtension(file);
			osgDB::ReaderWriter::ReadResult terrain_rr = osgDB::readNodeFile(filename, options);

			if (!terrain_rr.getNode())
				return terrain_rr;

			if (!terrain_rr.validNode())
				return terrain_rr;

			osg::ref_ptr<osg::Group> vegetation = m_Generator.CreateVegetationNode(osgVegetation::CloneAndConvertToPatches(terrain_rr.getNode(), osg::CopyOp::DEEP_COPY_ALL));
			vegetation->addChild(terrain_rr.getNode());
			if (m_TerrainStateSet)
				vegetation->getOrCreateStateSet()->merge(*m_TerrainStateSet);
			return vegetation;
		}

		static osg::ref_ptr<SimpleTerrainReader> readXML(osgDB::XmlNode* node)
		{
			SimpleTerrainReaderConfig config;
		
			if (osgDB::XmlNode* terrain_state_node = getFirstNodeByName(node, "TerrainStateSet"))
				config.TerrainStateSet = loadTerrainStateSet(terrain_state_node);
			
			if (osgDB::XmlNode* layers_node = getFirstNodeByName(node, "Layers"))
			{
				config.Layers = readLayers(layers_node);
			}
			return new SimpleTerrainReader(config);
		}

		static osg::ref_ptr<SimpleTerrainReader> readXML(const std::string& filename, const osgDB::Options* options)
		{
			osg::ref_ptr<SimpleTerrainReader> str;
			osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlFile(filename, options);
			if (xmlRoot.valid())
			{
				osgDB::FilePathList& filePaths = osgDB::getDataFilePathList();
				filePaths.push_back(osgDB::getFilePath(filename));
				if (osgDB::XmlNode* node = getFirstNodeByName(xmlRoot.get(), "SimpleTerrainReader"))
				{
					str = readXML(node);
				}
				filePaths.pop_back();
			}
			return str;
		}
	protected:
		virtual ~SimpleTerrainReader() {}
		osg::ref_ptr<osg::StateSet> m_TerrainStateSet;
		LayerGenerator m_Generator;
	};
}