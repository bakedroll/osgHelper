#pragma once

#include <osgHelper/IShaderBlueprint.h>

#include <osg/Referenced>
#include <osg/Shader>

namespace osgHelper
{

class IShaderFactory : public osg::Referenced
{
public:
	IShaderFactory() = default;
	virtual ~IShaderFactory() = default;

	virtual osg::ref_ptr<osg::Shader> fromSourceText(const std::string& key, const std::string& source, osg::Shader::Type type) = 0;
	virtual osg::ref_ptr<IShaderBlueprint> make() const = 0;

};

}