#pragma once

#include <map>

#include <osgHelper/ResourceLoader.h>

#include <osg/Image>
#include <osgText/Font>
#include <osg/Shader>

namespace osgHelper
{
namespace ioc
{
class Injector;
}

class ResourceManager : public osg::Referenced
  {
  public:
    typedef enum _resourceType
    {
      DETECT,
      TEXT,
      BINARY,
      SHADER
    } ResourceType;

    explicit ResourceManager(ioc::Injector& injector);
    ~ResourceManager();

    std::string                 loadText(const std::string& resourceKey);
    char*                       loadBinary(const std::string& resourceKey);
    osg::ref_ptr<osg::Image>    loadImage(const std::string& resourceKey);
    osg::ref_ptr<osgText::Font> loadFont(const std::string& resourceKey);
    osg::ref_ptr<osg::Shader>   loadShader(const std::string& resourceKey, osg::Shader::Type type);

    void setResourceLoader(const osg::ref_ptr<ResourceLoader>& loader);

    void clearCacheResource(const std::string& resourceKey);
    void clearCache();

    static void                        setDefaultFont(const osg::ref_ptr<osgText::Font>& font);
    static osg::ref_ptr<osgText::Font> getDefaultFont();

  private:
    typedef std::map<std::string, osg::ref_ptr<osg::Object>> ResourceDictionary;

    osg::ref_ptr<ResourceLoader> resourceLoader();

    char*                     loadBytesFromStream(std::ifstream& stream, long long length);
    std::string               loadTextFromStream(std::ifstream& stream, long long length);
    osg::ref_ptr<osg::Object> loadObject(const std::string& resourceKey, ResourceType type = DETECT,
                                         osg::Shader::Type shaderType = osg::Shader::FRAGMENT);

    osg::ref_ptr<osg::Object> getCacheItem(const std::string& key);
    void                      storeCacheItem(const std::string& key, const osg::ref_ptr<osg::Object>& obj);

    ResourceDictionary m_cache;

    osg::ref_ptr<ResourceLoader> m_resourceLoader;

    static osg::ref_ptr<osgText::Font> m_defaultFont;

  };
}