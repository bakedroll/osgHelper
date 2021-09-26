#pragma once

#include <utilsLib/PointerTypeDefinition.h>

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/observer_ptr>

namespace utilsLib
{

template <>
class PointerTypeDefinition<osg::ref_ptr<osg::Referenced>> final
{
public:
  template<class Type>
  using TypePtr = osg::ref_ptr<Type>;
  using GenericTypePtr = osg::ref_ptr<osg::Referenced>;
  using GenericWeakPtr = osg::observer_ptr<osg::Referenced>;

  template<typename Type>
  static std::function<GenericTypePtr(Injector<GenericTypePtr>&)> getNewInstanceFunc()
  {
    return [](Injector<GenericTypePtr>& injector)
    {
      return new Type(injector);
    };
  }

  template<typename Type>
  static TypePtr<Type> pointer_cast(const GenericTypePtr& ref)
  {
    return TypePtr<Type>(dynamic_cast<Type*>(ref.get()));
  }

  template<typename Type>
  static bool isNotNull(const GenericTypePtr& ref)
  {
    return ref.valid();
  }

  static bool isExpired(const GenericWeakPtr& ref)
  {
    return !ref.valid();
  }

  static long getUseCount(const GenericWeakPtr& ref)
  {
    return static_cast<long>(ref->referenceCount());
  }
};
		
}