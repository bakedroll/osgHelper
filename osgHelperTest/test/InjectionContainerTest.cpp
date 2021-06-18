#include <gtest/gtest.h>

#include <osgHelper/ioc/InjectionContainer.h>
#include <osgHelper/ioc/Injector.h>

TEST(InjectionContainerTest, DependencyTree)
{
  class A : public osg::Referenced
  {
  public:
    A(osgHelper::ioc::Injector& injector)
    {
    }
  };

  class B : public osg::Referenced
  {
  public:
    B(osgHelper::ioc::Injector& injector)
    {
    }
  };

  osgHelper::ioc::InjectionContainer container;

  container.registerSingletonType<A>();
  container.registerType<B>();

  osgHelper::ioc::Injector injector(container);

  // Singleton classes
  osg::ref_ptr<A> a1 = injector.inject<A>();
  osg::ref_ptr<A> a2 = injector.inject<A>();

  ASSERT_EQ(a1, a2);

  // Instances
  osg::ref_ptr<B> b1 = injector.inject<B>();
  osg::ref_ptr<B> b2 = injector.inject<B>();

  ASSERT_NE(b1, b2);
}
