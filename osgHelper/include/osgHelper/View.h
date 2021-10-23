#pragma once

#include <utilsLib/Utils.h>

#include <osgHelper/ppu/Effect.h>
#include <osgHelper/Camera.h>
#include <osgHelper/ppu/RenderTextureUnitSink.h>

#include <osgViewer/View>

#include <memory>
#include <functional>

namespace osgHelper
{
  class View : public osgViewer::View
  {
  public:
    using Ptr = osg::ref_ptr<View>;
    using WeakPtr = osg::observer_ptr<View>;

    //! Determines if something should be recreated
    enum class UpdateMode
    {
      Recreate, //!< recreate
      Keep      //!< do not recreate
    };

    //! Determines a specific camera that is used in this view
    enum class CameraType : int
    {
      Scene,  //!< The main scene camera, usually with a perspective projection mode
      Screen, //!< The camera that renders the frame buffer object resulting from the main render stage
              //!< to the screen, usually in ortho2D projection mode
      _Count  //!< Used to determine the number of different camera types in this view
    };

    enum class SlaveCameraMode
    {
      UseMasterSceneData,
      UseSlaveChildSceneData
    };

    enum class TextureComponent
    {
      ColorBuffer = 0x1,
      DepthBuffer = 0x2
    };

    using SlaveRenderTextures = std::map<TextureComponent, osg::ref_ptr<osg::Texture2D>>;

    struct RTTSlaveCameraData
    {
      osg::ref_ptr<Camera> camera;
      SlaveRenderTextures  textures;
    };

    struct RTTSlaveCameraScreenQuadData
    {
      RTTSlaveCameraData      slaveCameraData;
      osg::ref_ptr<osg::Node> screenQuadNode;
    };

    using ResizeCallbackFunc = std::function<void(const osg::Vec2i&)>;

    struct ResizeCallback
    {
      ResizeCallbackFunc func;
    };

    View();
    ~View();

    void updateResolution(const osg::Vec2f& resolution, float pixelRatio = 1.0f);
    void updateCameraViewports(int x, int y, int width, int height, float pixelRatio = 1.0f) const;

    void setClampColorEnabled(bool enabled);

    osg::ref_ptr<osg::Group>        getRootGroup() const;
    osg::ref_ptr<osgHelper::Camera> getCamera(CameraType type) const;

    void addPostProcessingEffect(const osg::ref_ptr<ppu::Effect>& ppe, bool enabled = true,
                                 const std::string& name = "");

    void setPostProcessingEffectEnabled(const std::string& ppeName, bool enabled);

    osg::ref_ptr<ppu::Effect> getPostProcessingEffect(const std::string& ppeName) const;
    osg::Vec2f                getResolution() const;

    bool getPostProcessingEffectEnabled(const std::string& ppeName) const;
    bool hasPostProcessingEffect(const std::string& ppeName) const;

    void cleanUp();

    std::shared_ptr<ResizeCallback> registerResizeCallback(const ResizeCallbackFunc& func);

    RTTSlaveCameraData createRenderToTextureSlaveCamera(const osg::Vec2i& resolution,
                                                        TextureComponent components = TextureComponent::ColorBuffer,
                                                        SlaveCameraMode mode = SlaveCameraMode::UseSlaveChildSceneData);

    osg::ref_ptr<Camera> createRenderToTextureSlaveCameraToUnitSink(const ppu::RenderTextureUnitSink& sink,
                                                                    SlaveCameraMode mode = SlaveCameraMode::UseSlaveChildSceneData);

    RTTSlaveCameraScreenQuadData createRenderToTextureSlaveCameraToScreenQuad(TextureComponent components = TextureComponent::ColorBuffer,
                                                                              SlaveCameraMode mode = SlaveCameraMode::UseSlaveChildSceneData);

  private:
    struct Impl;
    std::unique_ptr<Impl> m;

    void initializePipelineProcessor();

    void assemblePipeline();
    void disassemblePipeline();

    void alterPipelineState(const std::function<void()>& func, UpdateMode mode = UpdateMode::Recreate);

    void updateCameraRenderTextures(UpdateMode mode = UpdateMode::Keep);

    osg::ref_ptr<Camera> createSlaveCamera(const osg::Vec2i& resolution, bool inheritViewport, SlaveCameraMode mode);

  };
}

ENABLE_BITMASK_OPERATORS(osgHelper::View::TextureComponent);
