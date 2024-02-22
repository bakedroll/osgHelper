#pragma once

#include <vector>
#include <string>
#include <algorithm>

#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/Geometry>

#include <osg/StateAttribute>
#include <osgText/Text>

#define C_PI 3.14159265359f
#define C_2PI 6.28318530718f

namespace osgHelper
{

using StringList = std::vector<std::string>;

enum class QuadOrientation
{
	XY,
	XZ,
	YZ
};

void rotateVector(osg::Vec3* vec, const osg::Quat& quat);
void transformVector(osg::Vec3* vec, osg::Matrixd* mat);

osg::Quat getAlignedQuat(const osg::Vec3f& origin, const osg::Vec3f& target);

osg::Quat getQuatFromEuler(double pitch, double roll, double yaw);
osg::Vec3f getEulerFromQuat(const osg::Quat& quat);

osg::Matrix getMatrixFromEuler(double pitch, double roll, double yaw);
osg::Vec3f getVec3FromEuler(double pitch, double roll, double yaw, osg::Vec3 origin = osg::Vec3(0.0, 1.0, 0.0));

osg::Vec2f getPolarFromCartesian(const osg::Vec3f& cartesian);
osg::Vec3f getCartesianFromPolar(const osg::Vec2f& polar);

osg::Vec2f getTextSize(const osg::ref_ptr<osgText::Text>& text);

bool pointInRect(const osg::Vec2f& point, const osg::Vec2f& leftbottom, const osg::Vec2f& righttop);

bool sphereLineIntersection(const osg::Vec3f& sphereCenter,
                            float sphereRadius,
                            const osg::Vec3f& lineOrigin,
                            const osg::Vec3f& lineDirectionNornalized,
                            osg::Vec3f& result);

float pointLineDistance(const osg::Vec3f& origin, const osg::Vec3& direction,
                        const osg::Vec3f& point);

std::vector<osg::Vec3f> lineBoxIntersection(const osg::BoundingBox& bb,
	                                          const osg::Vec3f& l1, const osg::Vec3f& l2);

void generateTangentAndBinormal(osg::Node* node);

osg::StateAttribute::GLModeValue glModeValueFromBool(bool on);
std::string lowerString(std::string str);

osg::ref_ptr<osgText::Text> createTextNode(const std::string& text, float characterSize, const osg::ref_ptr<osgText::Font>& font = {});

osg::ref_ptr<osg::Geometry> createQuadGeometry(
  float left, float right, float bottom, float top, float z = 0.0f,
  QuadOrientation orientation = QuadOrientation::XZ, bool flipped = false);

StringList& splitString(const std::string& s, char delim, StringList& elems);
StringList  splitString(const std::string& s, char delim);

std::string utf8ToLatin1(const char* in);

template <typename T>
T parseVector(const std::string& s)
{
	auto values = splitString(s, ',');
	T result;

	if (values.size() == 1)
	{
		for (auto i = 0; i < T::num_components; i++)
		{
			result._v[i] = stof(values[0]);
		}
	}
	else
	{
		const auto n = std::min<int>(T::num_components, values.size());
		for (auto i = 0; i < n; i++)
		{
			result._v[i] = stof(values[i]);
		}
	}

	return result;
}

}