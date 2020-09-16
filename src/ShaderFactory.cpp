#include <osgHelper/ShaderFactory.h>

#include <osgHelper/Macros.h>

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
    : osg::Referenced()
    , m(new Impl())
  {
  }

  ShaderBlueprint::~ShaderBlueprint()
  {
  }

  osg::ref_ptr<ShaderBlueprint> ShaderBlueprint::version(int shaderVersion)
  {
    m->version = shaderVersion;
    return this;
  }

  osg::ref_ptr<ShaderBlueprint> ShaderBlueprint::type(osg::Shader::Type shaderType)
  {
    m->type = shaderType;
    return this;
  }

  osg::ref_ptr<ShaderBlueprint> ShaderBlueprint::extension(const std::string& extName)
  {
    m->extensions.push_back(extName);
    return this;
  }

  osg::ref_ptr<ShaderBlueprint> ShaderBlueprint::module(const std::string& module)
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
      code += ("#extension " + extension + " : enable\n");

    for (auto& module : m->modules)
      code += (module + "\n");

    shader->setShaderSource(code);
    return shader;
  }

  ShaderFactory::ShaderFactory(ioc::Injector& injector)
    : AbstractFactory<ShaderBlueprint>(injector)
  {
  }

  ShaderFactory::~ShaderFactory()
  {
  }

  osg::ref_ptr<osg::Shader> ShaderFactory::fromSourceText(std::string key, std::string source, osg::Shader::Type type)
  {
    ShaderDictionary::iterator it = m_shaderCache.find(key);

    if (it != m_shaderCache.end())
    {
      return it->second;
    }

    osg::ref_ptr<osg::Shader> shader = new osg::Shader(type);
    shader->setShaderSource(source);

    m_shaderCache.insert(ShaderDictionary::value_type(key, shader));

    return shader;
  }
}