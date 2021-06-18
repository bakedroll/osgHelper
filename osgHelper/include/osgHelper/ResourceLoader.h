#pragma once

#include <string>
#include <fstream>

#include <osg/Referenced>

namespace osgHelper
{
	class ResourceLoader : public osg::Referenced
	{
	public:
		virtual void getResourceStream(const std::string& resourceKey, std::ifstream& stream, long long& length) = 0;
	};
}