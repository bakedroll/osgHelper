#include <osgHelper/ShaderBlueprint.h>

#include <utilsLib/Utils.h>

namespace osgHelper
{

struct ShaderBlueprint::Impl
{
  Impl()
    : version(120)
    , type(osg::Shader::FRAGMENT)
  {}

  ~Impl() {}

  int version;
  osg::Shader::Type type;
  std::vector<std::string> extensions;
  std::vector<std::string> modules;

};

ShaderBlueprint::ShaderBlueprint()
  : IShaderBlueprint()
  , m(new Impl())
{
}

ShaderBlueprint::~ShaderBlueprint() = default;

osg::ref_ptr<IShaderBlueprint> ShaderBlueprint::version(int shaderVersion)
{
  m->version = shaderVersion;
  return this;
}

osg::ref_ptr<IShaderBlueprint> ShaderBlueprint::type(osg::Shader::Type shaderType)
{
  m->type = shaderType;
  return this;
}

osg::ref_ptr<IShaderBlueprint> ShaderBlueprint::extension(const std::string& extName)
{
  m->extensions.push_back(extName);
  return this;
}

osg::ref_ptr<IShaderBlueprint> ShaderBlueprint::module(const std::string& module)
{
  m->modules.push_back(module);
  return this;
}

osg::ref_ptr<osg::Shader> ShaderBlueprint::build()
{
  assert_return(!m->modules.empty(), nullptr);

  osg::ref_ptr<osg::Shader> shader = new osg::Shader(m->type);
  std::string code = "#version " + std::to_string(m->version) + "\n";

  for (auto& extension : m->extensions)
  {
    code += ("#extension " + extension + " : enable\n");
  }

  for (auto& module : m->modules)
  {
    code += (module + "\n");
  }

  shader->setShaderSource(code);
  return shader;
}

}