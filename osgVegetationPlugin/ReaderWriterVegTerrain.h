#ifndef __READERWRITER_VBSP_H_
#define __READERWRITER_VBSP_H_


#include <osgDB/Registry>
#include <osgDB/FileNameUtils>



class ReaderWriterOVT : public osgDB::ReaderWriter
{
public:

    virtual const char*   className() const;

    virtual bool   acceptsExtension(const std::string& extension) const;

    virtual ReadResult   readNode(const std::string& file,
                                  const Options* options) const;
};

#endif
