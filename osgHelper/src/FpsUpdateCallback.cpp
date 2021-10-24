#include <osgHelper/FpsUpdateCallback.h>

#include <osg/NodeVisitor>

namespace osgHelper
{

  FpsUpdateCallback::FpsUpdateCallback()
    : osg::Callback()
    , m_framesCount(0)
    , m_lastSimulationTime(0.0)
    , m_isEnabled(true)
  {

  }

  FpsUpdateCallback::~FpsUpdateCallback() = default;

  void FpsUpdateCallback::setUpdateFunc(const std::function<void(int)>& func)
  {
    m_updateFunc = func;
  }

  bool FpsUpdateCallback::run(osg::Object* node, osg::Object* data)
  {
    if (m_updateFunc && m_isEnabled)
    {
      const auto nv = dynamic_cast<osg::NodeVisitor*>(data);
      if (!nv)
      {
        return false;
      }

      const auto time = nv->getFrameStamp()->getSimulationTime();

      if (m_lastSimulationTime != 0.0)
      {
        if (time - m_lastSimulationTime >= 1.0)
        {
          m_updateFunc(m_framesCount);
          m_framesCount = 0;
          m_lastSimulationTime = time;
        }
        else
        {
          m_framesCount++;
        }
      }
      else
      {
        m_lastSimulationTime = time;
      }
    }

    return traverse(node, data);
  }

  void FpsUpdateCallback::setEnabled(bool enabled)
  {
    m_isEnabled = enabled;
  }

  bool FpsUpdateCallback::isEnabled() const
  {
    return m_isEnabled;
  }
}
