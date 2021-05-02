#pragma once

#include <osgHelper/ppu/Effect.h>
#include <osgHelper/Camera.h>

#include <osgViewer/View>

#include <memory>


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

    explicit View();
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

  private:
    struct Impl;
    std::unique_ptr<Impl> m;

    void initializePipelineProcessor();

    void assemblePipeline();
    void disassemblePipeline();

    void alterPipelineState(const std::function<void()>& func, UpdateMode mode = UpdateMode::Recreate);

    void updateCameraRenderTextures(UpdateMode mode = UpdateMode::Keep);

  };

}