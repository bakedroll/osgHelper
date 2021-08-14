#pragma once

#include <osgHelper/IShaderFactory.h>
#include <osgHelper/ioc/Injector.h>

namespace osgHelper
{

class ShaderFactory : public IShaderFactory
{
public:
  explicit ShaderFactory(ioc::Injector& injector);
  ~ShaderFactory() override;

	osg::ref_ptr<osg::Shader> fromSourceText(const std::string& key, const std::string& source, osg::Shader::Type type) override;
	osg::ref_ptr<IShaderBlueprint> make() const override;

private:
	using ShaderDictionary = std::map<std::string, osg::ref_ptr<osg::Shader>>;

	ShaderDictionary m_shaderCache;
};

}