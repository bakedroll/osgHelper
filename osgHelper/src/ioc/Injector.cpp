#include "osgHelper/ioc/Injector.h"

namespace osgHelper
{
namespace ioc
{
  Injector::Injector(InjectionContainer& container)
    : m_container(&container)
  {
#ifdef _DEBUG
    m_depth = 0;
#endif
  }
}
}