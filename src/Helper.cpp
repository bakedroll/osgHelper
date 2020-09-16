#include "../../osgGaming/include/osgGaming/Helper.h"

#include <osgHelper/Helper.h>

#include <math.h>
#include <sstream>

#include <osg/Geode>
#include <osg/Version>
#include <osgUtil/TangentSpaceGenerator>

using namespace osg;
using namespace osgText;
using namespace std;

// Z  Y
// | /
// |/
// +----- X

void osgHelper::rotateVector(Vec3* vec, Quat quat)
{
	Matrixd mat = Matrixd::identity();
	mat.setRotate(quat);

	transformVector(vec, &mat);
}

void osgHelper::transformVector(Vec3* vec, Matrixd* mat)
{
	Vec3 cpy(*vec);

	(*vec)[0] = (*mat)(0, 0)*cpy[0] + (*mat)(1, 0)*cpy[1] + (*mat)(2, 0)*cpy[2] + (*mat)(3, 0);
	(*vec)[1] = (*mat)(0, 1)*cpy[0] + (*mat)(1, 1)*cpy[1] + (*mat)(2, 1)*cpy[2] + (*mat)(3, 1);
	(*vec)[2] = (*mat)(0, 2)*cpy[0] + (*mat)(1, 2)*cpy[1] + (*mat)(2, 2)*cpy[2] + (*mat)(3, 2);
}

Quat osgHelper::getQuatFromEuler(double pitch, double roll, double yaw)
{
	Quat q;

	double c1 = cos(roll / 2.0);
	double s1 = sin(roll / 2.0);
	double c2 = cos(yaw / 2.0);
	double s2 = sin(yaw / 2.0);
	double c3 = cos(pitch / 2.0);
	double s3 = sin(pitch / 2.0);

	double c1c2 = c1*c2;
	double s1s2 = s1*s2;

	q[0] = c1c2*s3 + s1s2*c3;
	q[1] = s1*c2*c3 + c1*s2*s3;
	q[2] = c1*s2*c3 - s1*c2*s3;
	q[3] = c1c2*c3 - s1s2*s3;

	return q;
}

Matrix osgHelper::getMatrixFromEuler(double pitch, double roll, double yaw)
{
	Quat quat = getQuatFromEuler(pitch, roll, yaw);

	Matrix mat;
	mat.setRotate(quat);

	return mat;
}

Vec3f osgHelper::getVec3FromEuler(double pitch, double roll, double yaw, Vec3 origin)
{
	rotateVector(&origin, osgHelper::getQuatFromEuler(pitch, roll, yaw));

	return origin;
}

Vec2f osgHelper::getPolarFromCartesian(Vec3f cartesian)
{
	Vec2f result;

	result.x() = (atan2(-cartesian.x(), cartesian.y()) + C_PI) / (2.0f * C_PI);
	float xyLen = sqrt(cartesian.x() * cartesian.x() + cartesian.y() * cartesian.y());
	result.y() = (atan2(-cartesian.z(), xyLen) + C_PI / 2.0f) / C_PI;

	return result;
}

Vec3f osgHelper::getCartesianFromPolar(Vec2f polar)
{
	Matrix mat = Matrix::rotate(getQuatFromEuler(polar.x(), 0.0f, polar.y()));

	return Vec3f(0.0f, 1.0f, 0.0f) * mat;
}

Vec2f osgHelper::getTextSize(ref_ptr<Text> text)
{
	BoundingBox bb;

	Text::TextureGlyphQuadMap glyphs = text->getTextureGlyphQuadMap();
	for (Text::TextureGlyphQuadMap::iterator it = glyphs.begin(); it != glyphs.end(); ++it)
	{
#if OSG_MIN_VERSION_REQUIRED(3, 6, 4)

   // not needed anymore...

#else

		Text::GlyphQuads::Coords2 quads = it->second.getCoords();
#if OSG_MIN_VERSION_REQUIRED(3, 4, 0)
		for (int i=0; i<quads->size(); i++)
		  bb.expandBy((*quads)[i].x(), (*quads)[i].y(), 0.0f);
#else
		for (Text::GlyphQuads::Coords2::iterator qit = quads.begin(); qit != quads.end(); ++qit)
		{
			bb.expandBy(qit->x(), qit->y(), 0.0f);
		}
#endif
#endif
	}

	float width = bb.xMax() - bb.xMin();
	float height = bb.yMax() - bb.yMin();

	return Vec2f(width, height);
}

bool osgHelper::pointInRect(Vec2f point, Vec2f leftbottom, Vec2f righttop)
{
	return (point.x() >= leftbottom.x() && point.y() >= leftbottom.y()
		&& point.x() <= righttop.x() && point.y() <= righttop.y());
}

bool osgHelper::sphereLineIntersection(Vec3f sphereCenter, float sphereRadius, Vec3f lineOrigin, Vec3f lineDirectionNormalized, Vec3f& result)
{
	float a = lineDirectionNormalized * lineDirectionNormalized;
	float b = lineDirectionNormalized * ((lineOrigin - sphereCenter) * 2.0f);
	float c = (sphereCenter * sphereCenter) + (lineOrigin * lineOrigin) - 2.0f * (lineOrigin * sphereCenter) - sphereRadius * sphereRadius;
	float D = b * b + (-4.0f) * a * c;

	if (D < 0)
	{
		return false;
	}

	D = sqrtf(D);

	float t = a == 0.0f ? 1.0f : (-0.5f) * (b + D) / a;
	if (t > 0.0f)
	{
		result = lineOrigin + lineDirectionNormalized * t;
	}
	else
	{
		return false;
	}

	return true;
}
float osgHelper::pointLineDistance(osg::Vec3f origin, Vec3 direction, Vec3f point)
{
	Vec3f vec = direction ^ (point - origin);

	return vec.length();
}

void osgHelper::generateTangentAndBinormal(Node* node)
{
	ref_ptr<Group> group = node->asGroup();
	ref_ptr<Geode> geode = node->asGeode();

  if (geode)
  {
    for (unsigned int i = 0; i<geode->getNumDrawables(); i++)
    {
      Geometry *geometry = geode->getDrawable(i)->asGeometry();
      if (geometry)
      {
        ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator();
        tsg->generate(geometry);

        osg::Vec4Array* arr = tsg->getTangentArray();

        geometry->setVertexAttribArray(6, tsg->getTangentArray());
        geometry->setVertexAttribBinding(6, osg::Geometry::BIND_PER_VERTEX);

        geometry->setVertexAttribArray(7, tsg->getBinormalArray());
        geometry->setVertexAttribBinding(7, osg::Geometry::BIND_PER_VERTEX);
        geometry->setVertexAttribNormalize(7, GL_FALSE);

        geometry->setUseVertexBufferObjects(true);
        //geometry->getVertexAttribArray(6)->dirty();
        //geometry->getVertexAttribArray(7)->dirty();

        tsg.release();
      }
    }
  }
  else if (group)
	{
		for (unsigned int i = 0; i<group->getNumChildren(); i++)
			generateTangentAndBinormal(group->getChild(i));
	}
}

StateAttribute::GLModeValue osgHelper::glModeValueFromBool(bool on)
{
	return on ? StateAttribute::ON : StateAttribute::OFF;
}

string osgHelper::lowerString(string str)
{
#ifdef WIN32
	transform(str.begin(), str.end(), str.begin(), tolower);
#else
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
#endif

	return str;
}

ref_ptr<Text> osgHelper::createTextNode(string text, float characterSize, ref_ptr<Font> font)
{
	ref_ptr<Text> textNode = new Text();

	if (font.valid())
	{
		textNode->setFont(font);
	}

	textNode->setCharacterSize(characterSize);
	textNode->setText(text);
	textNode->setAxisAlignment(osgText::Text::SCREEN);
	textNode->setDrawMode(osgText::Text::TEXT);
	textNode->setColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	textNode->setDataVariance(osg::Object::DYNAMIC);

	return textNode;
}

ref_ptr<Geometry> osgHelper::createQuadGeometry(float left, float right, float bottom, float top, float z, QuadOrientation orientation, bool flipped)
{
	ref_ptr<Geometry> geo = new Geometry();

	ref_ptr<Vec3Array> verts = new Vec3Array();
	ref_ptr<Vec3Array> normals = new Vec3Array();

	float normal = flipped ? 1.0f : -1.0f;

	switch (orientation)
	{
	case XY:
		verts->push_back(Vec3(left, bottom, z));
		verts->push_back(Vec3(left, top, z));
		verts->push_back(Vec3(right, top, z));
		verts->push_back(Vec3(right, bottom, z));

		normals->push_back(Vec3(0.0f, 0.0f, normal));
		break;
	case XZ:
		verts->push_back(Vec3(left, z, bottom));
		verts->push_back(Vec3(right, z, bottom));
		verts->push_back(Vec3(right, z, top));
		verts->push_back(Vec3(left, z, top));

		normals->push_back(Vec3(0.0f, normal, 0.0f));
		break;
	case YZ:
		verts->push_back(Vec3(z, left, bottom));
		verts->push_back(Vec3(z, left, top));
		verts->push_back(Vec3(z, right, top));
		verts->push_back(Vec3(z, right, bottom));

		normals->push_back(Vec3(normal, 0.0f, 0.0f));
		break;
	}

	ref_ptr<DrawElementsUInt> indices = new DrawElementsUInt(PrimitiveSet::POLYGON, 0);

	if (flipped)
	{
		indices->push_back(3);
		indices->push_back(2);
		indices->push_back(1);
		indices->push_back(0);
	}
	else
	{
		indices->push_back(0);
		indices->push_back(1);
		indices->push_back(2);
		indices->push_back(3);
	}

	//ref_ptr<Vec4Array> colors = new Vec4Array();
	//colors->push_back(color);

	ref_ptr<Vec2Array> texcoords = new Vec2Array();
	texcoords->push_back(Vec2(0.0f, 1.0f));
	texcoords->push_back(Vec2(0.0f, 0.0f));
	texcoords->push_back(Vec2(1.0f, 0.0f));
  texcoords->push_back(Vec2(1.0f, 1.0f));

	geo->setTexCoordArray(0, texcoords);
	geo->addPrimitiveSet(indices);
	geo->setVertexArray(verts);
	geo->setNormalArray(normals);
	geo->setNormalBinding(Geometry::BIND_OVERALL);
	//geo->setColorArray(colors);
	//geo->setColorBinding(osg::Geometry::BIND_OVERALL);

	return geo;
}

osgHelper::StringList& osgHelper::splitString(const string &s, char delim, StringList &elems)
{
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim))
	{
		elems.push_back(item);
	}

	return elems;
}

osgHelper::StringList osgHelper::splitString(const string &s, char delim)
{
	StringList elems;
	splitString(s, delim, elems);
	return elems;
}

string osgHelper::utf8ToLatin1(const char* in)
{
	string out;
	if (in == nullptr)
	{
		return out;
	}

	unsigned int codepoint = 0;
	while (*in != 0)
	{
		unsigned char ch = static_cast<unsigned char>(*in);
		if (ch <= 0x7f)
			codepoint = ch;
		else if (ch <= 0xbf)
			codepoint = (codepoint << 6) | (ch & 0x3f);
		else if (ch <= 0xdf)
			codepoint = ch & 0x1f;
		else if (ch <= 0xef)
			codepoint = ch & 0x0f;
		else
			codepoint = ch & 0x07;
		++in;
		if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff))
		{
			if (codepoint <= 255)
			{
				out.append(1, static_cast<char>(codepoint));
			}
		}
	}

	return out;
}
