#pragma once

#include "osgHelper/ioc/InjectionContainer.h"
#include "osgHelper/LogManager.h"

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

    template<typename T>
    void traceInstance()
    {
      const std::type_index tid = typeid(T);

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

    template<typename T>
    osg::ref_ptr<T> createInstance()
    {
      traceInstance<T>();

      const std::type_index tid = typeid(T);
      m_creationOrder.push_back(tid);

      m_depth++;
      auto instance = new T(*this);
      m_depth--;

      m_creationOrder.pop_back();

      return instance;
    }
#endif

  public:
    explicit Injector(InjectionContainer& container);

    template<typename T>
    osg::ref_ptr<T> inject()
    {
      const std::type_index tid = typeid(T);

      auto& singletons = m_container->singletons();
      if (singletons.count(tid) > 0)
      {
        const auto& ptr = singletons[tid];
        if (ptr.valid())
        {
#ifdef _DEBUG
          traceInstance<T>();
#endif
          return osg::ref_ptr<T>(dynamic_cast<T*>(ptr.get()));
        }

#ifdef _DEBUG
        osg::ref_ptr<T> tptr = createInstance<T>();
#else
        osg::ref_ptr<T> tptr = new T(*this);
#endif
        singletons[tid] = tptr;
        return tptr;
      }
      
      const auto& classes = m_container->classes();
      if (classes.count(tid) > 0)
      {
#ifdef _DEBUG
        return createInstance<T>();
#else
        return new T(*this);
#endif
      }

      assert(false && "Class T was not registered.");
      return nullptr;
    }

  private:
    InjectionContainer* m_container;

  };
}
}