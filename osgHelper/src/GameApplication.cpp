#include <osgHelper/GameApplication.h>

#include <osgHelper/GameException.h>

#include <utilsLib/Utils.h>

#include <exception>

namespace osgHelper
{

GameApplication::GameApplication() = default;

GameApplication::~GameApplication() = default;

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
    UTILS_LOG_FATAL(std::string("Exception: ") + e.getMessage());
    onException(e.getMessage());
  }
  catch (std::exception& e)
  {
    UTILS_LOG_FATAL(std::string("Exception: ") + std::string(e.what()));
    onException(e.what());
  }

  return -1;
}

}
