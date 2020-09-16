#include <osgHelper/View.h>
#include <osgHelper/LogManager.h>
#include <osgHelper/Macros.h>

#include <osg/ClampColor>
#include <osg/Texture2D>

#include <osgViewer/Renderer>

#include <osgPPU/Unit.h>
#include <osgPPU/UnitInOut.h>
#include <osgPPU/Processor.h>

#include <osgDB/WriteFile>

#include <cassert>

namespace osgHelper
{
  struct View::Impl
  {
    Impl()
      : sceneGraph(new osg::Group())
      , resolutionInitialized(false)
      , ppuInitialized(false)
    {
    }

    //! Determines if the render texture should be recreated
    enum class UpdateTextureMode
    {
      Recreate, //!< recreate
      Keep      //!< do not recreate
    };

    struct RenderTexture
    {
      osg::ref_ptr<osg::Texture2D> texture;
      osg::ref_ptr<osgPPU::Unit>   bypassUnit;
    };

    struct PostProcessingState
    {
      osg::ref_ptr<ppu::Effect> effect;
      bool                      enabled = true;
    };

    using RenderTextureDictionary       = std::map<int, RenderTexture>;
    using PostProcessingStateDictionary = std::map<std::string, PostProcessingState>;

    osg::ref_ptr<osg::Group> sceneGraph;

    osg::Vec2f resolution;
    bool       resolutionInitialized;

    osg::ref_ptr<osgPPU::Processor> processor;
    osg::ref_ptr<osg::ClampColor>   clampColor;

    osg::ref_ptr<osgPPU::Unit>      lastUnit;
    osg::ref_ptr<osgPPU::UnitInOut> unitOutput;

    bool                          ppuInitialized;
    PostProcessingStateDictionary ppeDictionary;

    /*void resetPostProcessingEffects()
    {
      initializePPU();

      lastUnit = getLastUnit(true);

      for (const auto& it : ppeDictionary)
      {
        if (!it.second.enabled)
        {
          continue;
        }

        auto& ppe = it.second.effect;

        auto initialUnits = ppe->getInitialUnits();
        for (const auto& unit : initialUnits)
        {
          unitForType(unit.type)->removeChild(unit.unit);
        }

        auto inputToUniformList = ppe->getInputToUniform();
        for (const auto& itou : inputToUniformList)
        {
          unitForType(itou.type)->removeChild(itou.unit);
        }

        lastUnit = ppe->getResultUnit();
      }

      if (unitOutput.valid())
      {
        lastUnit->removeChild(unitOutput);
      }

      unitOutput = new osgPPU::UnitInOut();
      //unitOutput->setInputTextureIndexForViewportReference(-1);
      //screenStateSet->setTextureAttributeAndModes(0, unitOutput->getOrCreateOutputTexture(0), osg::StateAttribute::ON);
    }

    void setupPostProcessingEffects()
    {
      lastUnit = getLastUnit(true);

      for (const auto& it : ppeDictionary)
      {
        if (!it.second.enabled)
		    {
          continue;
		    }

        const auto& ppe = it.second.effect;

        ppe->initialize();

        auto initialUnits = ppe->getInitialUnits();
        for (const auto& unit : initialUnits)
        {
          unitForType(unit.type)->addChild(unit.unit);
        }

        auto inputToUniformList = ppe->getInputToUniform();
        for (const auto& itou : inputToUniformList)
        {
          itou.unit->setInputToUniform(unitForType(itou.type), itou.name, true);
        }

        lastUnit = ppe->getResultUnit();
      }

      lastUnit->addChild(unitOutput);

      // updateCameraRenderTextures();
      processor->dirtyUnitSubgraph();
    }

    void updateWindowRect(const osgViewer::ViewerBase::Windows& windows)
    {
      if (!resolutionInitialized)
      {
        return;
	    }

      osg::ref_ptr<osgViewer::GraphicsWindow> graphicsWindow = *windows.begin();

      if (!graphicsWindow.valid())
      {
        return;
	    }

      if (fullscreenEnabled)
      {
        auto screenWidth  = 0U;
        auto screenHeight = 0U;

        osg::GraphicsContext::getWindowingSystemInterface()->getScreenResolution(
                osg::GraphicsContext::ScreenIdentifier(0), screenWidth, screenHeight);

        graphicsWindow->setWindowDecoration(false);
        graphicsWindow->setWindowRectangle(0, 0, static_cast<int>(screenWidth), static_cast<int>(screenHeight));
      }
      else
      {
        graphicsWindow->setWindowDecoration(true);
        graphicsWindow->setWindowRectangle(
            static_cast<int>(windowRect.x()),
            static_cast<int>(windowRect.y()),
            static_cast<int>(windowRect.z()),
            static_cast<int>(windowRect.w()));
      }
    }*/

    /*void updateCameraRenderTextures(UpdateTextureMode mode = UpdateTextureMode::Keep)
    {
      auto renderer = dynamic_cast<osgViewer::Renderer*>(camera->getRenderer());
      renderer->getSceneView(0)->getRenderStage()->setCameraRequiresSetUp(true);
      renderer->getSceneView(0)->getRenderStage()->setFrameBufferObject(nullptr);
    }*/

    void initializePPU()
    {
      if (ppuInitialized)
	    {
        return;
	    }

      processor = new osgPPU::Processor();
      //processor->setCamera(getCamera());

      sceneGraph->addChild(processor);

      //osgPPU::Camera::resizeViewport(0, 0, resolution.x(), resolution.y(), camera);
      processor->onViewportChange();

      ppuInitialized = true;
    }
  };

  View::View()
    : osgViewer::View()
    , m(new Impl())
  {
    setSceneData(m->sceneGraph);
  }

  View::~View() = default;

  void View::updateResolution(const osg::Vec2f& resolution, int pixelRatio)
  {
    const auto width  = static_cast<int>(resolution.x());
    const auto height = static_cast<int>(resolution.y());

    m->resolution            = resolution;
    m->resolutionInitialized = true;

    if (m->processor.valid())
    {
      osgPPU::Camera::resizeViewport(0, 0, width, height, getCamera());
      m->processor->onViewportChange();
    }

    updateViewport(0, 0, width, height, pixelRatio);
    // m->updateCameraRenderTextures(Impl::UpdateTextureMode::Recreate);
  }

  void View::updateViewport(int x, int y, int width, int height, int pixelRatio)
  {
    const auto viewport = new osg::Viewport(x, y, pixelRatio * width, pixelRatio * height);
    getCamera()->setViewport(viewport);
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

    getCamera()->getOrCreateStateSet()->setAttribute(m->clampColor, enabled ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
  }

  osg::ref_ptr<osg::Group> View::getRootGroup() const
  {
    return m->sceneGraph;
  }

  // TODO: refactor
  /*void View::setHud(const osg::ref_ptr<Hud>& hud)
  {
    if (m->hud == hud)
	  {
      return;
	  }

    if (m->hud.valid())
    {
      m->hudSwitch->removeChild(m->hud->getProjection());
    }

    m->hud = hud;
    m->hudSwitch->addChild(m->hud->getProjection(), true);
  }*/

  void View::addPostProcessingEffect(const osg::ref_ptr<ppu::Effect>& ppe, bool enabled, const std::string& name)
  {
    if (enabled)
	  {
      //m->resetPostProcessingEffects();
	  }

    Impl::PostProcessingState pps;
    pps.effect  = ppe;
    pps.enabled = enabled;

    m->ppeDictionary[name.empty() ? ppe->getName() : name] = pps;

    if (enabled)
	  {
      //m->setupPostProcessingEffects();
	  }
  }

  void View::setPostProcessingEffectEnabled(const std::string& ppeName, bool enabled)
  {
    if (m->ppeDictionary.count(ppeName) == 0)
    {
      OSGG_LOG_WARN("Post processing effect '" + ppeName + "' not found");
      assert_return(false);
    }

    auto& ppe = m->ppeDictionary[ppeName];
    if (ppe.enabled == enabled)
	  {
      return;
	  }

    OSGG_LOG_DEBUG("Post processing effect '" + ppeName + "': " + (enabled ? "enabled" : "disabled"));

    //m->resetPostProcessingEffects();
    ppe.enabled = enabled;
    //m->setupPostProcessingEffects();
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
    assert(m->resolutionInitialized);
    return m->resolution;
  }

  bool View::getPostProcessingEffectEnabled(const std::string& ppeName) const
  {
    return (m->ppeDictionary.count(ppeName) > 0) ? m->ppeDictionary[ppeName].enabled : false;
  }

  bool View::hasPostProcessingEffect(const std::string& ppeName) const
  {
    return (m->ppeDictionary.count(ppeName) > 0);
  }
}
