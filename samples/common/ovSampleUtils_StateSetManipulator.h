#pragma once
#include <osgGA/GUIEventHandler>
#include <osg/Fog>
#include "ov_Scene.h"

namespace ovSampleUtils
{
	class StateSetManipulator : public osgGA::GUIEventHandler
	{
	private:
		osg::StateSet* m_StateSet;
		osg::Fog* m_Fog;
	public:
		StateSetManipulator(osg::StateSet* state_set, osg::Fog* fog = NULL) : m_StateSet(state_set),
			m_Fog(fog)
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
					if (m_Fog)
					{
						m_StateSet->setMode(GL_FOG, has_fog ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
						if(has_fog)
							osgVegetation::Scene::EnableFog(m_StateSet, m_Fog->getMode());
						else
						{
							osgVegetation::Scene::DisableFog(m_StateSet);
						}
					}
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