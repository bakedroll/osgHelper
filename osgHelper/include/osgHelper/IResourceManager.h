#pragma once

#include <osgHelper/IResourceLoader.h>

#include <osg/Referenced>
#include <osg/Image>
#include <osg/Shader>
#include <osgText/Font>

#include <string>

namespace osgHelper
{

class IResourceManager : public osg::Referenced
{
public:
  IResourceManager() = default;
  virtual ~IResourceManager() = default;

  virtual std::string                 loadText(const std::string& resourceKey) = 0;
  virtual char*                       loadBinary(const std::string& resourceKey) = 0;
  virtual osg::ref_ptr<osg::Image>    loadImage(const std::string& resourceKey) = 0;
  virtual osg::ref_ptr<osgText::Font> loadFont(const std::string& resourceKey) = 0;
  virtual osg::ref_ptr<osg::Shader>   loadShader(const std::string& resourceKey, osg::Shader::Type type) = 0;

  virtual void setResourceLoader(const osg::ref_ptr<IResourceLoader>& loader) = 0;

  virtual void clearCacheResource(const std::string& resourceKey) = 0;
  virtual void clearCache() = 0;

};

}