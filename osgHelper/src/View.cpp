#include <osgHelper/View.h>
#include <osgHelper/Helper.h>

#include <utilsLib/Utils.h>

#include <osg/ClampColor>
#include <osg/Texture2D>
#include <osg/GL2Extensions>

#include <osgViewer/Renderer>

#include <osgPPU/Unit.h>
#include <osgPPU/UnitInOut.h>
#include <osgPPU/Processor.h>
#include <osgPPU/UnitBypass.h>
#include <osgPPU/UnitDepthbufferBypass.h>
#include <osgPPU/UnitCamera.h>
#include <osgPPU/UnitCameraAttachmentBypass.h>

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

namespace osgHelper
{

osg::Camera::BufferComponent getCameraBufferComponent(View::TextureComponent textureComponent)
{
  return (textureComponent == View::TextureComponent::ColorBuffer)
    ? osg::Camera::BufferComponent::COLOR_BUFFER
    : osg::Camera::BufferComponent::DEPTH_BUFFER;
}

std::string getEffectNotSupportedMessage(const std::string& effectName)
{
  return "Post processing effect '" + effectName + "' is not supported";
}

osg::ref_ptr<osg::Texture2D> createCameraRenderTexture(const osg::ref_ptr<osgHelper::Camera>& camera,
                                                       const osg::Vec2f& resolution,
                                                       osg::Camera::BufferComponent component,
                                                       osg::Texture::FilterMode filterMode)
{
  auto texture = new osg::Texture2D();

  texture->setDataVariance(osg::Texture::DataVariance::DYNAMIC);
  texture->setTextureSize(static_cast<int>(resolution.x()), static_cast<int>(resolution.y()));
  texture->setFilter(osg::Texture2D::MIN_FILTER, filterMode);
  texture->setFilter(osg::Texture2D::MAG_FILTER, filterMode);
  texture->setResizeNonPowerOfTwoHint(false);
  texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
  texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);

  switch (component)
  {
  case osg::Camera::BufferComponent::COLOR_BUFFER:
    texture->setInternalFormat(GL_RGBA16F_ARB);
    texture->setSourceFormat(GL_RGBA);
    texture->setSourceType(GL_FLOAT);
    break;

  case osg::Camera::BufferComponent::DEPTH_BUFFER:
    texture->setInternalFormat(GL_DEPTH_COMPONENT);
    break;

  default:
    break;
  }

  camera->attach(component, texture);

  return texture;
}

void applyStateSetRemderTextures(const osg::ref_ptr<osg::StateSet>& stateSet, const View::SlaveRenderTextures& textures)
{
  for (const auto& component : textures)
  {
    stateSet->setTextureAttributeAndModes(
      component.first == View::TextureComponent::ColorBuffer ? 0 : 1, component.second);
  }
}

struct View::Impl
{
  Impl()
    : sceneGraph(new osg::Group())
    , cameras(utilsLib::underlying(CameraType::_Count))
    , isResolutionInitialized(false)
    , isPipelineDirty(false)
  {
  }

  struct RenderTexture
  {
      osg::ref_ptr<osg::Texture2D> texture;
      osg::ref_ptr<osgPPU::Unit>   bypassUnit;
  };

  struct PostProcessingState
  {
    osg::ref_ptr<ppu::Effect> effect;
    bool                      isEnabled    = true;
    bool                      isIntegrated = false;
  };

  struct RenderTextureUnitSinkData
  {
    osg::ref_ptr<Camera>              camera;
    ppu::RenderTextureUnitSink        sink;
    osg::ref_ptr<osgPPU::UnitCamera>  unitCamera;
    osg::ref_ptr<osgPPU::UnitCameraAttachmentBypass> unitCameraAttachmentBypass;
    osg::ref_ptr<osg::Texture2D>      texture;
  };

  using RenderTextureDictionary          = std::map<int, RenderTexture>;
  using PostProcessingStateDictionary    = std::map<std::string, PostProcessingState>;
  using RenderTextureUnitSinkList        = std::vector<RenderTextureUnitSinkData>;
  using RTTSlaveCameraScreenQuadDataList = std::vector<RTTSlaveCameraScreenQuadData>;

  osg::ref_ptr<osg::Group>          sceneGraph;
  std::vector<osg::ref_ptr<Camera>> cameras;

  std::vector<std::weak_ptr<ResizeCallback>> resizeCallbacks;

  osg::ref_ptr<osg::StateSet> screenStateSet;

  osg::Vec2f resolution;
  bool       isResolutionInitialized;
  bool       isPipelineDirty;

  osg::ref_ptr<osgPPU::Processor> processor;
  osg::ref_ptr<osg::ClampColor>   clampColor;

  osg::ref_ptr<osgPPU::Unit>      lastUnit;
  osg::ref_ptr<osgPPU::UnitInOut> unitOutput;

  PostProcessingStateDictionary ppeDictionary;
  RenderTextureDictionary       renderTextures;

  RenderTextureUnitSinkList renderTextureUnitSinks;
  RTTSlaveCameraScreenQuadDataList rttScreenQuadData;

  void setupCameras()
  {
    const auto sceneCamera  = new Camera(Camera::ProjectionMode::Perspective);
    sceneCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER);
    sceneCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    sceneCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    sceneCamera->setRenderOrder(osg::Camera::NESTED_RENDER);

    const auto screenCamera = new Camera(Camera::ProjectionMode::Ortho2D);
    screenCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER);
    screenCamera->setRenderOrder(osg::Camera::RenderOrder::POST_RENDER);

    cameras[utilsLib::underlying(CameraType::Scene)]  = sceneCamera;
    cameras[utilsLib::underlying(CameraType::Screen)] = screenCamera;

    const auto& texture = getOrCreateRenderTexture(osg::Camera::COLOR_BUFFER).texture;

    const auto geo = osg::createTexturedQuadGeometry(osg::Vec3f(-1.0f, -1.0f, 0.0f), osg::Vec3f(2.0f, 0.0f, 0.0f),
                                                     osg::Vec3f(0.0f, 2.0f, 0.0f));

    auto geode = new osg::Geode();
    geode->addDrawable(geo);

    screenStateSet = screenCamera->getOrCreateStateSet();
    screenStateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
    screenStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    screenStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    screenStateSet->setMode(GL_BLEND, osg::StateAttribute::OFF);

    screenCamera->addChild(geode);
  }

  osg::ref_ptr<osg::Texture2D> createAndAttachRenderTexture(osg::Camera::BufferComponent bufferComponent)
  {
    const auto sceneCamera = cameras.at(utilsLib::underlying(CameraType::Scene));
    return createCameraRenderTexture(sceneCamera, resolution, bufferComponent, osg::Texture::LINEAR);
  }

  RenderTexture getOrCreateRenderTexture(osg::Camera::BufferComponent bufferComponent,
                                         UpdateMode                   mode = UpdateMode::Keep)
  {
    auto it = renderTextures.find(bufferComponent);
    if (it != renderTextures.end())
    {
      auto& renderTexture = it->second;

      if (mode == UpdateMode::Recreate)
      {
        cameras[utilsLib::underlying(CameraType::Scene)]->detach(bufferComponent);
        renderTexture.texture = createAndAttachRenderTexture(bufferComponent);
      }

      return renderTexture;
    }

    RenderTexture rt;
    rt.texture = createAndAttachRenderTexture(bufferComponent);

    renderTextures[bufferComponent] = rt;

    return rt;
  }

  osg::ref_ptr<osgPPU::Unit> getBypassUnit(osg::Camera::BufferComponent bufferComponent)
  {
    auto renderTexture = getOrCreateRenderTexture(bufferComponent);

    if (!renderTexture.bypassUnit.valid())
    {
      switch (bufferComponent)
      {
        case osg::Camera::COLOR_BUFFER:
        {
          renderTexture.bypassUnit = new osgPPU::UnitBypass();
          processor->addChild(renderTexture.bypassUnit);

          break;
        }
        case osg::Camera::DEPTH_BUFFER:
        {
          renderTexture.bypassUnit = new osgPPU::UnitDepthbufferBypass();
          processor->addChild(renderTexture.bypassUnit);

          break;
        }
        default:
          break;
          ;
      }

      renderTextures[bufferComponent] = renderTexture;
    }

    return renderTexture.bypassUnit;
  }

  osg::ref_ptr<osgPPU::Unit> getLastUnit(bool reset = false)
  {
    if (!lastUnit.valid() || reset)
    {
      lastUnit = getBypassUnit(osg::Camera::COLOR_BUFFER);
    }

    return lastUnit;
  }

  osg::ref_ptr<osgPPU::Unit> unitForType(ppu::Effect::UnitType type)
  {
    switch (type)
    {
      case ppu::Effect::UnitType::BypassColor:
        return getBypassUnit(osg::Camera::COLOR_BUFFER);
      case ppu::Effect::UnitType::BypassDepth:
        return getBypassUnit(osg::Camera::DEPTH_BUFFER);
      case ppu::Effect::UnitType::OngoingColor:
        return getLastUnit();
      default:
        break;
    }

    return nullptr;
  }

};

View::View()
  : osgViewer::View()
  , m(new Impl())
{
  m->setupCameras();

  setCamera(getCamera(CameraType::Scene));
  setSceneData(m->sceneGraph);

  //m->sceneGraph->addChild(getCamera(CameraType::Screen));
  addSlave(getCamera(CameraType::Screen), false);
}

View::~View() = default;

void View::updateResolution(const osg::Vec2f& resolution, float pixelRatio)
{
  const auto width  = static_cast<int>(resolution.x());
  const auto height = static_cast<int>(resolution.y());

  updateCameraViewports(0, 0, width, height, pixelRatio);
  updateCameraRenderTextures(UpdateMode::Recreate);

  if (m->processor.valid())
  {
    osgPPU::Camera::resizeViewport(0, 0, width, height, getCamera(CameraType::Scene));

    for (const auto& sink : m->renderTextureUnitSinks)
    {
      osgPPU::Camera::resizeViewport(0, 0, width, height, sink.camera);
    }
    for (const auto& screenQuad : m->rttScreenQuadData)
    {
      osgPPU::Camera::resizeViewport(0, 0, width, height, screenQuad.slaveCameraData.camera);
    }

    m->processor->onViewportChange();
  }

  const auto initialResolutionUpdate = (m->resolution.length2() == 0.0f);

  m->resolution              = resolution;
  m->isResolutionInitialized = true;

  if (m->isPipelineDirty || initialResolutionUpdate)
  {
    alterPipelineState([](){}, UpdateMode::Recreate);
    m->isPipelineDirty = false;
  }

  for (auto& effect : m->ppeDictionary)
  {
    effect.second.effect->onResizeViewport(resolution);
  }

  auto it = m->resizeCallbacks.begin();
  while (it != m->resizeCallbacks.end())
  {
    auto ptr = it->lock();
    if (ptr)
    {
      ptr->func(osg::Vec2i(m->resolution.x(), m->resolution.y()));
      ++it;
    }
    else
    {
      it = m->resizeCallbacks.erase(it);
    }
  }
}

void View::updateCameraViewports(int x, int y, int width, int height, float pixelRatio) const
{
  const auto scaledWidth  = static_cast<int>(pixelRatio * static_cast<float>(width));
  const auto scaledHeight = static_cast<int>(pixelRatio * static_cast<float>(height));

  const auto viewport = new osg::Viewport(x, y, scaledWidth, scaledHeight);

  for (const auto& camera : m->cameras)
  {
    camera->setViewport(viewport);
    camera->updateResolution(osg::Vec2i(width, height));
  }

  for (const auto& sink : m->renderTextureUnitSinks)
  {
    sink.camera->setViewport(viewport);
    sink.camera->updateResolution(osg::Vec2i(width, height));
  }

  for (const auto& screenQuad : m->rttScreenQuadData)
  {
    screenQuad.slaveCameraData.camera->setViewport(viewport);
    screenQuad.slaveCameraData.camera->updateResolution(osg::Vec2i(width, height));
  }
}

void View::setClampColorEnabled(bool enabled)
{
  if (!m->clampColor.valid())
  {
    m->clampColor = new osg::ClampColor();
    m->clampColor->setClampVertexColor(GL_FALSE);
    m->clampColor->setClampFragmentColor(GL_FALSE);
    m->clampColor->setClampReadColor(GL_FALSE);
  }

  getCamera(CameraType::Scene)->getOrCreateStateSet()->setAttribute(m->clampColor,
      enabled ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
}

osg::ref_ptr<osg::Group> View::getRootGroup() const
{
  return m->sceneGraph;
}

osg::ref_ptr<osgHelper::Camera> View::getCamera(CameraType type) const
{
  return m->cameras[utilsLib::underlying(type)];
}

void View::addPostProcessingEffect(const osg::ref_ptr<ppu::Effect>& ppe, bool enabled, const std::string& name)
{
  alterPipelineState([this, ppe, enabled, name]()
  {
    Impl::PostProcessingState pps;
    pps.effect  = ppe;
    pps.isEnabled = enabled;

    m->ppeDictionary[name.empty() ? ppe->getName() : name] = pps;
  }, enabled ? UpdateMode::Recreate : UpdateMode::Keep);
}

void View::setPostProcessingEffectEnabled(const std::string& ppeName, bool enabled)
{
  if (m->ppeDictionary.count(ppeName) == 0)
  {
    UTILS_LOG_WARN("Post processing effect '" + ppeName + "' not found");
    assert_return(false);
  }

  auto& ppe = m->ppeDictionary[ppeName];

  if ((ppe.isEnabled != enabled) && ppe.effect->isSupported())
  {
    alterPipelineState([&ppe, enabled]()
    {
      ppe.isEnabled = enabled;
    });
  }

  if (ppe.effect->isSupported())
  {
    UTILS_LOG_DEBUG("Post processing effect '" + ppeName + "': " + (enabled ? "enabled" : "disabled"));
  }
  else if (enabled)
  {
    if (ppe.isEnabled)
    {
      ppe.isEnabled = false;
    }
    else
    {
      UTILS_LOG_WARN(getEffectNotSupportedMessage(ppeName) + ". Could not enable.");
    }
  }
}

osg::ref_ptr<ppu::Effect> View::getPostProcessingEffect(const std::string& ppeName) const
{
  if (m->ppeDictionary.count(ppeName) == 0)
  {
    return nullptr;
  }

  return m->ppeDictionary[ppeName].effect;
}

osg::Vec2f View::getResolution() const
{
  assert(m->isResolutionInitialized);
  return m->resolution;
}

bool View::getPostProcessingEffectEnabled(const std::string& ppeName) const
{
  return (m->ppeDictionary.count(ppeName) > 0) ? m->ppeDictionary[ppeName].isEnabled : false;
}

bool View::hasPostProcessingEffect(const std::string& ppeName) const
{
  return (m->ppeDictionary.count(ppeName) > 0);
}

void View::cleanUp()
{
  setSceneData(nullptr);
  m->sceneGraph->removeChildren(0, m->sceneGraph->getNumChildren());

  m->ppeDictionary.clear();
}

std::shared_ptr<View::ResizeCallback> View::registerResizeCallback(const ResizeCallbackFunc& func)
{
  auto callback = std::make_shared<ResizeCallback>();
  callback->func = func;
  callback->func(osg::Vec2i(m->resolution.x(), m->resolution.y()));

  m->resizeCallbacks.emplace_back(callback);

  return callback;
}

View::RTTSlaveCameraData View::createRenderToTextureSlaveCamera(const osg::Vec2i& resolution, TextureComponent components, SlaveCameraMode mode)
{
  RTTSlaveCameraData data;
  data.camera = createSlaveCamera(resolution, false, mode);

  const auto addCameraRenderTextureComponentIfInMask = [this, &data, components](TextureComponent component)
  {
    const auto osgComponent = (component == TextureComponent::DepthBuffer)
      ? osg::Camera::BufferComponent::DEPTH_BUFFER
      : osg::Camera::COLOR_BUFFER;

    if (utilsLib::bitmask_has(components, component))
    {
      data.textures[component] = createCameraRenderTexture(
        data.camera, m->resolution, osgComponent, osg::Texture::NEAREST);
    }
    else
    {
      data.camera->detach(osgComponent);
    }
  };

  addCameraRenderTextureComponentIfInMask(TextureComponent::ColorBuffer);
  addCameraRenderTextureComponentIfInMask(TextureComponent::DepthBuffer);

  return data;
}

osg::ref_ptr<Camera> View::createRenderToTextureSlaveCameraToUnitSink(const ppu::RenderTextureUnitSink& sink, SlaveCameraMode mode)
{
  auto camera = createSlaveCamera(osg::Vec2i(m->resolution.x(), m->resolution.y()), true, mode);

  const auto texture = createCameraRenderTexture(camera, m->resolution, sink.getBufferComponent(), osg::Texture::NEAREST);

  Impl::RenderTextureUnitSinkData data
  {
    camera, sink,
    new osgPPU::UnitCamera(),
    new osgPPU::UnitCameraAttachmentBypass(),
    texture
  };

  data.unitCamera->setCamera(camera);
  data.unitCameraAttachmentBypass->setBufferComponent(data.sink.getBufferComponent());

  data.unitCamera->addChild(data.unitCameraAttachmentBypass);

  const auto e = data.sink.getEffect();
  m->renderTextureUnitSinks.emplace_back(data);

  data.sink.getUnitSink()->setInputToUniform(data.unitCameraAttachmentBypass, data.sink.getUniformName(), true);

  if (m->processor)
  {
    for (const auto& effect : m->ppeDictionary)
    {
      if (effect.second.effect == e && effect.second.isEnabled && effect.second.isIntegrated)
      {
        m->processor->addChild(data.unitCamera);
      }
    } 
  }

  return camera;
}

View::RTTSlaveCameraScreenQuadData View::createRenderToTextureSlaveCameraToScreenQuad(TextureComponent components,
                                                                                      SlaveCameraMode mode)
{
  RTTSlaveCameraScreenQuadData data;
  data.slaveCameraData = createRenderToTextureSlaveCamera(osg::Vec2i(m->resolution.x(), m->resolution.y()), components, mode);
  data.screenQuadNode  = getCamera(CameraType::Scene)->createScreenQuad();

  auto quadStateSet = data.screenQuadNode->getOrCreateStateSet();
  quadStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
  quadStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
  quadStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
  quadStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
  quadStateSet->setRenderBinDetails(10, "RenderBin");
  quadStateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);

  applyStateSetRemderTextures(quadStateSet, data.slaveCameraData.textures);

  m->rttScreenQuadData.emplace_back(data);

  return data;
}


void View::initializePipelineProcessor()
{
  const auto& sceneCamera = getCamera(CameraType::Scene);

  m->processor = new osgPPU::Processor();
  m->processor->setCamera(sceneCamera);

  m->sceneGraph->addChild(m->processor);

  osgPPU::Camera::resizeViewport(0, 0, m->resolution.x(), m->resolution.y(), sceneCamera);
  m->processor->onViewportChange();
}

void View::assemblePipeline()
{
  m->lastUnit = m->getLastUnit(true);

  const auto state = getCamera(CameraType::Scene)->getGraphicsContext()->getState();

  if (state)
  {
    const auto extensions = osg::GL2Extensions::Get(state->getContextID(), false);

    for (auto& it : m->ppeDictionary)
    {
      it.second.isIntegrated = it.second.isEnabled;

      if (!it.second.isEnabled || !it.second.effect->isSupported())
      {
        continue;
      }

      const auto& ppe = it.second.effect;

      const auto status = ppe->initialize(extensions);
      if (status.result != ppu::Effect::InitResult::Initialized)
      {
        UTILS_LOG_WARN(getEffectNotSupportedMessage(ppe->getName()) + ": " + status.message);
        continue;
      }

      auto initialUnits = ppe->getInitialUnits();
      for (const auto& unit : initialUnits)
      {
        m->unitForType(unit.type)->addChild(unit.unit);
      }

      auto inputToUniformList = ppe->getInputToUniform();
      for (const auto& itou : inputToUniformList)
      {
        itou.unit->setInputToUniform(m->unitForType(itou.type), itou.name, true);
      }

      for (const auto& data : m->renderTextureUnitSinks)
      {
        data.sink.getUnitSink()->setInputToUniform(data.unitCameraAttachmentBypass, data.sink.getUniformName(), true);
      }

      m->lastUnit = ppe->getResultUnit();
    }
  }
  else
  {
    UTILS_LOG_WARN("Could not build postprocessing pipeline due to invalid OpenGL state");
  }

  m->lastUnit->addChild(m->unitOutput);

  updateCameraRenderTextures();
  m->processor->dirtyUnitSubgraph();
}

void View::disassemblePipeline()
{
  if (!m->processor.valid())
  {
    initializePipelineProcessor();
  }

  m->lastUnit = m->getLastUnit(true);

  for (auto& it : m->ppeDictionary)
  {
    if (!it.second.isEnabled || !it.second.isIntegrated || !it.second.effect->isSupported())
    {
        continue;
    }

    auto& ppe = it.second.effect;

    auto initialUnits = ppe->getInitialUnits();
    for (const auto& unit : initialUnits)
    {
        m->unitForType(unit.type)->removeChild(unit.unit);
    }

    auto inputToUniformList = ppe->getInputToUniform();
    for (const auto& itou : inputToUniformList)
    {
        m->unitForType(itou.type)->removeChild(itou.unit);
    }

    for (const auto& data : m->renderTextureUnitSinks)
    {
      data.unitCameraAttachmentBypass->removeChild(data.sink.getUnitSink());
    }

    m->lastUnit            = ppe->getResultUnit();
    it.second.isIntegrated = false;
  }

  if (m->unitOutput.valid())
  {
      m->lastUnit->removeChild(m->unitOutput);
  }

  m->unitOutput = new osgPPU::UnitInOut();
  m->unitOutput->setInputTextureIndexForViewportReference(-1);
  m->screenStateSet->setTextureAttributeAndModes(0, m->unitOutput->getOrCreateOutputTexture(0),
                                                 osg::StateAttribute::ON);

}

void View::alterPipelineState(const std::function<void()>& func, UpdateMode mode)
{
  const auto canSetupPipeline = (m->isResolutionInitialized && (mode == UpdateMode::Recreate));
  m->isPipelineDirty          = (!m->isResolutionInitialized && (mode == UpdateMode::Recreate));

  if (!canSetupPipeline)
  {
    func();
    return;
  }

  disassemblePipeline();
  func();
  assemblePipeline();

  updateCameraRenderTextures();
  m->processor->dirtyUnitSubgraph();
}

void View::updateCameraRenderTextures(UpdateMode mode)
{
  for (const auto& it : m->renderTextures)
  {
    const auto& tex = m->getOrCreateRenderTexture(static_cast<osg::Camera::BufferComponent>(it.first), mode).texture;
    if ((it.first == osg::Camera::COLOR_BUFFER) && !m->processor.valid())
    {
      m->screenStateSet->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
    }
  }

  if (mode == UpdateMode::Recreate)
  {
    for (auto& data : m->renderTextureUnitSinks)
    {
      data.camera->detach(data.sink.getBufferComponent());

      data.texture = createCameraRenderTexture(data.camera, m->resolution,
        data.sink.getBufferComponent(),
        osg::Texture::NEAREST);

      data.sink.getUnitSink()->setInputToUniform(data.unitCameraAttachmentBypass, data.sink.getUniformName());
    }

    for (auto& data : m->rttScreenQuadData)
    {
      const auto& camera = data.slaveCameraData.camera;
      for (auto& texture : data.slaveCameraData.textures)
      {
        const auto component = getCameraBufferComponent(texture.first);
        camera->detach(component);

        texture.second = createCameraRenderTexture(
          camera, m->resolution, component, osg::Texture::NEAREST);
      }

      applyStateSetRemderTextures(data.screenQuadNode->getOrCreateStateSet(),
        data.slaveCameraData.textures);
    }
  }

  auto renderer = dynamic_cast<osgViewer::Renderer*>(getCamera(CameraType::Scene)->getRenderer());
  renderer->getSceneView(0)->getRenderStage()->setCameraRequiresSetUp(true);
  renderer->getSceneView(0)->getRenderStage()->setFrameBufferObject(nullptr);
}

osg::ref_ptr<Camera> View::createSlaveCamera(const osg::Vec2i& resolution, bool inheritViewport, SlaveCameraMode mode)
{
  auto camera = new Camera();

  camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER);
  camera->setRenderOrder(osg::Camera::PRE_RENDER);

  const auto refCamera = *m->cameras.begin();
  camera->setGraphicsContext(refCamera->getGraphicsContext());

  addSlave(camera, mode == SlaveCameraMode::UseMasterSceneData);

  if (inheritViewport)
  {
    camera->setViewport(refCamera->getViewport());
  }
  else
  {
    camera->setViewport(0, 0, resolution.x(), resolution.y());
  }

  camera->updateResolution(resolution);

  return camera;
}

}
