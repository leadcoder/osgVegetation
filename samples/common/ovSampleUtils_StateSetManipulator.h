#pragma once
#include <osgGA/GUIEventHandler>

namespace ovSampleUtils
{
	class StateSetManipulator : public osgGA::GUIEventHandler
	{
	private:
		osg::StateSet* m_StateSet;
	public:
		StateSetManipulator(osg::StateSet* state_set = 0) : m_StateSet(state_set)
		{

		}

		bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/, osg::Object*, osg::NodeVisitor* /*nv*/)
		{
			//if (ea.getHandled()) return false;

			switch (ea.getEventType())
			{
			case(osgGA::GUIEventAdapter::KEYDOWN):
			{
				if (ea.getKey() == 'l')
				{
					static bool has_light = true;
					has_light = !has_light;
					m_StateSet->setDefine("OSG_LIGHTING", has_light ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
					return true;
				}
				else if (ea.getKey() == 'o')
				{
					static bool has_fog = true;
					has_fog = !has_fog;
					return true;
				}
				break;
			}
			default:
				break;
			}
			return false;
		}
	};
}