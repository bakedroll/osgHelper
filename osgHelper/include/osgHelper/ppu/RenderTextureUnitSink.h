#pragma once

#include <osgHelper/ppu/Effect.h>

#include <osgPPU/Unit.h>

#include <string>

namespace osgHelper
{
namespace ppu
{

class RenderTextureUnitSink
{
public:
  RenderTextureUnitSink(const osg::ref_ptr<Effect>& effect,
                        const osg::ref_ptr<osgPPU::Unit>& unitSink,
                        const std::string& uniformName,
                        osg::Camera::BufferComponent bufferComponent = osg::Camera::COLOR_BUFFER);
  virtual ~RenderTextureUnitSink();

  osg::ref_ptr<Effect> getEffect() const;
  osg::ref_ptr<osgPPU::Unit> getUnitSink() const;
  std::string getUniformName() const;
  osg::Camera::BufferComponent getBufferComponent() const;

private:
  osg::ref_ptr<Effect> m_effect;
  osg::ref_ptr<osgPPU::Unit> m_unitSink;
  std::string m_uniformName;
  osg::Camera::BufferComponent m_bufferComponent;

};

}
}