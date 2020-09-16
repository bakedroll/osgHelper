#include <osgHelper/ppu/FXAA.h>
#include <osgHelper/ppu/Shaders.h>

#include <osgHelper/ShaderFactory.h>

#include <osgPPU/ShaderAttribute.h>

namespace osgHelper
{
namespace ppu
{
  class FastApproximateAntiAliasingEffectCallback : public osgGA::GUIEventHandler
  {
  public:
    FastApproximateAntiAliasingEffectCallback(const osg::ref_ptr<osgPPU::ShaderAttribute>& shaderFxaa,
                                              const osg::Vec2f&                            resolution)
      : osgGA::GUIEventHandler()
    , shaderFxaa(shaderFxaa)
    , resolution(resolution)
    {
    }

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override
    {
      switch (ea.getEventType())
      {
      case osgGA::GUIEventAdapter::RESIZE:
      {
        const auto width = ea.getWindowWidth();
        const auto height = ea.getWindowHeight();

        if (width != static_cast<int>(resolution.x()) || height != static_cast<int>(resolution.y()))
        {
          resolution = osg::Vec2f(static_cast<float>(width), static_cast<float>(height));

          shaderFxaa->set("rt_w", resolution.x());
          shaderFxaa->set("rt_h", resolution.y());

          return true;
        }
      }
      default:
        break;
      }

      return false;
    }

  private:
    osg::ref_ptr<osgPPU::ShaderAttribute> shaderFxaa;
    osg::Vec2f resolution;
  };

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

  void FXAA::setResolution(osg::Vec2f resolution)
  {
    m->resolution = resolution;

    if (m->shaderFxaa.valid())
    {
      m->shaderFxaa->set("rt_w", m->resolution.x());
      m->shaderFxaa->set("rt_h", m->resolution.y());
    }
  }

  void FXAA::initializeUnits()
  {
    auto shaderFxaaFp = m->shaderFactory->fromSourceText("ShaderFxaaFp", Shaders::ShaderFxaaFp, osg::Shader::FRAGMENT);
    auto shaderFxaaVp = m->shaderFactory->fromSourceText("ShaderFxaaVp", Shaders::ShaderFxaaVp, osg::Shader::VERTEX);

    m->unitFxaa = new osgPPU::UnitInOut();
    {
      m->shaderFxaa = new osgPPU::ShaderAttribute();
      m->shaderFxaa->addShader(shaderFxaaFp);
      m->shaderFxaa->addShader(shaderFxaaVp);

      m->shaderFxaa->add("rt_w", osg::Uniform::FLOAT);
      m->shaderFxaa->add("rt_h", osg::Uniform::FLOAT);

      m->shaderFxaa->set("rt_w", m->resolution.x());
      m->shaderFxaa->set("rt_h", m->resolution.y());
      
      m->unitFxaa->getOrCreateStateSet()->setAttributeAndModes(m->shaderFxaa);

      m->unitFxaa->setEventCallback(new FastApproximateAntiAliasingEffectCallback(m->shaderFxaa, m->resolution));
    }
  }

}
}