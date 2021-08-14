#pragma once

#include "osgHelper/ioc/InjectionContainer.h"
#include "osgHelper/LogManager.h"
#include "osgHelper/Macros.h"

#ifdef _DEBUG
#include <vector>
#endif

namespace osgHelper
{
namespace ioc
{

  class Injector
  {
#ifdef _DEBUG
  private:
    int                          m_depth;
    std::vector<std::type_index> m_creationOrder;

    void traceInstance(const std::type_index& tid)
    {
      for (const auto& it : m_creationOrder)
      {
        if (it == tid)
        {
          OSGH_LOG_WARN("Circular dependency detected!");
        }
      }

      char buffer[256];
      sprintf_s(buffer, "%*s%s%s", (m_depth)* 2, "", "Injecting ", tid.name());

      OSGH_LOG_INFO(std::string(buffer));
    }

    osg::ref_ptr<osg::Referenced> createInstance(
      const std::type_index& tid,
      const InjectionContainer::NewInstanceFunc& newInstanceFunc)
    {
      traceInstance(tid);

      m_creationOrder.push_back(tid);

      m_depth++;
      auto instance = newInstanceFunc(this);
      m_depth--;

      m_creationOrder.pop_back();

      return instance;
    }
#endif

  public:
    explicit Injector(InjectionContainer& container);

    template<typename Type>
    std::vector<osg::ref_ptr<Type>> injectAll()
    {
      const std::type_index tid = typeid(Type);
      std::vector<osg::ref_ptr<Type>> resultInstances;

      auto& interfaceSingletons = m_container->getRegisteredInterfaceSingletonTypes();
      if (interfaceSingletons.count(tid) > 0)
      {
        auto& infos = interfaceSingletons.at(tid);
        for (auto& info : infos)
        {
          resultInstances.push_back(getOrCreateSingletonInstance<Type>(info.first, info.second));
        }

        return resultInstances;
      }

      const auto& interfaceTypes = m_container->getRegisteredInterfaceTypes();
      if (interfaceTypes.count(tid) > 0)
      {
        auto& funcs = interfaceTypes.at(tid);
        for (const auto& func : funcs)
        {
          resultInstances.push_back(createTypeInstance<Type>(func.first, func.second));
        }

        return resultInstances;
      }

      OSGH_LOG_FATAL(std::string("Interface ") + tid.name() + "was not registered");
      assert(false);
      return resultInstances;
    }

    template<typename Type>
    osg::ref_ptr<Type> inject()
    {
      const std::type_index tid = typeid(Type);

      auto& interfaceSingletons = m_container->getRegisteredInterfaceSingletonTypes();
      if (interfaceSingletons.count(tid) > 0)
      {
        auto& instances = interfaceSingletons.at(tid);
        if (instances.size() > 1)
        {
          assertAndLogMoreThanOneImplementationsError(tid);
          return osg::ref_ptr<Type>();
        }

        const auto it = instances.begin();
        return getOrCreateSingletonInstance<Type>(it->first, it->second);
      }

      const auto& interfaceTypes = m_container->getRegisteredInterfaceTypes();
      if (interfaceTypes.count(tid) > 0)
      {
        const auto& instances = interfaceTypes.at(tid);
        if (instances.size() > 1)
        {
          assertAndLogMoreThanOneImplementationsError(tid);
          return osg::ref_ptr<Type>();
        }

        const auto it = instances.begin();
        return createTypeInstance<Type>(it->first, it->second);
      }

      auto& singletons = m_container->getRegisteredSingletonTypes();
      if (singletons.count(tid) > 0)
      {
        auto& info = singletons.at(tid);
        return getOrCreateSingletonInstance<Type>(tid, info);
      }
      
      const auto& types = m_container->getRegisteredTypes();
      if (types.count(tid) > 0)
      {
        return createTypeInstance<Type>(tid, types.at(tid));
      }

      OSGH_LOG_FATAL(std::string("Type ") + tid.name() + " was not registered");
      assert(false);
      return osg::ref_ptr<Type>();
    }

  private:
    InjectionContainer* m_container;

    template <typename Type>
    static osg::ref_ptr<Type> castRefPtr(const osg::ref_ptr<osg::Referenced>& ref)
    {
      return dynamic_cast<Type*>(ref.get());
    }

    static void assertAndLogMoreThanOneImplementationsError(const std::type_index& tid)
    {
      OSGH_LOG_FATAL(std::string("More than one interface implementations registered for ") + tid.name());
      assert(false);
    }

    template <typename Type>
    osg::ref_ptr<Type> createTypeInstance(const std::type_index& tid, const InjectionContainer::NewInstanceFunc& newInstanceFunc)
    {
#ifdef _DEBUG
      return castRefPtr<Type>(createInstance(tid, newInstanceFunc));
#else
      return castRefPtr<Type>(newInstanceFunc(this));
#endif
    }

    template <typename Type>
    osg::ref_ptr<Type> getOrCreateSingletonInstance(const std::type_index& tid, InjectionContainer::SingletonTypeInfo& info)
    {
      if (info.instance.valid())
      {
#ifdef _DEBUG
        traceInstance(tid);
#endif
        return castRefPtr<Type>(info.instance);
      }

#ifdef _DEBUG
      info.instance = createInstance(tid, info.newInstanceFunc);
#else
      info.instance = info.newInstanceFunc(this);
#endif
      return castRefPtr<Type>(info.instance);
    }

  };
}
}