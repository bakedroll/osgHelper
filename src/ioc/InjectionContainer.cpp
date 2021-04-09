#include "osgHelper/ioc/InjectionContainer.h"
#include "osgHelper/LogManager.h"

namespace osgHelper
{
namespace ioc
{
  InjectionContainer::InjectionContainer() = default;

  InjectionContainer::Classes& InjectionContainer::classes()
  {
    return m_registeredClasses;
  }

  InjectionContainer::Singletons& InjectionContainer::singletons()
  {
    return m_registeredSingletons;
  }

  void InjectionContainer::clear()
  {
#ifdef _DEBUG
    for (const auto& it : m_registeredSingletons)
    {
      if (!it.second.valid())
      {
        continue;
      }

      auto refcount = it.second->referenceCount();
      if (refcount > 1)
      {
        char buffer[256];
        sprintf_s(buffer, "%s has %d references left", it.first.name(), refcount - 1);

        OSGH_LOG_WARN(std::string(buffer));
      }
    }
#endif

    m_registeredClasses.clear();
    m_registeredSingletons.clear();
  }
}
}