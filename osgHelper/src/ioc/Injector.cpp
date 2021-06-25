#include "osgHelper/ioc/Injector.h"

namespace osgHelper
{
namespace ioc
{
  Injector::Injector(InjectionContainer& container, Mode mode)
    : m_container(&container)
    , m_mode(mode)
  {
#ifdef _DEBUG
    m_depth = 0;
#endif
  }
}
}