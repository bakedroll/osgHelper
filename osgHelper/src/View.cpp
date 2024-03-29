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

osg::Camera::BufferComponent textureComponentToOsgBufferComponent(View::TextureComponent component)
{
  return (component == View::TextureComponent::DepthBuffer)
    ? osg::Camera::BufferComponent::DEPTH_BUFFER
    : osg::Camera::COLOR_BUFFER;
}

View::TextureComponent OsgBufferComponentToTextureComponent(osg::Camera::BufferComponent component)
{
  switch (component)
  {
  case osg::Camera::BufferComponent::DEPTH_BUFFER:
    return View::TextureComponent::DepthBuffer;
  case osg::Camera::BufferComponent::COLOR_BUFFER:
    return View::TextureComponent::ColorBuffer;
  default:
    break;
  }

  assert(false);
  return View::TextureComponent::Unknown;
}

osg::Vec2i scaledVec2i(const osg::Vec2i& vec, float factor)
{
  return osg::Vec2i(static_cast<int>(vec.x() * factor),
    static_cast<int>(vec.y() * factor));
}

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
                                                       const osg::Vec2i& resolution,
                                                       osg::Camera::BufferComponent component,
                                                       osg::Texture::FilterMode filterMode)
{
  auto texture = new osg::Texture2D();

  texture->setDataVariance(osg::Texture::DataVariance::DYNAMIC);
  texture->setTextureSize(resolution.x(), resolution.y());
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

void applyStateSetRenderTextures(const osg::ref_ptr<osg::StateSet>& stateSet, const View::SlaveRenderTextures& textures)
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
    bool isEnabled    = true;
    bool isIntegrated = false;
  };

  struct RenderTextureUnitSinkData
  {
    ScreenBoundRTTData screenBoundData;
    ppu::RenderTextureUnitSink sink;
    osg::ref_ptr<osgPPU::UnitCamera> unitCamera;
    osg::ref_ptr<osgPPU::UnitCameraAttachmentBypass> unitCameraAttachmentBypass;
  };

  using RenderTextureDictionary = std::map<int, RenderTexture>;
  using PostProcessingStateDictionary = std::map<std::string, PostProcessingState>;
  using RTTSlaveCameraDataList = std::vector<RTTSlaveCameraData>;
  using RenderTextureUnitSinkList = std::vector<RenderTextureUnitSinkData>;
  using RTTSlaveCameraScreenQuadDataList = std::vector<RTTSlaveCameraScreenQuadData>;

  osg::ref_ptr<osg::Group> sceneGraph;
  std::vector<osg::ref_ptr<osgHelper::Camera>> cameras;

  struct SlaveCameraData
  {
    SlaveCameraMode mode;
    ViewportMode viewportMode;
  };

  std::map<osg::ref_ptr<osgHelper::Camera>, SlaveCameraData> slaveCameraModi;

  std::vector<std::weak_ptr<ResizeCallback>> resizeCallbacks;

  osg::ref_ptr<osg::StateSet> screenStateSet;

  osg::Vec2i resolution;
  bool isResolutionInitialized;
  bool isPipelineDirty;

  osg::ref_ptr<osgPPU::Processor> processor;
  osg::ref_ptr<osg::ClampColor> clampColor;

  osg::ref_ptr<osgPPU::Unit> lastUnit;
  osg::ref_ptr<osgPPU::UnitInOut> unitOutput;

  PostProcessingStateDictionary ppeDictionary;
  RenderTextureDictionary renderTextures;

  RTTSlaveCameraDataList rttSlaveCameraData;
  RenderTextureUnitSinkList renderTextureUnitSinks;
  RTTSlaveCameraScreenQuadDataList rttScreenQuadData;

  void setupCameras()
  {
    const auto sceneCamera  = new osgHelper::Camera(osgHelper::Camera::ProjectionMode::Perspective);
    sceneCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER);
    sceneCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    sceneCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    sceneCamera->setRenderOrder(osg::Camera::NESTED_RENDER);

    const auto screenCamera = new osgHelper::Camera(osgHelper::Camera::ProjectionMode::Ortho2D);
    screenCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER);
    screenCamera->setRenderOrder(osg::Camera::RenderOrder::POST_RENDER);

    cameras[utilsLib::underlying(CameraType::Scene)]  = sceneCamera;
    cameras[utilsLib::underlying(CameraType::Screen)] = screenCamera;

    const auto& texture = getOrCreateRenderTexture(osg::Camera::COLOR_BUFFER).texture;

    const auto geo = osg::createTexturedQuadGeometry(osg::Vec3f(-1.0f, -1.0f, 0.0f), osg::Vec3f(2.0f, 0.0f, 0.0f),
                                                     osg::Vec3f(0.0f, 2.0f, 0.0f));

    auto geode = new osg::Geode();
    geode->addDrawable(geo);

    screenStateSet = geo->getOrCreateStateSet();
    screenStateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

    auto camStateSet = screenCamera->getOrCreateStateSet();
    camStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    camStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    camStateSet->setMode(GL_BLEND, osg::StateAttribute::OFF);

    screenCamera->addChild(geode);
  }

  osg::ref_ptr<osg::Texture2D> createAndAttachRenderTexture(osg::Camera::BufferComponent bufferComponent)
  {
    const auto sceneCamera = cameras.at(utilsLib::underlying(CameraType::Scene));
    return createCameraRenderTexture(sceneCamera, resolution, bufferComponent, osg::Texture::LINEAR);
  }

  RenderTexture getOrCreateRenderTexture(
    osg::Camera::BufferComponent bufferComponent,
    UpdateMode mode = UpdateMode::Keep)
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

void View::updateResolution(const osg::Vec2i& resolution, float pixelRatio)
{
  const auto width  = resolution.x();
  const auto height = resolution.y();

  updateCameraViewports(0, 0, width, height, pixelRatio);
  updateCameraRenderTextures(UpdateMode::Recreate);

  if (m->processor.valid())
  {
    osgPPU::Camera::resizeViewport(0, 0, width, height, getCamera(CameraType::Scene));

    for (const auto& data : m->rttSlaveCameraData)
    {
      if (data.camera->getRenderer() != nullptr)
      {
        osgPPU::Camera::resizeViewport(0, 0, width, height, data.camera);
      }
    }

    m->processor->onViewportChange();
  }

  const auto initialResolutionUpdate = ((m->resolution.x() == 0) && (m->resolution.y() == 0));

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
      ptr->func(m->resolution);
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
  const osg::Vec2i resolution(width, height);

  for (const auto& camera : m->cameras)
  {
    camera->setViewport(viewport);
    camera->updateResolution(resolution);
  }

  for (const auto& elem : m->slaveCameraModi)
  {
    if (elem.second.viewportMode == ViewportMode::InheritViewport)
    {
      elem.first->setViewport(viewport);
      elem.first->updateResolution(resolution);
    }
  }

  for (const auto& data : m->rttSlaveCameraData)
  {
    data.camera->setViewport(viewport);
    data.camera->updateResolution(resolution);
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

void View::setSlaveCameraEnabled(const osg::ref_ptr<osgHelper::Camera>& camera, bool enabled)
{
  if (m->slaveCameraModi.count(camera) == 0)
  {
    UTILS_LOG_WARN("Camera is not a registered slave camera");
    return;
  }

  auto pos = -1;
  for (auto i=0U; i<getNumSlaves(); i++)
  {
    if (getSlave(i)._camera == camera)
    {
      pos = i;
      break;
    }
  }

  if (enabled && (pos == -1))
  {
    addSlave(camera,
      m->slaveCameraModi.at(camera).mode == SlaveCameraMode::UseMasterSceneData);
  }
  else if (!enabled && (pos >= 0))
  {
    removeSlave(pos);
    camera->setRenderer(nullptr);
  }
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

osg::Vec2i View::getResolution() const
{
  assert(m->isResolutionInitialized);
  return m->resolution;
}

bool View::isResolutionInitialized() const
{
  return m->isResolutionInitialized;
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

std::shared_ptr<View::ResizeCallback> View::registerResizeCallback(const ResizeCallbackFunc& func, bool callNow)
{
  auto callback = std::make_shared<ResizeCallback>();
  callback->func = func;
  if (callNow)
  {
    callback->func(m->resolution);
  }

  m->resizeCallbacks.emplace_back(callback);

  return callback;
}

View::RTTSlaveCameraData View::createRenderToTextureSlaveCamera(
  TextureComponent components, osg::Camera::RenderOrder renderOrder, float textureScale, SlaveCameraMode mode)
{
  const auto resolution = scaledVec2i(m->resolution, textureScale);
  RTTSlaveCameraData data;
  data.camera = createSlaveCamera(mode, osg::Camera::FRAME_BUFFER_OBJECT, renderOrder, ViewportMode::FixedViewport, resolution);

  const auto addCameraRenderTextureComponentIfInMask = [this, &data, &resolution, components](TextureComponent component)
  {
    const auto osgComponent = textureComponentToOsgBufferComponent(component);

    if (utilsLib::bitmask_has(components, component))
    {
      data.textures[component] = createCameraRenderTexture(
        data.camera, resolution, osgComponent, osg::Texture::NEAREST);
    }
    else
    {
      data.camera->detach(osgComponent);
    }
  };

  addCameraRenderTextureComponentIfInMask(TextureComponent::ColorBuffer);
  addCameraRenderTextureComponentIfInMask(TextureComponent::DepthBuffer);

  m->rttSlaveCameraData.emplace_back(data);
  return data;
}

void View::removeRenderToTextureSlaveCamera(const osg::ref_ptr<osgHelper::Camera>& camera)
{
  for (auto it = m->rttSlaveCameraData.begin(); it != m->rttSlaveCameraData.end(); ++it)
  {
    if (it->camera == camera)
    {
      m->rttSlaveCameraData.erase(it);
      removeSlaveCamera(camera);
      return;
    }
  }

  UTILS_LOG_WARN("Could not remove render-to-texture slave camera");
}

osg::ref_ptr<osgHelper::Camera> View::createRenderToTextureSlaveCameraToUnitSink(
  const ppu::RenderTextureUnitSink& sink,
  osg::Camera::RenderOrder renderOrder,
  float textureScale, SlaveCameraMode mode)
{
  const auto rttData = createRenderToTextureSlaveCamera(
    OsgBufferComponentToTextureComponent(sink.getBufferComponent()),
    renderOrder,
    textureScale,
    mode);

  Impl::RenderTextureUnitSinkData data
  {
    { rttData, textureScale },
    sink,
    new osgPPU::UnitCamera(),
    new osgPPU::UnitCameraAttachmentBypass(),
  };

  data.unitCamera->setCamera(rttData.camera);
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

  return rttData.camera;
}

void View::removeRenderToTextureSlaveCameraToUnitSink(const osg::ref_ptr<osgHelper::Camera>& camera)
{
  for (auto it = m->renderTextureUnitSinks.begin(); it != m->renderTextureUnitSinks.end(); ++it)
  {
    if (it->screenBoundData.rttData.camera == camera)
    {
      if (m->processor && m->processor->containsNode(it->unitCamera))
      {
        m->processor->removeChild(it->unitCamera);
      }

      m->renderTextureUnitSinks.erase(it);
      removeSlaveCamera(camera);
      return;
    }
  }

  UTILS_LOG_WARN("Could not remove unit-sink slave camera");
}

View::RTTSlaveCameraScreenQuadData View::createRenderToTextureSlaveCameraToScreenQuad(TextureComponent components,
                                                                                      osg::Camera::RenderOrder renderOrder,
                                                                                      float textureScale,
                                                                                      SlaveCameraMode mode)
{
  const auto rttData = createRenderToTextureSlaveCamera(components, renderOrder, textureScale, mode);
  const auto screenQuadNode = getCamera(CameraType::Scene)->createScreenQuad();

  RTTSlaveCameraScreenQuadData data
  {
    { rttData, textureScale },
    screenQuadNode
  };

  auto quadStateSet = data.screenQuadNode->getOrCreateStateSet();
  quadStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
  quadStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
  quadStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
  quadStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
  quadStateSet->setRenderBinDetails(10, "RenderBin");
  quadStateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);

  applyStateSetRenderTextures(quadStateSet, data.screenBoundData.rttData.textures);

  m->rttScreenQuadData.emplace_back(data);

  return data;
}

void View::removeRenderToTextureSlaveCameraToScreenQuad(const osg::ref_ptr<osgHelper::Camera>& camera)
{
  for (auto it = m->rttScreenQuadData.begin(); it != m->rttScreenQuadData.end(); ++it)
  {
    if (it->screenBoundData.rttData.camera == camera)
    {
      m->rttScreenQuadData.erase(it);
      removeSlaveCamera(camera);
      return;
    }
  }

  UTILS_LOG_WARN("Could not remove screen-quad slave camera");
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
    const auto extensions = osg::GL2Extensions::Get(state->getContextID(), true);

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
    const auto updateRTTSlaveCameraData = [this](ScreenBoundRTTData& data)
    {
      const auto& camera = data.rttData.camera;
      for (auto& texture : data.rttData.textures)
      {
        const auto component = getCameraBufferComponent(texture.first);
        camera->detach(component);

        texture.second = createCameraRenderTexture(
          camera, scaledVec2i(m->resolution, data.textureScale), component, osg::Texture::NEAREST);
      }
    };

    for (auto& data : m->renderTextureUnitSinks)
    {
      updateRTTSlaveCameraData(data.screenBoundData);

      data.sink.getUnitSink()->setInputToUniform(data.unitCameraAttachmentBypass, data.sink.getUniformName());
    }

    for (auto& data : m->rttScreenQuadData)
    {
      updateRTTSlaveCameraData(data.screenBoundData);

      applyStateSetRenderTextures(data.screenQuadNode->getOrCreateStateSet(),
        data.screenBoundData.rttData.textures);
    }
  }

  auto renderer = dynamic_cast<osgViewer::Renderer*>(getCamera(CameraType::Scene)->getRenderer());
  renderer->getSceneView(0)->getRenderStage()->setCameraRequiresSetUp(true);
  renderer->getSceneView(0)->getRenderStage()->setFrameBufferObject(nullptr);
}

osg::ref_ptr<osgHelper::Camera> View::createSlaveCamera(SlaveCameraMode mode, osg::Camera::RenderTargetImplementation renderTargetImplementation,
                                             osg::Camera::RenderOrder renderOrder, ViewportMode viewportMode, const osg::Vec2i& resolution)
{
  auto camera = new osgHelper::Camera();

  camera->setRenderTargetImplementation(renderTargetImplementation, osg::Camera::FRAME_BUFFER);
  camera->setRenderOrder(renderOrder);

  const auto refCamera = *m->cameras.begin();
  camera->setGraphicsContext(refCamera->getGraphicsContext());

  m->slaveCameraModi[camera] = { mode, viewportMode };

  addSlave(camera, mode == SlaveCameraMode::UseMasterSceneData);

  if (viewportMode == ViewportMode::InheritViewport)
  {
    const auto viewport = refCamera->getViewport();
    camera->setViewport(viewport);
    camera->updateResolution(osg::Vec2i(viewport->width(), viewport->height()));
  }
  else
  {
    camera->setViewport(0, 0, resolution.x(), resolution.y());
    camera->updateResolution(resolution);
  }

  return camera;
}

void View::removeSlaveCamera(const osg::ref_ptr<osgHelper::Camera>& camera)
{
  setSlaveCameraEnabled(camera, false);
  const auto it = m->slaveCameraModi.find(camera);
  if (it != m->slaveCameraModi.end())
  {
    m->slaveCameraModi.erase(it);
  }
}

}
