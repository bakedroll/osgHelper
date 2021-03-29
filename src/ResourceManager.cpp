#include <osgHelper/ResourceManager.h>
#include <osgHelper/TextResource.h>
#include <osgHelper/Helper.h>
#include <osgHelper/BinaryResource.h>
#include <osgHelper/GameException.h>
#include <osgHelper/FileResourceLoader.h>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>

namespace osgHelper
{

ResourceManager::ResourceManager(Injector& injector)
{
}

ResourceManager::~ResourceManager() = default;

std::string ResourceManager::loadText(const std::string& resourceKey)
{
  const auto textRes = dynamic_cast<TextResource*>(loadObject(resourceKey, TEXT).get());
  return textRes ? textRes->text : "";
}

char* ResourceManager::loadBinary(const std::string& resourceKey)
{
  const auto binRes = dynamic_cast<BinaryResource*>(loadObject(resourceKey, BINARY).get());
  return binRes ? binRes->getBytes() : nullptr;
}

osg::ref_ptr<osg::Image> ResourceManager::loadImage(const std::string& resourceKey)
{
  return dynamic_cast<osg::Image*>(loadObject(resourceKey).get());
}

osg::ref_ptr<osgText::Font> ResourceManager::loadFont(const std::string& resourceKey)
{
  return dynamic_cast<osgText::Font*>(loadObject(resourceKey).get());
}

osg::ref_ptr<osg::Shader> ResourceManager::loadShader(const std::string& resourceKey, osg::Shader::Type type)
{
  return dynamic_cast<osg::Shader*>(loadObject(resourceKey, SHADER, type).get());
}

void ResourceManager::setResourceLoader(const osg::ref_ptr<ResourceLoader>& loader)
{
  m_resourceLoader = loader;
}

void ResourceManager::clearCacheResource(const std::string& resourceKey)
{
  const auto it = m_cache.find(lowerString(resourceKey));
  if (it == m_cache.end())
  {
    throw GameException("Clear cache resource: resource key '" + resourceKey + "' not found");
  }

  m_cache.erase(it);
}

void ResourceManager::clearCache()
{
  m_cache.clear();
}

void ResourceManager::setDefaultFont(const osg::ref_ptr<osgText::Font>& font)
{
  m_defaultFont = font;
}

osg::ref_ptr<osgText::Font> ResourceManager::getDefaultFont()
{
  return m_defaultFont;
}

osg::ref_ptr<ResourceLoader> ResourceManager::resourceLoader()
{
  if (!m_resourceLoader.valid())
  {
    m_resourceLoader = new FileResourceLoader();
  }

  return m_resourceLoader;
}

char* ResourceManager::loadBytesFromStream(std::ifstream& stream, long long length)
{
  char* buffer = new char[length];
  stream.read(buffer, length);

  return buffer;
}

std::string ResourceManager::loadTextFromStream(std::ifstream& stream, long long length)
{
  char* buffer = new char[length + 1];
  stream.read(buffer, length);
  buffer[length] = '\0';

  std::string text(buffer);

  delete[] buffer;

  return text;
}

osg::ref_ptr<osg::Object> ResourceManager::loadObject(const std::string& resourceKey, ResourceType type,
                                                      osg::Shader::Type shaderType)
{
  auto obj = getCacheItem(resourceKey);
  if (obj.valid())
  {
    return obj;
  }

  std::ifstream stream;

  auto length = 0LL;
  resourceLoader()->getResourceStream(resourceKey, stream, length);

  if (type == DETECT)
  {
    auto rw  = osgDB::Registry::instance()->getReaderWriterForExtension(osgDB::getLowerCaseFileExtension(resourceKey));
    auto res = rw->readObject(stream);

    obj = res.getObject();
  }
  else if (type == TEXT)
  {
    auto textRes = new TextResource();
    textRes->text = loadTextFromStream(stream, length);

    obj = textRes;
  }
  else if (type == BINARY)
  {
    auto binRes = new BinaryResource();
    binRes->setBytes(loadBytesFromStream(stream, length));

    obj = binRes;
  }
  else if (type == SHADER)
  {
    auto       shader = new osg::Shader(shaderType);
    const auto source = loadTextFromStream(stream, length);

    shader->setShaderSource(source);

    obj = shader;
  }

  stream.close();

  storeCacheItem(resourceKey, obj);

  return obj;
}

osg::ref_ptr<osg::Object> ResourceManager::getCacheItem(const std::string& key)
{
  const auto lower_key = lowerString(key);

  const auto it = m_cache.find(lower_key);
  if (it == m_cache.end())
  {
    return nullptr;
  }

  return it->second;
}

void ResourceManager::storeCacheItem(const std::string& key, const osg::ref_ptr<osg::Object>& obj)
{
  std::string lower_key = lowerString(key);

  m_cache.insert(ResourceDictionary::value_type(lower_key, obj));
}

osg::ref_ptr<osgText::Font> ResourceManager::m_defaultFont;

}