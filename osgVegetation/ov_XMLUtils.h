#pragma once

#include <osg/io_utils>
#include <osg/FrameBufferObject>
#include <osg/ImageStream>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/XmlParser>
#include <iostream>
#include <sstream>

namespace osgVegetation
{
    static bool isXMLNodeType(osgDB::XmlNode* xmlNode)
    {
        switch (xmlNode->type)
        {
        case osgDB::XmlNode::ATOM:
        case osgDB::XmlNode::NODE:
        case osgDB::XmlNode::GROUP:
            return true;
        default:
            return false;
        }
    }

    bool QueryStringAttribute(osgDB::XmlNode* xmlNode, const std::string name, std::string& value)
    {
        std::map<std::string, std::string>::const_iterator iter = xmlNode->properties.find(name);
        if (iter != xmlNode->properties.end())
        {
            value = iter->second.c_str();
            return true;
        }
        return false;
    }

    bool QueryBoolAttribute(osgDB::XmlNode* xmlNode, const std::string name, bool& value)
    {
        std::map<std::string, std::string>::const_iterator iter = xmlNode->properties.find(name);
        if (iter != xmlNode->properties.end())
        {
            value = (iter->second == "true") || (iter->second == "1");
            return true;
        }
        return false;
    }

    bool QueryFloatAttribute(osgDB::XmlNode* xmlNode, const std::string name, float& value)
    {
        std::map<std::string, std::string>::const_iterator iter = xmlNode->properties.find(name);
        if (iter != xmlNode->properties.end())
        {
            value = atof(iter->second.c_str());
            return true;
        }
        return false;
    }

    bool QueryIntAttribute(osgDB::XmlNode* xmlNode, const std::string name, int& value)
    {
        std::map<std::string, std::string>::const_iterator iter = xmlNode->properties.find(name);
        if (iter != xmlNode->properties.end())
        {
            value = atoi(iter->second.c_str());
            return true;
        }
        return false;
    }

    bool QueryUnsignedAttribute(osgDB::XmlNode* xmlNode, const std::string name, unsigned int& value)
    {
        std::map<std::string, std::string>::const_iterator iter = xmlNode->properties.find(name);
        if (iter != xmlNode->properties.end())
        {
            value = atoi(iter->second.c_str());
            return true;
        }
        return false;
    }
	
	osgDB::XmlNode* getFirstNodeByName(osgDB::XmlNode* xmlNode, const std::string &name)
    {
        if (xmlNode)
        {
            if (xmlNode->name == name)
            {
                return xmlNode;
            }
            else
            {
                for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
                {
                    osgDB::XmlNode* ret_node = getFirstNodeByName(xmlNode->children[i].get(), name);
                    if (ret_node)
                        return ret_node;
                }
            }
        }
        return NULL;
    }
}