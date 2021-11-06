#pragma once

#include <osgHelper/IShaderBlueprint.h>

#include <memory>

namespace osgHelper
{

class ShaderBlueprint : public IShaderBlueprint
{
public:
  ShaderBlueprint();
  ~ShaderBlueprint() override;

  osg::ref_ptr<IShaderBlueprint> version(int shaderVersion) override;
  osg::ref_ptr<IShaderBlueprint> type(osg::Shader::Type shaderType) override;
  osg::ref_ptr<IShaderBlueprint> extension(const std::string& extName) override;
  osg::ref_ptr<IShaderBlueprint> module(const std::string& module) override;

  osg::ref_ptr<osg::Shader> build() override;

private:
  struct Impl;
  std::unique_ptr<Impl> m;

};

}