#pragma once

#include <osg/Callback>

namespace osgHelper
{
	class SimulationCallback : public osg::Callback
	{
	public:
		struct SimulationData
		{
			double time;
			double timeDelta;
		};

	  SimulationCallback();
    ~SimulationCallback() override;

    bool run(osg::Object* node, osg::Object* data) override;
		void resetTimeDelta();

	protected:
		virtual void action(const SimulationData& data) = 0;

	private:
		SimulationData m_data;

    double m_lastSimulationTime;
    bool   m_resetTimeDelta;
  };
}