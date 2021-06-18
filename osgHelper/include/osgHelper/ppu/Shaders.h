#pragma once

#include <string>

namespace osgHelper
{
namespace ppu
{

	struct Shaders
	{
		static const std::string ShaderBrightpassFp;
		static const std::string ShaderDepthOfFieldFp;
		static const std::string ShaderFxaaFp;
		static const std::string ShaderFxaaVp;
		static const std::string ShaderGaussConvolution1dxFp;
		static const std::string ShaderGaussConvolution1dyFp;
		static const std::string ShaderGaussConvolutionVp;
		static const std::string ShaderLuminanceAdaptedFp;
		static const std::string ShaderLuminanceFp;
		static const std::string ShaderLuminanceMipmapFp;
		static const std::string ShaderTonemapHdrFp;
	};

}
}