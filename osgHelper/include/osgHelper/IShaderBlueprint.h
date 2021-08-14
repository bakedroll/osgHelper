#pragma once

#include <osg/Referenced>
#include <osg/Shader>

namespace osgHelper
{

class IShaderBlueprint : public osg::Referenced
{
public:
  IShaderBlueprint() = default;
  virtual ~IShaderBlueprint() = default;

  virtual osg::ref_ptr<IShaderBlueprint> version(int shaderVersion) = 0;
  virtual osg::ref_ptr<IShaderBlueprint> type(osg::Shader::Type shaderType) = 0;
  virtual osg::ref_ptr<IShaderBlueprint> extension(const std::string& extName) = 0;
  virtual osg::ref_ptr<IShaderBlueprint> module(const std::string& module) = 0;

  virtual osg::ref_ptr<osg::Shader> build() = 0;

};

}