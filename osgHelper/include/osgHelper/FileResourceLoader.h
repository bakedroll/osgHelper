#pragma once

#include <osgHelper/IResourceLoader.h>

namespace osgHelper
{
	class FileResourceLoader : public osgHelper::IResourceLoader
	{
	public:
    void getResourceStream(const std::string& resourceKey, std::ifstream& stream, long long& length) override;

	};
}