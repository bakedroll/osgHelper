#pragma once

#include <osgHelper/ResourceLoader.h>

namespace osgHelper
{
	class FileResourceLoader : public osgHelper::ResourceLoader
	{
	public:
    void getResourceStream(const std::string& resourceKey, std::ifstream& stream, long long& length) override;

	};
}