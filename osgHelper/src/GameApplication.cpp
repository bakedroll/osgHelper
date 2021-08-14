#include <osgHelper/GameApplication.h>

#include <osgHelper/GameException.h>

#include <future>

#include <osgHelper/ShaderFactory.h>
#include <osgHelper/TextureFactory.h>
#include <osgHelper/ResourceManager.h>
#include <osgHelper/LogManager.h>

using namespace osg;
using namespace std;

namespace osgHelper
{

GameApplication::GameApplication()
  : m_injector(nullptr)
{
}

GameApplication::~GameApplication()
{
  m_container.clear();
  osgHelper::LogManager::clearInstance();
}

void GameApplication::onException(const std::string& message)
{
}

int GameApplication::safeExecute(const std::function<int()>& func)
{
  try
  {
    return func();
  }
  catch (osgHelper::GameException& e)
  {
    OSGH_LOG_FATAL(std::string("Exception: ") + e.getMessage());
    onException(e.getMessage());
  }
  catch (exception& e)
  {
    OSGH_LOG_FATAL(std::string("Exception: ") + std::string(e.what()));
    onException(e.what());
  }

  return -1;
}

void GameApplication::initialize(osgHelper::ioc::Injector& injector)
{
}

void GameApplication::registerComponents(osgHelper::ioc::InjectionContainer& container)
{
}

void GameApplication::registerEssentialComponents()
{
  m_container.registerSingletonType<osgHelper::ShaderFactory>();
  m_container.registerSingletonType<osgHelper::ResourceManager>();
  m_container.registerSingletonType<osgHelper::TextureFactory>();
}

osgHelper::ioc::Injector& GameApplication::injector() const
{
  return *m_injector;
}

void GameApplication::setupIOC()
{
  m_injector = std::make_unique<osgHelper::ioc::Injector>(m_container);

  registerComponents(m_container);
  initialize(*m_injector);
}

}
