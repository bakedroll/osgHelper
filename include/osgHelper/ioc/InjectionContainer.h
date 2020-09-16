#pragma once

#include <typeindex>
#include <set>
#include <map>
#include <cassert>

#include <osg/Referenced>
#include <osg/ref_ptr>

namespace osgHelper
{
namespace ioc
{

  class InjectionContainer
  {
  public:
    using Classes    = std::set<std::type_index>;
    using Singletons = std::map<std::type_index, osg::ref_ptr<osg::Referenced>>;

    InjectionContainer();

    template<typename T>
    void registerType()
    {
      std::type_index tid = typeid(T);
      assert(m_registeredClasses.count(tid) == 0);

      m_registeredClasses.insert(tid);
    }

    template<typename T>
    void registerSingletonType()
    {
      std::type_index tid = typeid(T);
      assert(m_registeredSingletons.count(tid) == 0);

      m_registeredSingletons[tid] = osg::ref_ptr<osg::Referenced>();
    }

    Classes&    classes();
    Singletons& singletons();

    void clear();

  private:
    Classes    m_registeredClasses;
    Singletons m_registeredSingletons;

  };

}
}