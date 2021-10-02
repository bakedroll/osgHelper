#include "osgHelper/ppu/RenderTextureUnitSink.h"

namespace osgHelper
{
namespace ppu
{

RenderTextureUnitSink::RenderTextureUnitSink(const osg::ref_ptr<Effect>& effect,
                                             const osg::ref_ptr<osgPPU::Unit>& unitSink,
                                             const std::string& uniformName,
                                             osg::Camera::BufferComponent bufferComponent)
  : m_effect(effect)
  , m_unitSink(unitSink)
  , m_uniformName(uniformName)
  , m_bufferComponent(bufferComponent)
{
}

RenderTextureUnitSink::~RenderTextureUnitSink() = default;

osg::ref_ptr<Effect> RenderTextureUnitSink::getEffect() const
{
  return m_effect;
}

osg::ref_ptr<osgPPU::Unit> RenderTextureUnitSink::getUnitSink() const
{
  return m_unitSink;
}

std::string RenderTextureUnitSink::getUniformName() const
{
  return m_uniformName;
}

osg::Camera::BufferComponent RenderTextureUnitSink::getBufferComponent() const
{
  return m_bufferComponent;
}

}
}