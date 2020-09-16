#pragma once

#include <osg/Drawable>
#include <osg/Callback>

namespace osgHelper
{
	class SimulationCallback : public osg::Callback
	{
	public:
	  SimulationCallback();
    ~SimulationCallback() override;

    bool run(osg::Object* node, osg::Object* data) override;
		void resetTimeDiff();

	protected:
		virtual void action(osg::Object* object, osg::Object* data, double simTime, double timeDiff) = 0;

	private:
      double m_lastSimulationTime;
      bool   m_resetTimeDiff;
  };
}