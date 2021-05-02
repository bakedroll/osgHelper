#include <osgHelper/ResizeEventCallback.h>

namespace osgHelper
{
ResizeEventHandler::ResizeEventHandler(const CallbackFunc& func)
  : osgGA::GUIEventHandler()
  , m_func(func)
{
}

ResizeEventHandler::~ResizeEventHandler() = default;

bool ResizeEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  switch (ea.getEventType())
  {
  case osgGA::GUIEventAdapter::RESIZE:
    return m_func(ea.getWindowWidth(), ea.getWindowHeight());
  default:
    break;
  }

  return osgGA::GUIEventHandler::handle(ea, aa);
}

}
