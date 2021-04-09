#include <osgHelper/ppu/HDR.h>
#include <osgHelper/ppu/Shaders.h>
#include <osgHelper/ShaderFactory.h>
#include <osgHelper/SimulationCallback.h>

#include <osgDB/ReadFile>

#include <osgPPU/UnitInMipmapOut.h>
#include <osgPPU/ShaderAttribute.h>
#include <osgPPU/UnitInResampleOut.h>

namespace osgHelper
{
namespace ppu
{

  class HighDynamicRangeEffectCallback : public osgHelper::SimulationCallback
  {
  public:
    HighDynamicRangeEffectCallback(const osg::ref_ptr<osgPPU::UnitInOut>& unitAdaptedLuminance)
      : osgHelper::SimulationCallback()
      , unitAdaptedLuminance(unitAdaptedLuminance)
    {
    }

    void action(const SimulationData& data) override
    {
      unitAdaptedLuminance->getOrCreateStateSet()
              ->getOrCreateUniform("invFrameTime", osg::Uniform::FLOAT)
              ->set(static_cast<float>(data.timeDelta));
    }

  private:
    osg::ref_ptr<osgPPU::UnitInOut> unitAdaptedLuminance;

  };

  struct HDR::Impl
  {
    explicit Impl(osgHelper::ioc::Injector& injector)
      : shaderFactory(injector.inject<osgHelper::ShaderFactory>())
      , midGrey(5.0f)
      , hdrBlurSigma(4.0f)
      , hdrBlurRadius(5.0f)
      , glareFactor(7.5f)
      , adaptFactor(0.03f)
      , minLuminance(0.2f)
      , maxLuminance(5.0f)
    {
      
    }

    osg::ref_ptr<osgHelper::ShaderFactory> shaderFactory;

    float midGrey;
    float hdrBlurSigma;
    float hdrBlurRadius;
    float glareFactor;
    float adaptFactor;
    float minLuminance;
    float maxLuminance;

    osg::ref_ptr<osgPPU::UnitInResampleOut> unitResample;
    osg::ref_ptr<osgPPU::UnitInOut> unitHdr;

    osg::ref_ptr<osgPPU::ShaderAttribute> shaderBrightpass;
    osg::ref_ptr<osgPPU::ShaderAttribute> shaderHdr;
    osg::ref_ptr<osgPPU::ShaderAttribute> shaderGaussX;
    osg::ref_ptr<osgPPU::ShaderAttribute> shaderGaussY;
    osg::ref_ptr<osgPPU::ShaderAttribute> shaderAdapted;
  };

  const std::string HDR::Name = "hdrEffect";

  HDR::HDR(osgHelper::ioc::Injector& injector)
    : Effect()
    , m(new Impl(injector))
  {

  }

  HDR::~HDR() = default;

  std::string HDR::getName() const
  {
    return Name;
  }

  Effect::InitialUnitList HDR::getInitialUnits() const
  {
    InitialUnitList list;

    InitialUnit unitResample;
    unitResample.type = UnitType::OngoingColor;
    unitResample.unit = m->unitResample;

    list.push_back(unitResample);

    return list;
  }

  osg::ref_ptr<osgPPU::Unit> HDR::getResultUnit() const
  {
    return m->unitHdr;
  }

  Effect::InputToUniformList HDR::getInputToUniform() const
  {
    InputToUniformList list;

    InputToUniform ituBypass;
    ituBypass.name = "hdrInput";
    ituBypass.type = UnitType::OngoingColor;
    ituBypass.unit = m->unitHdr;

    list.push_back(ituBypass);

    return list;
  }

  void HDR::setMidGrey(float midGrey)
  {
    m->midGrey = midGrey;

    if (isInitialized())
    {
      m->shaderBrightpass->set("g_fMiddleGray", m->midGrey);
      m->shaderHdr->set("g_fMiddleGray", m->midGrey);
    }

    printf("Set midGrey to %f\n", m->midGrey);
  }

  void HDR::setBlurSigma(float blurSigma)
  {
    m->hdrBlurSigma = blurSigma;

    if (isInitialized())
    {
      m->shaderGaussX->set("sigma", m->hdrBlurSigma);
      m->shaderGaussY->set("sigma", m->hdrBlurSigma);
    }

    printf("Set blurSigma to %f\n", m->hdrBlurSigma);
  }

  void HDR::setBlurRadius(float blurRadius)
  {
    m->hdrBlurRadius = blurRadius;

    if (isInitialized())
    {
      m->shaderGaussX->set("radius", m->hdrBlurRadius);
      m->shaderGaussY->set("radius", m->hdrBlurRadius);
    }

    printf("Set blurRadius to %f\n", m->hdrBlurRadius);
  }

  void HDR::setGlareFactor(float glareFactor)
  {
    m->glareFactor = glareFactor;

    if (isInitialized())
    {
      m->shaderHdr->set("fBlurFactor", m->glareFactor);
    }

    printf("Set glateFactor to %f\n", m->glareFactor);
  }

  void HDR::setAdaptFactor(float adaptFactor)
  {
    m->adaptFactor = adaptFactor;

    if (isInitialized())
    {
      m->shaderAdapted->set("adaptScaleFactor", m->adaptFactor);
    }

    printf("Set adaptedFactor to %f\n", m->adaptFactor);
  }

  void HDR::setMinLuminance(float minLuminance)
  {
    m->minLuminance = minLuminance;

    if (isInitialized())
    {
      m->shaderAdapted->set("minLuminance", m->minLuminance);
    }

    printf("Set minLuminance to %f\n", m->minLuminance);
  }

  void HDR::setMaxLuminance(float maxLuminance)
  {
    m->maxLuminance = maxLuminance;

    if (isInitialized())
    {
      m->shaderAdapted->set("maxLuminance", m->maxLuminance);
    }

    printf("Set maxLuminance to %f\n", m->maxLuminance);
  }

  float HDR::getMidGrey() const
  {
    return m->midGrey;
  }

  float HDR::getBlurSigma() const
  {
    return m->hdrBlurSigma;
  }

  float HDR::getBlurRadius() const
  {
    return m->hdrBlurRadius;
  }

  float HDR::getGlareFactor() const
  {
    return m->glareFactor;
  }

  float HDR::getAdaptFactor() const
  {
    return m->adaptFactor;
  }

  float HDR::getMinLuminance() const
  {
    return m->minLuminance;
  }

  float HDR::getMaxLuminance() const
  {
    return m->maxLuminance;
  }

  void HDR::initializeUnits()
  {
    const auto shaderBrightpassFp =
            m->shaderFactory->fromSourceText("ShaderBrightpassFp", Shaders::ShaderBrightpassFp, osg::Shader::FRAGMENT);
    const auto shaderGaussConvolution1dxFp = m->shaderFactory->fromSourceText(
            "ShaderGaussConvolution1dxFp", Shaders::ShaderGaussConvolution1dxFp, osg::Shader::FRAGMENT);
    const auto shaderGaussConvolution1dyFp = m->shaderFactory->fromSourceText(
            "ShaderGaussConvolution1dyFp", Shaders::ShaderGaussConvolution1dyFp, osg::Shader::FRAGMENT);
    const auto shaderGaussConvolutionVp = m->shaderFactory->fromSourceText(
            "ShaderGaussConvolutionVp", Shaders::ShaderGaussConvolutionVp, osg::Shader::VERTEX);
    const auto shaderLuminanceAdaptedFp = m->shaderFactory->fromSourceText(
            "ShaderLuminanceAdaptedFp", Shaders::ShaderLuminanceAdaptedFp, osg::Shader::FRAGMENT);
    const auto shaderLuminanceFp =
            m->shaderFactory->fromSourceText("ShaderLuminanceFp", Shaders::ShaderLuminanceFp, osg::Shader::FRAGMENT);
    const auto shaderLuminanceMipmapFp = m->shaderFactory->fromSourceText(
            "ShaderLuminanceMipmapFp", Shaders::ShaderLuminanceMipmapFp, osg::Shader::FRAGMENT);
    const auto shaderTonemapHdrFp =
            m->shaderFactory->fromSourceText("ShaderTonemapHdrFp", Shaders::ShaderTonemapHdrFp, osg::Shader::FRAGMENT);

    m->unitResample = new osgPPU::UnitInResampleOut();
    {
      m->unitResample->setFactorX(0.25);
      m->unitResample->setFactorY(0.25);
    }

    auto pixelLuminance = new osgPPU::UnitInOut();
    {
        auto lumShader = new osgPPU::ShaderAttribute();
      lumShader->addShader(shaderLuminanceFp);
      lumShader->add("texUnit0", osg::Uniform::SAMPLER_2D);
      lumShader->set("texUnit0", 0);

      pixelLuminance->getOrCreateStateSet()->setAttributeAndModes(lumShader);
    }
    m->unitResample->addChild(pixelLuminance);

    auto sceneLuminance = new osgPPU::UnitInMipmapOut();
    {
        auto lumShaderMipmap = new osgPPU::ShaderAttribute();
      lumShaderMipmap->addShader(shaderLuminanceMipmapFp);

      lumShaderMipmap->add("texUnit0", osg::Uniform::SAMPLER_2D);
      lumShaderMipmap->set("texUnit0", 0);

      sceneLuminance->getOrCreateStateSet()->setAttributeAndModes(lumShaderMipmap);
      sceneLuminance->setGenerateMipmapForInputTexture(0);
    }
    pixelLuminance->addChild(sceneLuminance);

    osgPPU::Unit* brightpass = new osgPPU::UnitInOut();
    {
      m->shaderBrightpass = new osgPPU::ShaderAttribute();
      m->shaderBrightpass->addShader(shaderBrightpassFp);

      m->shaderBrightpass->add("g_fMiddleGray", osg::Uniform::FLOAT);
      m->shaderBrightpass->set("g_fMiddleGray", m->midGrey);
      brightpass->getOrCreateStateSet()->setAttributeAndModes(m->shaderBrightpass);

      brightpass->setInputToUniform(m->unitResample, "hdrInput", true);
      brightpass->setInputToUniform(sceneLuminance, "lumInput", true);
    }

    auto blurx = new osgPPU::UnitInOut();
    auto blury = new osgPPU::UnitInOut();
    {
      m->shaderGaussX = new osgPPU::ShaderAttribute();
      m->shaderGaussX->addShader(shaderGaussConvolutionVp);
      m->shaderGaussX->addShader(shaderGaussConvolution1dxFp);
      m->shaderGaussX->add("sigma", osg::Uniform::FLOAT);
      m->shaderGaussX->add("radius", osg::Uniform::FLOAT);
      m->shaderGaussX->add("texUnit0", osg::Uniform::SAMPLER_2D);

      m->shaderGaussX->set("sigma", m->hdrBlurSigma);
      m->shaderGaussX->set("radius", m->hdrBlurRadius);
      m->shaderGaussX->set("texUnit0", 0);

      blurx->getOrCreateStateSet()->setAttributeAndModes(m->shaderGaussX);

      m->shaderGaussY = new osgPPU::ShaderAttribute();
      m->shaderGaussY->addShader(shaderGaussConvolutionVp);
      m->shaderGaussY->addShader(shaderGaussConvolution1dyFp);
      m->shaderGaussY->add("sigma", osg::Uniform::FLOAT);
      m->shaderGaussY->add("radius", osg::Uniform::FLOAT);
      m->shaderGaussY->add("texUnit0", osg::Uniform::SAMPLER_2D);

      m->shaderGaussY->set("sigma", m->hdrBlurSigma);
      m->shaderGaussY->set("radius", m->hdrBlurRadius);
      m->shaderGaussY->set("texUnit0", 0);

      blury->getOrCreateStateSet()->setAttributeAndModes(m->shaderGaussY);
    }

    brightpass->addChild(blurx);
    blurx->addChild(blury);

    m->unitHdr = new osgPPU::UnitInOut();
    {
      m->shaderHdr = new osgPPU::ShaderAttribute();
      m->shaderHdr->addShader(shaderTonemapHdrFp);

      m->shaderHdr->add("fBlurFactor", osg::Uniform::FLOAT);
      m->shaderHdr->add("g_fMiddleGray", osg::Uniform::FLOAT);

      m->shaderHdr->set("fBlurFactor", m->glareFactor);
      m->shaderHdr->set("g_fMiddleGray", m->midGrey);

      m->unitHdr->getOrCreateStateSet()->setAttributeAndModes(m->shaderHdr);
      m->unitHdr->setInputTextureIndexForViewportReference(-1);

      m->unitHdr->setInputToUniform(blury, "blurInput", true);
      m->unitHdr->setInputToUniform(sceneLuminance, "lumInput", true);
    }

    auto adaptedLuminance = new osgPPU::UnitInOut();
    {
      m->shaderAdapted = new osgPPU::ShaderAttribute();
      m->shaderAdapted->addShader(shaderLuminanceAdaptedFp);
      m->shaderAdapted->add("texLuminance", osg::Uniform::SAMPLER_2D);
      m->shaderAdapted->set("texLuminance", 0);
      m->shaderAdapted->add("texAdaptedLuminance", osg::Uniform::SAMPLER_2D);
      m->shaderAdapted->set("texAdaptedLuminance", 1);

      m->shaderAdapted->add("maxLuminance", osg::Uniform::FLOAT);
      m->shaderAdapted->add("minLuminance", osg::Uniform::FLOAT);
      m->shaderAdapted->add("adaptScaleFactor", osg::Uniform::FLOAT);

      adaptedLuminance->getOrCreateStateSet()->getOrCreateUniform("invFrameTime", osg::Uniform::FLOAT);

      m->shaderAdapted->set("maxLuminance", m->maxLuminance);
      m->shaderAdapted->set("minLuminance", m->minLuminance);
      m->shaderAdapted->set("adaptScaleFactor", m->adaptFactor);

      adaptedLuminance->getOrCreateStateSet()->setAttributeAndModes(m->shaderAdapted);
      adaptedLuminance->setViewport(new osg::Viewport(0, 0, 1, 1));
      adaptedLuminance->setInputTextureIndexForViewportReference(-1);
    }

    sceneLuminance->addChild(adaptedLuminance);

    auto adaptedlumCopy = new osgPPU::UnitInOut();
    adaptedlumCopy->addChild(adaptedLuminance);

    adaptedLuminance->addChild(adaptedlumCopy);
    adaptedLuminance->addChild(brightpass);
    brightpass->setInputToUniform(adaptedLuminance, "texAdaptedLuminance");

    adaptedLuminance->addChild(m->unitHdr);
    m->unitHdr->setInputToUniform(adaptedLuminance, "texAdaptedLuminance");

    adaptedLuminance->setUpdateCallback(new HighDynamicRangeEffectCallback(adaptedLuminance));
  }
}
}