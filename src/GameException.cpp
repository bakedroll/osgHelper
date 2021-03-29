#include <osgHelper/GameException.h>

#include <string>

namespace osgHelper
{

GameException::GameException(const std::string& message)
  : GameException(message, "")
{
}

GameException::GameException(const std::string& message, const std::string& description)
  : std::runtime_error(message.c_str())
  , m_message(message)
  , m_description(description)
{
}

std::string GameException::getMessage() const
{
  return m_message;
}

std::string GameException::getDescription() const
{
  return m_description;
}

}
