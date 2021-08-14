#include "osgHelper/ioc/InjectionContainer.h"
#include "osgHelper/LogManager.h"

#include <vector>

#include <osg/observer_ptr>

namespace osgHelper::ioc
{

#ifdef _DEBUG

struct SingletonObserver
{
  osg::observer_ptr<osg::Referenced> ptr;
  std::string                        name;
};

using SingletonObservers = std::vector<SingletonObserver>;

#endif

InjectionContainer::InjectionContainer() = default;

const InjectionContainer::TypeNewInstanceFuncMap& InjectionContainer::getRegisteredTypes() const
{
  return m_registeredTypes;
}

InjectionContainer::SingletonTypeInfosMap& InjectionContainer::getRegisteredSingletonTypes()
{
  return m_registeredSingletonTypes;
}

const InjectionContainer::InterfaceTypesSetMap& InjectionContainer::getRegisteredInterfaceTypes() const
{
  return m_registeredInterfaceTypes;
}

InjectionContainer::InterfaceSingletonTypeInfosMap& InjectionContainer::getRegisteredInterfaceSingletonTypes()
{
  return m_registeredInterfaceSingletonTypes;
}

void InjectionContainer::clear()
{
#ifdef _DEBUG
  SingletonObservers singletonObservers;
  singletonObservers.reserve(m_registeredSingletonTypes.size());

  for (const auto& singleton : m_registeredSingletonTypes)
  {
    singletonObservers.push_back({ singleton.second.instance, singleton.first.name() });
  }
#endif

  m_registeredTypes.clear();
  m_registeredSingletonTypes.clear();

#ifdef _DEBUG
  for (const auto& it : singletonObservers)
  {
    if (!it.ptr.valid())
    {
      continue;
    }

    const auto refCount = it.ptr->referenceCount();
    OSGH_LOG_WARN(it.name + " has " + std::to_string(refCount) + " references left");
  }
#endif
}

}
