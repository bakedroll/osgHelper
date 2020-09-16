#include <osgHelper/ppu/Shaders.h>

namespace osgHelper
{
	namespace ppu
	{

const std::string Shaders::ShaderBrightpassFp = 

	"uniform sampler2D hdrInput;" \
	"uniform sampler2D lumInput;" \
	"uniform sampler2D texAdaptedLuminance;" \
	"uniform float g_fMiddleGray;" \

	"float computeScaledLuminance(float avg, float lum)" \
	"{" \
	"	float scaledLum = lum * (g_fMiddleGray / (avg + 0.001));" \
	"	scaledLum = min(scaledLum, 65504.0);" \
	"	return scaledLum / (1.0 + scaledLum);" \
	"}" \

	"void main(void)" \
	"{" \
	"	const float BRIGHT_PASS_THRESHOLD = 0.9;" \
	"	const float BRIGHT_PASS_OFFSET = 1.0;" \
	
	"	float fLuminance = texture2D(lumInput, gl_TexCoord[0].st).r;" \
	"	float fAdaptedLum = texture2D(texAdaptedLuminance, vec2(0.5, 0.5)).w;" \
	"	float fScaledLum = computeScaledLuminance(fAdaptedLum, fLuminance);" \

	"	vec3 vSample = texture2D(hdrInput, gl_TexCoord[0].st).rgb;" \
	"	vSample *= fScaledLum;" \
	"	vSample -= BRIGHT_PASS_THRESHOLD;" \
	"	vSample = max(vSample, vec3(0.0, 0.0, 0.0));" \
	"	vSample /= (BRIGHT_PASS_OFFSET + vSample);" \

	"	gl_FragColor.rgb = vSample;" \
	"	gl_FragColor.a = fAdaptedLum;" \
	"}";

const std::string Shaders::ShaderDepthOfFieldFp =

	"uniform sampler2D texColorMap;" \
	"uniform sampler2D texBlurredColorMap;" \
	"uniform sampler2D texStrongBlurredColorMap;" \
	"uniform sampler2D texDepthMap;" \
	"uniform float focalLength;" \
	"uniform float focalRange;" \
	"uniform float zNear;" \
	"uniform float zFar;" \

	"float convertZ(in float near, in float far, in float depthBufferValue)" \
	"{" \
	"	float clipZ = (depthBufferValue - 0.5) * 2.0;" \
	"	return -(2.0 * far * near) / (clipZ * (far - near) - (far + near));" \
	"}" \

	"void main(void)" \
	"{" \
	"	vec2 inTex = gl_TexCoord[0].st;" \

	"	float a = zFar / (zFar - zNear);" \
	"	float b = zFar * zNear / (zNear - zFar);" \
	"	float depth = texture2D(texDepthMap, inTex).x;" \
	"	float dist = b / (depth - a);" \

	"	vec4 colorValue = texture2D(texColorMap, inTex).rgba;" \
	"	vec4 blurredValue1 = texture2D(texBlurredColorMap, inTex).rgba;" \
	"	vec4 blurredValue2 = texture2D(texStrongBlurredColorMap, inTex).rgba;" \

	"	float blur = clamp(abs(dist - focalLength) / focalRange, 0.0, 1.0);" \
	"	float factor1 = 1.0;" \
	"	float factor2 = 0.0;" \

	"	if (blur > 0.5)" \
	"		factor2 = (blur - 0.5) * 2.0;" \
	"	else" \
	"		factor1 = blur * 2.0;" \

	"	vec4 result = mix(colorValue, blurredValue1, factor1);" \
	"	gl_FragColor = mix(result, blurredValue2, factor2);" \
	"}";

const std::string Shaders::ShaderFxaaFp = 

	"#version 120\n" \
	"#extension GL_EXT_gpu_shader4 : enable\n" \
	"uniform sampler2D tex0;" \
	"uniform float vx_offset;" \
	"uniform float rt_w;" \
	"uniform float rt_h;" \
	"uniform float FXAA_SPAN_MAX = 8.0;" \
	"uniform float FXAA_REDUCE_MUL = 1.0 / 8.0;" \
	"varying vec4 posPos;\n" \

	"#define FxaaInt2 ivec2\n" \
	"#define FxaaFloat2 vec2\n" \
	"#define FxaaTexLod0(t, p) texture2DLod(t, p, 0.0)\n" \
	"#define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)\n" \

	"vec3 FxaaPixelShader(" \
	"	vec4 posPos," \
	"	sampler2D tex," \
	"	vec2 rcpFrame)" \
	"{\n" \

	"#define FXAA_REDUCE_MIN   (1.0/128.0)\n" \

	"	vec3 rgbNW = FxaaTexLod0(tex, posPos.zw).xyz;" \
	"	vec3 rgbNE = FxaaTexOff(tex, posPos.zw, FxaaInt2(1, 0), rcpFrame.xy).xyz;" \
	"	vec3 rgbSW = FxaaTexOff(tex, posPos.zw, FxaaInt2(0, 1), rcpFrame.xy).xyz;" \
	"	vec3 rgbSE = FxaaTexOff(tex, posPos.zw, FxaaInt2(1, 1), rcpFrame.xy).xyz;" \
	"	vec3 rgbM = FxaaTexLod0(tex, posPos.xy).xyz;" \

	"	vec3 luma = vec3(0.299, 0.587, 0.114);" \
	"	float lumaNW = dot(rgbNW, luma);" \
	"	float lumaNE = dot(rgbNE, luma);" \
	"	float lumaSW = dot(rgbSW, luma);" \
	"	float lumaSE = dot(rgbSE, luma);" \
	"	float lumaM = dot(rgbM, luma);" \

	"	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));" \
	"	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));" \

	"	vec2 dir;" \
	"	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));" \
	"	dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));" \

	"	float dirReduce = max(" \
	"		(lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL)," \
	"		FXAA_REDUCE_MIN);" \
	"	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);" \
	"	dir = min(FxaaFloat2(FXAA_SPAN_MAX, FXAA_SPAN_MAX)," \
	"		max(FxaaFloat2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX)," \
	"		dir * rcpDirMin)) * rcpFrame.xy;" \

	"	vec3 rgbA = (1.0 / 2.0) * (" \
	"		FxaaTexLod0(tex, posPos.xy + dir * (1.0 / 3.0 - 0.5)).xyz +" \
	"		FxaaTexLod0(tex, posPos.xy + dir * (2.0 / 3.0 - 0.5)).xyz);" \
	"	vec3 rgbB = rgbA * (1.0 / 2.0) + (1.0 / 4.0) * (" \
	"		FxaaTexLod0(tex, posPos.xy + dir * (0.0 / 3.0 - 0.5)).xyz +" \
	"		FxaaTexLod0(tex, posPos.xy + dir * (3.0 / 3.0 - 0.5)).xyz);" \
	"	float lumaB = dot(rgbB, luma);" \
	"	if ((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;" \
	"	return rgbB;" \
	"}" \

	"vec4 PostFX(sampler2D tex, vec2 uv, float time)" \
	"{" \
	"	vec4 c = vec4(0.0);" \
	"	vec2 rcpFrame = vec2(1.0 / rt_w, 1.0 / rt_h);" \
	"	c.rgb = FxaaPixelShader(posPos, tex, rcpFrame);" \
	"	c.a = 1.0;" \
	"	return c;" \
	"}" \

	"void main()" \
	"{" \
	"	vec2 uv = gl_TexCoord[0].st;" \
	"	gl_FragColor = PostFX(tex0, uv, 0.0);" \
	"}";

const std::string Shaders::ShaderFxaaVp =

	"#version 120\n" \
	"varying vec4 posPos;" \
	"uniform float FXAA_SUBPIX_SHIFT = 1.0 / 4.0;" \
	"uniform float rt_w;" \
	"uniform float rt_h;" \

	"void main(void)" \
	"{" \
	"	gl_Position = ftransform();" \
	"	gl_TexCoord[0] = gl_MultiTexCoord0;" \
	"	vec2 rcpFrame = vec2(1.0 / rt_w, 1.0 / rt_h);" \
	"	posPos.xy = gl_MultiTexCoord0.xy;" \
	"	posPos.zw = gl_MultiTexCoord0.xy - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT));" \
	"}";

const std::string Shaders::ShaderGaussConvolution1dxFp =

	"uniform sampler2D texUnit0;" \
	"uniform float radius;" \
	"uniform float sigma;" \
	"varying float sigma2;" \
	"varying float c;" \
	"uniform float osgppu_ViewportWidth;" \
	"uniform float osgppu_ViewportHeight;" \

	"void main(void)" \
	"{" \
	"	vec4 color = vec4(0.0);" \
	"	float totalWeigth = 0.0;" \
	"	float inputTexTexelWidth = 1.0 / osgppu_ViewportWidth;" \

	"	for (float i = -radius; i < radius; i += 1.0)" \
	"	{" \
	"		float weight = c * exp((i*i) / (-sigma2));" \
	"		totalWeigth += weight;" \
	"		color += texture2D(texUnit0, gl_TexCoord[0].xy + vec2(i * inputTexTexelWidth, 0)) * weight;" \
	"	}" \
	"	color /= totalWeigth;" \

	"	gl_FragColor = color;" \
	"}";

const std::string Shaders::ShaderGaussConvolution1dyFp =

	"uniform sampler2D texUnit0;" \
	"uniform float radius;" \
	"uniform float sigma;" \
	"varying float sigma2;" \
	"varying float c;" \
	"uniform float osgppu_ViewportWidth;" \
	"uniform float osgppu_ViewportHeight;" \

	"void main(void)" \
	"{" \
	"	vec4 color = vec4(0.0);" \
	"	float totalWeigth = 0.0;" \
	"	float inputTexTexelWidth = 1.0 / osgppu_ViewportHeight;" \

	"	for (float i = -radius; i < radius; i += 1.0)" \
	"	{" \
	"		float weight = c * exp((i*i) / (-sigma2));" \
	"		totalWeigth += weight;" \

	"		color += texture2D(texUnit0, gl_TexCoord[0].xy + vec2(0, i * inputTexTexelWidth)) * weight;" \
	"	}" \
	"	color /= totalWeigth;" \

	"	gl_FragColor = color;" \
	"}";

const std::string Shaders::ShaderGaussConvolutionVp =

	"uniform float sigma;" \
	"const float PI = 3.1415926535897;" \
	"varying float sigma2;" \
	"varying float c;" \

	"void main(void)" \
	"{" \
	"	gl_TexCoord[0] = gl_MultiTexCoord0;" \
	"	gl_Position = ftransform();" \
	"	gl_FrontColor = gl_Color;" \

	"	sigma2 = 2.0 * sigma * sigma;" \
	"	c = sqrt((1.0 / (sigma2 * PI)));" \
	"}";

const std::string Shaders::ShaderLuminanceAdaptedFp =

	"uniform sampler2D texLuminance;" \
	"uniform sampler2D texAdaptedLuminance;" \
	"uniform float maxLuminance;" \
	"uniform float minLuminance;" \
	"uniform float invFrameTime;" \
	"uniform float adaptScaleFactor;" \

	"const float TauCone = 0.01;" \
	"const float TauRod = 0.04;" \

	"void main(void)" \
	"{" \
	"	float current = texture2D(texLuminance, vec2(0.5, 0.5), 100.0).x;" \
	"	float old = texture2D(texAdaptedLuminance, vec2(0.5, 0.5)).w;" \
	"	float sigma = clamp(0.4 / (0.04 + current), 0.0, 1.0);" \
	"	float Tau = mix(TauCone, TauRod, sigma) / adaptScaleFactor;" \
	"	float lum = old + (current - old) * (1.0 - exp(-(invFrameTime) / Tau));" \

	"	gl_FragData[0].xyzw = vec4(clamp(lum, minLuminance, maxLuminance));" \
	"}";

const std::string Shaders::ShaderLuminanceFp =

	"uniform sampler2D texUnit0;" \

	"void main(void)" \
	"{" \
	"	vec4 texColor0 = texture2D(texUnit0, gl_TexCoord[0].st);" \

	"	gl_FragColor.xyz = vec3(texColor0.r * 0.2125 + texColor0.g * 0.7154 + texColor0.b * 0.0721);" \
	"	gl_FragColor.a = texColor0.a;" \
	"}";

const std::string Shaders::ShaderLuminanceMipmapFp =

	"uniform sampler2D texUnit0;" \
	"uniform float osgppu_ViewportWidth;" \
	"uniform float osgppu_ViewportHeight;" \
	"uniform float osgppu_MipmapLevel;" \
	"uniform float osgppu_MipmapLevelNum;" \

	"void main(void)" \
	"{" \
	"	const float epsilon = 0.001;" \
	"	float res = 0.0;" \
	"	float c[4];" \

	"	vec2 size = vec2(osgppu_ViewportWidth, osgppu_ViewportHeight) * 2.0;" \
	"	vec2 iCoord = gl_TexCoord[0].st;" \
	"	vec2 texel = vec2(1.0, 1.0) / (size);" \
	"	vec2 halftexel = vec2(0.5, 0.5) / size;" \

	"	vec2 st[4];" \
	"	st[0] = iCoord - halftexel + vec2(0, 0);" \
	"	st[1] = iCoord - halftexel + vec2(texel.x, 0);" \
	"	st[2] = iCoord - halftexel + vec2(0, texel.y);" \
	"	st[3] = iCoord - halftexel + vec2(texel.x, texel.y);" \

	"	for (int i = 0; i < 4; i++)" \
	"	{" \
	"		st[i] = clamp(st[i], vec2(0, 0), vec2(1, 1));" \
	"		c[i] = texture2D(texUnit0, st[i], osgppu_MipmapLevel - 1.0).r;" \
	"	}" \

	"	if (abs(osgppu_MipmapLevel - 1.0) < 0.00001)" \
	"	{" \
	"		res += log(epsilon + c[0]);" \
	"		res += log(epsilon + c[1]);" \
	"		res += log(epsilon + c[2]);" \
	"		res += log(epsilon + c[3]);" \
	"	}" \
	"	else" \
	"	{" \
	"		res += c[0];" \
	"		res += c[1];" \
	"		res += c[2];" \
	"		res += c[3];" \
	"	}" \

	"	res *= 0.25;" \

	"	if (osgppu_MipmapLevelNum - osgppu_MipmapLevel < 2.0)" \
	"	{" \
	"		res = exp(res);" \
	"	}" \

	"	gl_FragData[0].rgba = vec4(min(res, 65504.0));" \
	"}";

const std::string Shaders::ShaderTonemapHdrFp =

	"uniform sampler2D blurInput;" \
	"uniform sampler2D hdrInput;" \
	"uniform sampler2D lumInput;" \
	"uniform sampler2D texAdaptedLuminance;" \
	"uniform float fBlurFactor;" \
	"uniform float g_fMiddleGray;" \

	"float computeScaledLuminance(float avg, float lum)" \
	"{" \
	"	float scaledLum = lum * (g_fMiddleGray / (avg + 0.001));" \
	"	scaledLum = min(scaledLum, 65504.0);" \
	"	return scaledLum / (1.0 + scaledLum);" \
	"}" \

	"void main(void)" \
	"{" \
	"	vec2 inTex = gl_TexCoord[0].st;" \
	"	vec4 blurColor = texture2D(blurInput, inTex);" \
	"	vec4 hdrColor = texture2D(hdrInput, inTex);" \

	"	float fLuminance = texture2D(lumInput, inTex).r;" \
	"	float fAdaptedLum = texture2D(texAdaptedLuminance, vec2(0.5, 0.5)).w;" \
	"	float fScaledLum = computeScaledLuminance(fAdaptedLum, fLuminance);" \

	"	vec4 color = hdrColor * fScaledLum;" \

	"	gl_FragColor.rgb = blurColor.rgb * fBlurFactor + color.rgb;" \
	"	gl_FragColor.a = 1.0;" \
	"}";

}
}