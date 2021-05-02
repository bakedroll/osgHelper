#pragma once

#include <functional>

#include <osgGA/GUIEventHandler>

namespace osgHelper
{

class ResizeEventHandler : public osgGA::GUIEventHandler
{
public:
  using CallbackFunc = std::function<bool(int, int)>;

  explicit ResizeEventHandler(const CallbackFunc& func);
  ~ResizeEventHandler() override;

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

private:
  CallbackFunc m_func;

};

}
