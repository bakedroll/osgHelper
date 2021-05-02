#include <osgHelper/ppu/DOF.h>
#include <osgHelper/ppu/Shaders.h>
#include <osgHelper/ShaderFactory.h>

#include <osgPPU/UnitInResampleOut.h>
#include <osgPPU/ShaderAttribute.h>

namespace osgHelper
{
namespace ppu
{

  struct DOF::Impl
  {
    Impl(osgHelper::ioc::Injector& injector)
      : shaderFactory(injector.inject<osgHelper::ShaderFactory>())
      , gaussSigma(1.5f)
      , gaussRadius(5.0f)
      , focalLength(10.0f)
      , focalRange(8.0f)
      , zNear(1.0f)
      , zFar(1000.0f)
    {
      
    }

    osg::ref_ptr<osgHelper::ShaderFactory> shaderFactory;

    float gaussSigma;
    float gaussRadius;
    float focalLength;
    float focalRange;
    float zNear;
    float zFar;

    osg::ref_ptr<osgPPU::ShaderAttribute> shaderDof;
    osg::ref_ptr<osgPPU::ShaderAttribute> shaderGaussX;
    osg::ref_ptr<osgPPU::ShaderAttribute> shaderGaussY;
    osg::ref_ptr<osgPPU::UnitInResampleOut> unitResampleLight;
    osg::ref_ptr<osgPPU::UnitInResampleOut> unitResampleStrong;
    osg::ref_ptr<osgPPU::UnitInOut> unitDof;
  };

  const std::string DOF::Name = "dofEffect";

  DOF::DOF(osgHelper::ioc::Injector& injector)
    : Effect()
    , m(new Impl(injector))
  {

  }

  DOF::~DOF() = default;

  std::string DOF::getName() const
  {
    return Name;
  }

  Effect::InitialUnitList DOF::getInitialUnits() const
  {
    InitialUnitList list;

    InitialUnit unitResampleLight;
    unitResampleLight.type = UnitType::OngoingColor;
    unitResampleLight.unit = m->unitResampleLight;

    InitialUnit unitResampleStrong;
    unitResampleStrong.type = UnitType::OngoingColor;
    unitResampleStrong.unit = m->unitResampleStrong;

    list.push_back(unitResampleLight);
    list.push_back(unitResampleStrong);

    return list;
  }

  osg::ref_ptr<osgPPU::Unit> DOF::getResultUnit() const
  {
    return m->unitDof;
  }

  Effect::InputToUniformList DOF::getInputToUniform() const
  {
    InputToUniformList list;

    InputToUniform ituBypass;
    ituBypass.name = "texColorMap";
    ituBypass.type = UnitType::OngoingColor;
    ituBypass.unit = m->unitDof;

    InputToUniform ituDepthBypass;
    ituDepthBypass.name = "texDepthMap";
    ituDepthBypass.type = UnitType::BypassDepth;
    ituDepthBypass.unit = m->unitDof;

    list.push_back(ituBypass);
    list.push_back(ituDepthBypass);

    return list;
  }

  void DOF::setGaussSigma(float gaussSigma)
  {
    m->gaussSigma = gaussSigma;

    if (isInitialized())
    {
      m->shaderGaussX->set("sigma", m->gaussSigma);
      m->shaderGaussY->set("sigma", m->gaussSigma);
    }
  }

  void DOF::setGaussRadius(float gaussRadius)
  {
    m->gaussRadius = gaussRadius;

    if (isInitialized())
    {
      m->shaderGaussX->set("radius", m->gaussRadius);
      m->shaderGaussY->set("radius", m->gaussRadius);
    }
  }

  void DOF::setFocalLength(float focalLength)
  {
    m->focalLength = focalLength;

    if (isInitialized())
    {
      m->shaderDof->set("focalLength", m->focalLength);
    }
  }

  void DOF::setFocalRange(float focalRange)
  {
    m->focalRange = focalRange;

    if (isInitialized())
    {
      m->shaderDof->set("focalRange", m->focalRange);
    }
  }

  void DOF::setZNear(float zNear)
  {
    m->zNear = zNear;

    if (isInitialized())
    {
      m->shaderDof->set("zNear", m->zNear);
    }
  }

  void DOF::setZFar(float zFar)
  {
    m->zFar = zFar;

    if (isInitialized())
    {
      m->shaderDof->set("zFar", m->zFar);
    }
  }

  float DOF::getGaussSigma() const
  {
    return m->gaussSigma;
  }

  float DOF::getGaussRadius() const
  {
    return m->gaussRadius;
  }

  float DOF::getFocalLength() const
  {
    return m->focalLength;
  }

  float DOF::getFocalRange() const
  {
    return m->focalRange;
  }

  float DOF::getZNear() const
  {
    return m->zNear;
  }

  float DOF::getZFar() const
  {
    return m->zFar;
  }

  Effect::Status DOF::initializeUnits(const osg::GL2Extensions* extensions)
  {
    auto shaderDepthOfFieldFp = m->shaderFactory->fromSourceText(
            "ShaderDepthOfFieldFp", Shaders::ShaderDepthOfFieldFp, osg::Shader::FRAGMENT);
    auto shaderGaussConvolution1dxFp = m->shaderFactory->fromSourceText(
            "ShaderGaussConvolution1dxFp", Shaders::ShaderGaussConvolution1dxFp,
            osg::Shader::FRAGMENT);
    auto shaderGaussConvolution1dyFp = m->shaderFactory->fromSourceText(
            "ShaderGaussConvolution1dyFp", Shaders::ShaderGaussConvolution1dyFp,
            osg::Shader::FRAGMENT);
    auto shaderGaussConvolutionVp = m->shaderFactory->fromSourceText(
            "ShaderGaussConvolutionVp", Shaders::ShaderGaussConvolutionVp, osg::Shader::VERTEX);

    m->unitResampleLight = new osgPPU::UnitInResampleOut();
    {
      m->unitResampleLight->setFactorX(0.5);
      m->unitResampleLight->setFactorY(0.5);
    }

    m->shaderGaussX = new osgPPU::ShaderAttribute();
    m->shaderGaussY = new osgPPU::ShaderAttribute();
    {
      m->shaderGaussX->addShader(shaderGaussConvolutionVp);
      m->shaderGaussX->addShader(shaderGaussConvolution1dxFp);

      m->shaderGaussX->add("sigma", osg::Uniform::FLOAT);
      m->shaderGaussX->add("radius", osg::Uniform::FLOAT);
      m->shaderGaussX->add("texUnit0", osg::Uniform::SAMPLER_2D);

      m->shaderGaussX->set("sigma", m->gaussSigma);
      m->shaderGaussX->set("radius", m->gaussRadius);
      m->shaderGaussX->set("texUnit0", 0);

      m->shaderGaussY->addShader(shaderGaussConvolutionVp);
      m->shaderGaussY->addShader(shaderGaussConvolution1dyFp);

      m->shaderGaussY->add("sigma", osg::Uniform::FLOAT);
      m->shaderGaussY->add("radius", osg::Uniform::FLOAT);
      m->shaderGaussY->add("texUnit0", osg::Uniform::SAMPLER_2D);

      m->shaderGaussY->set("sigma", m->gaussSigma);
      m->shaderGaussY->set("radius", m->gaussRadius);
      m->shaderGaussY->set("texUnit0", 0);
    }

    auto blurxlight = new osgPPU::UnitInOut();
    auto blurylight = new osgPPU::UnitInOut();
    {
      blurxlight->getOrCreateStateSet()->setAttributeAndModes(m->shaderGaussX);
      blurylight->getOrCreateStateSet()->setAttributeAndModes(m->shaderGaussY);
    }
    m->unitResampleLight->addChild(blurxlight);
    blurxlight->addChild(blurylight);


    m->unitResampleStrong = new osgPPU::UnitInResampleOut();
    {
      m->unitResampleStrong->setFactorX(0.25f);
      m->unitResampleStrong->setFactorY(0.25f);
    }

    auto blurxstrong = new osgPPU::UnitInOut();
    auto blurystrong = new osgPPU::UnitInOut();
    {
      blurxstrong->getOrCreateStateSet()->setAttributeAndModes(m->shaderGaussX);
      blurystrong->getOrCreateStateSet()->setAttributeAndModes(m->shaderGaussY);
    }
    m->unitResampleStrong->addChild(blurxstrong);
    blurxstrong->addChild(blurystrong);

    m->unitDof = new osgPPU::UnitInOut();
    {
      m->shaderDof = new osgPPU::ShaderAttribute();
      m->shaderDof->addShader(shaderDepthOfFieldFp);

      m->shaderDof->add("focalLength", osg::Uniform::FLOAT);
      m->shaderDof->add("focalRange", osg::Uniform::FLOAT);
      m->shaderDof->add("zNear", osg::Uniform::FLOAT);
      m->shaderDof->add("zFar", osg::Uniform::FLOAT);

      m->shaderDof->set("focalLength", m->focalLength);
      m->shaderDof->set("focalRange", m->focalRange);
      m->shaderDof->set("zNear", m->zNear);
      m->shaderDof->set("zFar", m->zFar);

      m->unitDof->getOrCreateStateSet()->setAttributeAndModes(m->shaderDof);
      m->unitDof->setInputTextureIndexForViewportReference(0);

      m->unitDof->setInputToUniform(blurylight, "texBlurredColorMap", true);
      m->unitDof->setInputToUniform(blurystrong, "texStrongBlurredColorMap", true);
    }

    return { InitResult::Initialized, "" };
  }

}
}