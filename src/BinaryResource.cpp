#include <osgHelper/BinaryResource.h>

namespace osgHelper
{

BinaryResource::BinaryResource() : Object(), m_bytes(nullptr)
{
}

BinaryResource::~BinaryResource()
{
  delete[] m_bytes;
}

osg::Object* BinaryResource::cloneType() const
{
  const auto res = new BinaryResource();
  return res;
}

osg::Object* BinaryResource::clone(const osg::CopyOp& copyOp) const
{
  const auto res = new BinaryResource();
  return res;
}

const char* BinaryResource::libraryName() const
{
  return "osgGaming";
}

const char* BinaryResource::className() const
{
  return "BinaryResource";
}

char* BinaryResource::getBytes() const
{
  return m_bytes;
}

void BinaryResource::setBytes(char* bytes)
{
  delete[] m_bytes;
  m_bytes = bytes;
}

}  // namespace osgHelper