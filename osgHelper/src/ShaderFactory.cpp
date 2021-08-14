#include <osgHelper/ShaderFactory.h>
#include <osgHelper/ShaderBlueprint.h>

namespace osgHelper
{

ShaderFactory::ShaderFactory(ioc::Injector& injector)
  : IShaderFactory()
{
}

ShaderFactory::~ShaderFactory() = default;

osg::ref_ptr<osg::Shader> ShaderFactory::fromSourceText(const std::string& key, const std::string& source,
                                                        osg::Shader::Type type)
{
  const auto it = m_shaderCache.find(key);

  if (it != m_shaderCache.end())
  {
    return it->second;
  }

  osg::ref_ptr<osg::Shader> shader = new osg::Shader(type);
  shader->setShaderSource(source);

  m_shaderCache.insert(ShaderDictionary::value_type(key, shader));

  return shader;
}

osg::ref_ptr<IShaderBlueprint> ShaderFactory::make() const
{
  return new ShaderBlueprint();
}

}
