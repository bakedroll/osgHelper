#include <osgHelper/SimulationCallback.h>

namespace osgHelper
{

SimulationCallback::SimulationCallback()
	: osg::Callback(),
	  m_lastSimulationTime(0.0),
	  m_resetTimeDiff(false)
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

	auto time = nv->getFrameStamp()->getSimulationTime();
	auto time_diff = 0.0;

	if (m_resetTimeDiff)
	{
		m_resetTimeDiff = false;
	}
	else if (m_lastSimulationTime > 0.0)
	{
		time_diff = time - m_lastSimulationTime;
	}

	m_lastSimulationTime = time;

	action(node, data, time, time_diff);

  return true;
}

void SimulationCallback::resetTimeDiff()
{
	m_resetTimeDiff = true;
}

}