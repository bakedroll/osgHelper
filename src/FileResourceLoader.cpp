#include <osgHelper/FileResourceLoader.h>
#include <osgHelper/GameException.h>

namespace osgHelper
{

void FileResourceLoader::getResourceStream(const std::string& resourceKey, std::ifstream& stream, long long& length)
{
  stream.open(resourceKey.c_str(), std::ios::binary);

  if (!stream.is_open())
  {
    throw GameException("Could not open file '" + resourceKey + "'");
  }

  stream.seekg(0, stream.end);
  length = stream.tellg();
  stream.seekg(0, stream.beg);
}

}  // namespace osgHelper