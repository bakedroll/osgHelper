#pragma once

#include <stdexcept>
#include <string>

namespace osgHelper
{

class GameException : public std::runtime_error
{
public:
	explicit GameException(const std::string& message);
  GameException(const std::string& message, const std::string& description);

	std::string getMessage() const;
	std::string getDescription() const;

private:
	std::string m_message;
	std::string m_description;

};

}
