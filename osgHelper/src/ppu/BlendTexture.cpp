#include <osgHelper/ppu/BlendTexture.h>
#include <osgHelper/ppu/Shaders.h>

#include <osgHelper/IShaderFactory.h>

#include <osgPPU/ShaderAttribute.h>

namespace osgHelper::ppu
{

  struct BlendTexture::Impl
  {
    Impl(osgHelper::ioc::Injector& injector)
      : shaderFactory(injector.inject<osgHelper::IShaderFactory>())
      , resolution(osg::Vec2f(512.0f, 512.0f))
    {
    }

    osg::ref_ptr<osgHelper::IShaderFactory> shaderFactory;

    osg::ref_ptr<osgPPU::UnitInOut> unitBlend;

    osg::Vec2f resolution;
    osg::ref_ptr<osgPPU::ShaderAttribute> shaderBlend;
  };

  const std::string BlendTexture::Name = "BlendTextureEffect";

  BlendTexture::BlendTexture(osgHelper::ioc::Injector& injector)
    : Effect()
    , m(new Impl(injector))
  {
  }

  BlendTexture::~BlendTexture() = default;

  std::string BlendTexture::getName() const
  {
    return Name;
  }

  Effect::InitialUnitList BlendTexture::getInitialUnits() const
  {
    return InitialUnitList();
  }

  osg::ref_ptr<osgPPU::Unit> BlendTexture::getResultUnit() const
  {
    return m->unitBlend;
  }

  Effect::InputToUniformList BlendTexture::getInputToUniform() const
  {
    InputToUniformList list;

    InputToUniform ituBypass;
    ituBypass.name = "tex0";
    ituBypass.type = UnitType::OngoingColor;
    ituBypass.unit = m->unitBlend;

    list.push_back(ituBypass);

    return list;
  }

  RenderTextureUnitSink BlendTexture::getBlendTextureSink()
  {
    return RenderTextureUnitSink(this, m->unitBlend, "blendTex");
  }

  void BlendTexture::setResolution(const osg::Vec2f& resolution)
  {
    m->resolution = resolution;

    if (m->shaderBlend.valid())
    {
      updateResolutionUniforms();
    }
  }

  Effect::Status BlendTexture::initializeUnits(const osg::GL2Extensions* extensions)
  {
    const auto shaderVert = new osg::Shader(osg::Shader::Type::VERTEX,
      "void main()" \
      "{" \
      "	 gl_Position = ftransform();" \
      "	 gl_TexCoord[0] = gl_MultiTexCoord0;" \
      "}");

    const auto shaderFrag = new osg::Shader(osg::Shader::Type::FRAGMENT,
      "uniform sampler2D tex0;" \
      "uniform sampler2D blendTex;" \
      "" \
      "void main()" \
	    "{" \
	    "	 vec2 uv = gl_TexCoord[0].st;" \
      "  if (texture2D(blendTex, uv).a == 0.0)" \
      "  {" \
      "    gl_FragColor = texture2D(tex0, uv);" \
      "    return;" \
      "  }" \
	    "	 gl_FragColor = texture2D(blendTex, uv);" \
	    "}");

    m->unitBlend = new osgPPU::UnitInOut();
    
    m->shaderBlend = new osgPPU::ShaderAttribute();
    m->shaderBlend->addShader(shaderVert);
    m->shaderBlend->addShader(shaderFrag);

    //updateResolutionUniforms();

    m->unitBlend->getOrCreateStateSet()->setAttributeAndModes(m->shaderBlend);
    

    return { InitResult::Initialized, "" };
  }

  void BlendTexture::onResizeViewport(const osg::Vec2f& resolution)
  {
    setResolution(resolution);
  }

  void BlendTexture::updateResolutionUniforms()
  {
    m->shaderBlend->set("rt_w", m->resolution.x());
    m->shaderBlend->set("rt_h", m->resolution.y());
  }

}
