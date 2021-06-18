#include <osgHelper/SimulationCallback.h>

#include <osg/NodeVisitor>

namespace osgHelper
{

SimulationCallback::SimulationCallback()
	: osg::Callback(),
    m_data({ 0.0, 0.0 }),
	  m_lastSimulationTime(0.0),
	  m_resetTimeDelta(false)
{

}

SimulationCallback::~SimulationCallback() = default;

bool SimulationCallback::run(osg::Object* node, osg::Object* data)
{
  const auto nv = dynamic_cast<osg::NodeVisitor*>(data);
  if (!nv)
  {
    return false;
  }

	m_data.time = nv->getFrameStamp()->getSimulationTime();
	m_data.timeDelta = 0.0;

	if (m_resetTimeDelta)
	{
		m_resetTimeDelta = false;
	}
	else if (m_lastSimulationTime > 0.0)
	{
		m_data.timeDelta = m_data.time - m_lastSimulationTime;
	}

	m_lastSimulationTime = m_data.time;

	action(m_data);

	traverse(node, data);
  return true;
}

void SimulationCallback::resetTimeDelta()
{
	m_resetTimeDelta = true;
}

}