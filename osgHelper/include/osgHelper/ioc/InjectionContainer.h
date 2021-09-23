#pragma once

#include <typeindex>
#include <set>
#include <map>
#include <cassert>
#include <functional>

#include <osg/Referenced>
#include <osg/ref_ptr>

namespace osgHelper
{
namespace ioc
{
  class Injector;

  class InjectionContainer
  {
  public:
    using ReferencedPtr   = osg::ref_ptr<osg::Referenced>;
    using NewInstanceFunc = std::function<ReferencedPtr(Injector*)>;

    struct SingletonTypeInfo
    {
      ReferencedPtr   instance;
      NewInstanceFunc newInstanceFunc;
    };

    using TypeNewInstanceFuncMap = std::map<std::type_index, NewInstanceFunc>;
    using SingletonTypeInfosMap  = std::map<std::type_index, SingletonTypeInfo>;

    using InterfaceTypesSetMap           = std::map<std::type_index, TypeNewInstanceFuncMap>;
    using InterfaceSingletonTypeInfosMap = std::map<std::type_index, SingletonTypeInfosMap>;


    InjectionContainer();
    virtual ~InjectionContainer() = default;

    template<typename Type>
    void registerType()
    {
      const std::type_index tid = typeid(Type);
      assert(m_registeredTypes.count(tid) == 0);

      m_registeredTypes[tid] = getNewInstanceFunc<Type>();
    }

    template<typename Type>
    void registerSingletonType()
    {
      const std::type_index tid = typeid(Type);
      assert(m_registeredSingletonTypes.count(tid) == 0);

      m_registeredSingletonTypes[tid] = { osg::ref_ptr<osg::Referenced>(), getNewInstanceFunc<Type>() };
    }

    template<typename Interface, typename Type,
             typename = typename std::enable_if<std::is_base_of<Interface, Type>::value>::type>
    void registerInterfaceType()
    {
      const std::type_index iid = typeid(Interface);
      const std::type_index tid = typeid(Type);

      m_registeredInterfaceTypes[iid][tid] = getNewInstanceFunc<Type>();
    }

    template<typename Interface, typename Type,
             typename = typename std::enable_if<std::is_base_of<Interface, Type>::value>::type>
    void registerSingletonInterfaceType()
    {
      const std::type_index iid = typeid(Interface);
      const std::type_index tid = typeid(Type);

      m_registeredInterfaceSingletonTypes[iid][tid] = { osg::ref_ptr<osg::Referenced>(), getNewInstanceFunc<Type>() };
    }

    const TypeNewInstanceFuncMap& getRegisteredTypes() const;
    SingletonTypeInfosMap&        getRegisteredSingletonTypes();

    const InterfaceTypesSetMap&     getRegisteredInterfaceTypes() const;
    InterfaceSingletonTypeInfosMap& getRegisteredInterfaceSingletonTypes();

    void clear();

  private:
    TypeNewInstanceFuncMap m_registeredTypes;
    SingletonTypeInfosMap  m_registeredSingletonTypes;

    InterfaceTypesSetMap           m_registeredInterfaceTypes;
    InterfaceSingletonTypeInfosMap m_registeredInterfaceSingletonTypes;

    template<typename Type>
    static std::function<ReferencedPtr(Injector*)> getNewInstanceFunc()
    {
      return [](Injector* injector)
      {
        return new Type(*injector);
      };
    }

  };

}
}