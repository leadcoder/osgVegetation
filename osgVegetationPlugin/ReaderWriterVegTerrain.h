#ifndef __READERWRITER_OVT_H_
#define __READERWRITER_OVT_H_


#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

namespace osgVegetation
{
	class VPBVegetationInjection;
}


class ReaderWriterOVT : public osgDB::ReaderWriter
{
public:
	ReaderWriterOVT();
    virtual const char*   className() const;

    virtual bool   acceptsExtension(const std::string& extension) const;

    virtual ReadResult   readNode(const std::string& file,
                                  const Options* options) const;

private:
	ReaderWriter::ReadResult _readOVT(const std::string& file, const ReaderWriter::Options* options) const;
	ReaderWriter::ReadResult _readPseudo(const std::string& file, const ReaderWriter::Options* options) const;
	mutable osgVegetation::VPBVegetationInjection* m_VPBInjection;

};

#endif
