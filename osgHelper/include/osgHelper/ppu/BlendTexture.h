#pragma once

#include <osgHelper/ppu/Effect.h>
#include <osgHelper/ppu/RenderTextureUnitSink.h>
#include <osgHelper/ioc/Injector.h>

#include <memory>

#include <osgGA/GUIEventHandler>

#include <osgPPU/UnitInOut.h>

namespace osgHelper
{
  namespace ppu
  {
    class BlendTexture : public Effect
    {
    public:
      static const std::string Name;

      explicit BlendTexture(osgHelper::ioc::Injector& injector);
      ~BlendTexture();

      std::string                getName() const override;
      InitialUnitList            getInitialUnits() const override;
      osg::ref_ptr<osgPPU::Unit> getResultUnit() const override;
      InputToUniformList         getInputToUniform() const override;

      RenderTextureUnitSink getBlendTextureSink();

      void setResolution(const osg::Vec2f& resolution);
      void onResizeViewport(const osg::Vec2f& resolution) override;

    protected:
      Status initializeUnits(const osg::GL2Extensions* extensions) override;

    private:
      struct Impl;
      std::unique_ptr<Impl> m;

      void updateResolutionUniforms();

    };
  }
}