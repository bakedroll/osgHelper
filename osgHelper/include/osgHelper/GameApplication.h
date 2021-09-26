#pragma once

#include <functional>
#include <string>

namespace osgHelper
{
  class GameApplication
	{
  public:
    GameApplication();
    virtual ~GameApplication();

  protected:
    virtual void onException(const std::string& message);
    int safeExecute(const std::function<int()>& func);

  };
}
