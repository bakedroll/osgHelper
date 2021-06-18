#pragma once

#include <string>

#include <osg/Object>

namespace osgHelper
{
	class TextResource : public osg::Object
	{
	public:
		osg::Object* cloneType() const override;
		osg::Object* clone(const osg::CopyOp& copyOp) const override;
		const char* libraryName() const override;
		const char* className() const override;

		std::string text;
	};
}