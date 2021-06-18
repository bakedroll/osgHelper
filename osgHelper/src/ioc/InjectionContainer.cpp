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
  SingletonObservers singletonObservers;
  singletonObservers.reserve(m_registeredSingletons.size());

  for (const auto& singleton : m_registeredSingletons)
  {
    singletonObservers.push_back({ singleton.second, singleton.first.name() });
  }

  m_registeredClasses.clear();
  m_registeredSingletons.clear();

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
