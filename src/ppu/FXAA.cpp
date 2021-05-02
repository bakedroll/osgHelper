#include <osgHelper/ppu/FXAA.h>
#include <osgHelper/ppu/Shaders.h>

#include <osgHelper/ShaderFactory.h>

#include <osgPPU/ShaderAttribute.h>

namespace osgHelper::ppu
{

struct FXAA::Impl
{
  Impl(osgHelper::ioc::Injector& injector)
    : shaderFactory(injector.inject<osgHelper::ShaderFactory>())
    , resolution(osg::Vec2f(512.0f, 512.0f))
  {
  }

  osg::ref_ptr<osgHelper::ShaderFactory> shaderFactory;

  osg::ref_ptr<osgPPU::UnitInOut> unitFxaa;
  osg::Vec2f resolution;
  osg::ref_ptr<osgPPU::ShaderAttribute> shaderFxaa;
};

const std::string FXAA::Name = "fxaaEffect";

FXAA::FXAA(osgHelper::ioc::Injector& injector)
  : Effect()
  , m(new Impl(injector))
{
}

FXAA::~FXAA() = default;

std::string FXAA::getName() const
{
  return Name;
}

Effect::InitialUnitList FXAA::getInitialUnits() const
{
  return InitialUnitList();
}

osg::ref_ptr<osgPPU::Unit> FXAA::getResultUnit() const
{
  return m->unitFxaa;
}

Effect::InputToUniformList FXAA::getInputToUniform() const
{
  InputToUniformList list;

  InputToUniform ituBypass;
  ituBypass.name = "tex0";
  ituBypass.type = UnitType::OngoingColor;
  ituBypass.unit = m->unitFxaa;

  list.push_back(ituBypass);

  return list;
}

void FXAA::setResolution(const osg::Vec2f& resolution)
{
  m->resolution = resolution;

  if (m->shaderFxaa.valid())
  {
    updateResolutionUniforms();
  }
}

Effect::Status FXAA::initializeUnits(const osg::GL2Extensions* extensions)
{
  if (!extensions->isGpuShader4Supported)
  {
    return { InitResult::UnsupportedShaders, "GL_EXT_gpu_shader4 not supported" };
  }

  const auto shaderFxaaFp = m->shaderFactory->fromSourceText("ShaderFxaaFp", Shaders::ShaderFxaaFp, osg::Shader::FRAGMENT);
  const auto shaderFxaaVp = m->shaderFactory->fromSourceText("ShaderFxaaVp", Shaders::ShaderFxaaVp, osg::Shader::VERTEX);

  m->unitFxaa = new osgPPU::UnitInOut();
  {
    m->shaderFxaa = new osgPPU::ShaderAttribute();
    m->shaderFxaa->addShader(shaderFxaaFp);
    m->shaderFxaa->addShader(shaderFxaaVp);

    m->shaderFxaa->add("rt_w", osg::Uniform::FLOAT);
    m->shaderFxaa->add("rt_h", osg::Uniform::FLOAT);

    updateResolutionUniforms();
    
    m->unitFxaa->getOrCreateStateSet()->setAttributeAndModes(m->shaderFxaa);
  }

  return { InitResult::Initialized, "" };
}

void FXAA::onResizeViewport(const osg::Vec2f& resolution)
{
  setResolution(resolution);
}

void FXAA::updateResolutionUniforms()
{
  m->shaderFxaa->set("rt_w", m->resolution.x());
  m->shaderFxaa->set("rt_h", m->resolution.y());
}

}
