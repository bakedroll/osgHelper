#pragma once

#include <osgHelper/ioc/Injector.h>

namespace osgHelper
{
  template<typename T>
  class AbstractFactory : public osg::Referenced
  {
  public:
    explicit AbstractFactory(ioc::Injector& injector) {}
    ~AbstractFactory() = default;

    osg::ref_ptr<T> make() const
    {
      return new T();
    }
  };
}