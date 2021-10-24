#pragma once

#include <osg/Callback>

#include <functional>

namespace osgHelper
{
	class FpsUpdateCallback : public osg::Callback
	{
	public:
		FpsUpdateCallback();
    ~FpsUpdateCallback() override;

    void setUpdateFunc(const std::function<void(int)>& func);
    bool run(osg::Object* node, osg::Object* data) override;

    void setEnabled(bool enabled);
    bool isEnabled() const;

	private:
    int                      m_framesCount;
    double                   m_lastSimulationTime;
    std::function<void(int)> m_updateFunc;

    bool m_isEnabled;

  };
}