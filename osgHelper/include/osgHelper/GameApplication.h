#pragma once

#include <osgHelper/ioc/Injector.h>
#include <osgHelper/ioc/InjectionContainer.h>
#include <osgHelper/Observable.h>

#include <functional>

namespace osgHelper
{
  class GameApplication
	{
  public:
    GameApplication();
    virtual ~GameApplication();

    void setupIOC(osgHelper::ioc::Injector::Mode injectorMode =
                      osgHelper::ioc::Injector::Mode::OnlyRegisteredClasses);

  protected:
    virtual void initialize(osgHelper::ioc::Injector& injector);
    virtual void deinitialize();

    virtual void onException(const std::string& message);
    int safeExecute(const std::function<int()>& func);

    virtual void registerComponents(osgHelper::ioc::InjectionContainer& container);
    void registerEssentialComponents();

    osgHelper::ioc::InjectionContainer& container();

    osgHelper::ioc::Injector& injector() const;

	private:
    osgHelper::ioc::InjectionContainer        m_container;
    std::unique_ptr<osgHelper::ioc::Injector> m_injector;

  };
}
