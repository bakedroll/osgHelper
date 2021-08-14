#pragma once

#include <map>

#include <osgHelper/IResourceManager.h>

namespace osgHelper
{
namespace ioc
{
class Injector;
}

class ResourceManager : public IResourceManager
{
public:
  enum class ResourceType
  {
    Detect,
    Text,
    Binary,
    Shader
  };

  explicit ResourceManager(ioc::Injector& injector);
  ~ResourceManager() override;

  std::string                 loadText(const std::string& resourceKey) override;
  char*                       loadBinary(const std::string& resourceKey) override;
  osg::ref_ptr<osg::Image>    loadImage(const std::string& resourceKey) override;
  osg::ref_ptr<osgText::Font> loadFont(const std::string& resourceKey) override;
  osg::ref_ptr<osg::Shader>   loadShader(const std::string& resourceKey, osg::Shader::Type type) override;

  void setResourceLoader(const osg::ref_ptr<IResourceLoader>& loader) override;

  void clearCacheResource(const std::string& resourceKey) override;
  void clearCache() override;

  static void                        setDefaultFont(const osg::ref_ptr<osgText::Font>& font);
  static osg::ref_ptr<osgText::Font> getDefaultFont();

private:
  using ResourceDictionary = std::map<std::string, osg::ref_ptr<osg::Object>>;

  osg::ref_ptr<IResourceLoader> resourceLoader();

  char*                     loadBytesFromStream(std::ifstream& stream, long long length);
  std::string               loadTextFromStream(std::ifstream& stream, long long length);
  osg::ref_ptr<osg::Object> loadObject(const std::string& resourceKey, ResourceType type = ResourceType::Detect,
                                       osg::Shader::Type shaderType = osg::Shader::FRAGMENT);

  osg::ref_ptr<osg::Object> getCacheItem(const std::string& key);
  void                      storeCacheItem(const std::string& key, const osg::ref_ptr<osg::Object>& obj);

  ResourceDictionary m_cache;

  osg::ref_ptr<IResourceLoader> m_resourceLoader;

  static osg::ref_ptr<osgText::Font> m_defaultFont;

};

}