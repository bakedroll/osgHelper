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

  class IC : public osg::Referenced
  {
  public:
    virtual void pureVirtualFunc() = 0;
  };

  class ID : public osg::Referenced
  {
  public:
    virtual void pureVirtualFunc() = 0;
  };

  class IE : public osg::Referenced
  {
  public:
    virtual void pureVirtualFunc() = 0;
  };

  class IF : public osg::Referenced
  {
  public:
    virtual void pureVirtualFunc() = 0;
  };

  class C : public IC
  {
  public:
    C(osgHelper::ioc::Injector& injector)
    {
    }

    void pureVirtualFunc() override {}
  };

  class D1 : public ID
  {
  public:
    D1(osgHelper::ioc::Injector& injector)
    {
    }

    void pureVirtualFunc() override {}
  };

  class D2 : public ID
  {
  public:
    D2(osgHelper::ioc::Injector& injector)
    {
    }

    void pureVirtualFunc() override {}
  };

  class E : public IE
  {
  public:
    E(osgHelper::ioc::Injector& injector)
    {
    }

    void pureVirtualFunc() override {}
  };

  class F1 : public IF
  {
  public:
    F1(osgHelper::ioc::Injector& injector)
    {
    }

    void pureVirtualFunc() override {}
  };

  class F2 : public IF
  {
  public:
    F2(osgHelper::ioc::Injector& injector)
    {
    }

    void pureVirtualFunc() override {}
  };

  osgHelper::ioc::InjectionContainer container;

  container.registerSingletonType<A>();
  container.registerType<B>();

  container.registerSingletonInterfaceType<IC, C>();
  container.registerSingletonInterfaceType<ID, D1>();
  container.registerSingletonInterfaceType<ID, D2>();

  container.registerInterfaceType<IE, E>();
  container.registerInterfaceType<IF, F1>();
  container.registerInterfaceType<IF, F2>();

  osgHelper::ioc::Injector injector(container);

  // Singleton types
  const auto a1 = injector.inject<A>();
  const auto a2 = injector.inject<A>();

  ASSERT_EQ(a1, a2);

  // Instances
  const auto b1 = injector.inject<B>();
  const auto b2 = injector.inject<B>();

  ASSERT_NE(b1, b2);

  // Singleton interface types
  const auto c1 = injector.inject<IC>();
  const auto c2 = injector.inject<IC>();

  ASSERT_EQ(c1, c2);

  const auto dlist1 = injector.injectAll<ID>();
  const auto dlist2 = injector.injectAll<ID>();

  ASSERT_EQ(dlist1.size(), 2);
  ASSERT_EQ(dlist2.size(), 2);

  for (auto i=0; i<dlist1.size(); i++)
  {
    ASSERT_EQ(dlist1[i], dlist2[i]);
  }

  // Interface instances
  const auto e1 = injector.inject<IE>();
  const auto e2 = injector.inject<IE>();

  ASSERT_NE(e1, e2);

  const auto flist1 = injector.injectAll<IF>();
  const auto flist2 = injector.injectAll<IF>();

  ASSERT_EQ(flist1.size(), 2);
  ASSERT_EQ(flist2.size(), 2);

  for (auto i=0; i<flist1.size(); i++)
  {
    ASSERT_NE(flist1[i], flist2[i]);
  }
}
