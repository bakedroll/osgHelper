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

    View();
    ~View();

    void updateResolution(const osg::Vec2f& resolution, int pixelRatio = 1);
    void updateViewport(int x, int y, int width, int height, int pixelRatio = 1);

    void setClampColorEnabled(bool enabled);

    osg::ref_ptr<osg::Group>        getRootGroup() const;
    osg::ref_ptr<osgHelper::Camera> getSceneCamera() const;

    void addPostProcessingEffect(const osg::ref_ptr<ppu::Effect>& ppe, bool enabled = true,
                                 const std::string& name = "");

    void setPostProcessingEffectEnabled(const std::string& ppeName, bool enabled);

    osg::ref_ptr<ppu::Effect> getPostProcessingEffect(const std::string& ppeName) const;
    osg::Vec2f                getResolution() const;

    bool getPostProcessingEffectEnabled(const std::string& ppeName) const;
    bool hasPostProcessingEffect(const std::string& ppeName) const;

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