#include <osgHelper/TextResource.h>

namespace osgHelper
{

osg::Object* TextResource::cloneType() const
{
  auto res  = new TextResource();
  res->text = text;
  return res;
}

osg::Object* TextResource::clone(const osg::CopyOp& copyOp) const
{
  auto res  = new TextResource();
  res->text = text;
  return res;
}

const char* TextResource::libraryName() const
{
  return "osgGaming";
}

const char* TextResource::className() const
{
  return "TextResource";
}

}  // namespace osgHelper