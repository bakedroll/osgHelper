#include <osgHelper/Helper.h>

#include <cmath>
#include <sstream>

#include <osg/Geode>
#include <osg/Version>
#include <osgUtil/TangentSpaceGenerator>

// Z  Y
// | /
// |/
// +----- X

bool getIntersection(float fDst1, float fDst2, const osg::Vec3f& p1, const osg::Vec3f& p2, osg::Vec3f& hit)
{
	if ((fDst1 * fDst2) >= 0.0f)
	{
		return false;
	}

	if (fDst1 == fDst2)
	{
		return false;
	}

	hit = p1 + (p2-p1) * (-fDst1 / (fDst2 - fDst1));
	return true;
}

bool isInBox(const osg::Vec3f& hit, const osg::Vec3f& b1, const osg::Vec3f& b2, const int axis)
{
	if (axis==1 && hit.z() > b1.z() && hit.z() < b2.z() && hit.y() > b1.y() && hit.y() < b2.y())
	{
		return true;
	}

	if (axis==2 && hit.z() > b1.z() && hit.z() < b2.z() && hit.x() > b1.x() && hit.x() < b2.x())
	{
		return true;
	}

	if (axis==3 && hit.x() > b1.x() && hit.x() < b2.x() && hit.y() > b1.y() && hit.y() < b2.y())
	{
		return true;
	}

	return false;
}

void osgHelper::rotateVector(osg:: Vec3* vec, const osg::Quat& quat)
{
	osg::Matrixd mat = osg::Matrixd::identity();
	mat.setRotate(quat);

	transformVector(vec, &mat);
}

void osgHelper::transformVector(osg::Vec3* vec, osg::Matrixd* mat)
{
	osg::Vec3 cpy(*vec);

	(*vec)[0] = (*mat)(0, 0)*cpy[0] + (*mat)(1, 0)*cpy[1] + (*mat)(2, 0)*cpy[2] + (*mat)(3, 0);
	(*vec)[1] = (*mat)(0, 1)*cpy[0] + (*mat)(1, 1)*cpy[1] + (*mat)(2, 1)*cpy[2] + (*mat)(3, 1);
	(*vec)[2] = (*mat)(0, 2)*cpy[0] + (*mat)(1, 2)*cpy[1] + (*mat)(2, 2)*cpy[2] + (*mat)(3, 2);
}

osg::Quat osgHelper::getQuatFromEuler(double pitch, double roll, double yaw)
{
	osg::Quat q;

	const auto c1 = cos(roll / 2.0);
	const auto s1 = sin(roll / 2.0);
	const auto c2 = cos(yaw / 2.0);
	const auto s2 = sin(yaw / 2.0);
	const auto c3 = cos(pitch / 2.0);
	const auto s3 = sin(pitch / 2.0);

	const auto c1c2 = c1*c2;
	const auto s1s2 = s1*s2;

	q[0] = c1c2*s3 + s1s2*c3;
	q[1] = s1*c2*c3 + c1*s2*s3;
	q[2] = c1*s2*c3 - s1*c2*s3;
	q[3] = c1c2*c3 - s1s2*s3;

	return q;
}

osg::Matrix osgHelper::getMatrixFromEuler(double pitch, double roll, double yaw)
{
	const auto quat = getQuatFromEuler(pitch, roll, yaw);

	osg::Matrix mat;
	mat.setRotate(quat);

	return mat;
}

osg::Vec3f osgHelper::getVec3FromEuler(double pitch, double roll, double yaw, osg::Vec3 origin)
{
	rotateVector(&origin, osgHelper::getQuatFromEuler(pitch, roll, yaw));

	return origin;
}

osg::Vec2f osgHelper::getPolarFromCartesian(const osg::Vec3f& cartesian)
{
	osg::Vec2f result;

	result.x()       = (atan2(-cartesian.x(), cartesian.y()) + C_PI) / (2.0f * C_PI);
	const auto xyLen = sqrt(cartesian.x() * cartesian.x() + cartesian.y() * cartesian.y());
	result.y()       = (atan2(-cartesian.z(), xyLen) + C_PI / 2.0f) / C_PI;

	return result;
}

osg::Vec3f osgHelper::getCartesianFromPolar(const osg::Vec2f& polar)
{
	const auto mat = osg::Matrix::rotate(getQuatFromEuler(polar.x(), 0.0f, polar.y()));

	return osg::Vec3f(0.0f, 1.0f, 0.0f) * mat;
}

osg::Vec2f osgHelper::getTextSize(const osg::ref_ptr<osgText::Text>& text)
{
	osg::BoundingBox bb;

	osgText::Text::TextureGlyphQuadMap glyphs = text->getTextureGlyphQuadMap();
	for (osgText::Text::TextureGlyphQuadMap::iterator it = glyphs.begin(); it != glyphs.end(); ++it)
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

  const auto width  = bb.xMax() - bb.xMin();
  const auto height = bb.yMax() - bb.yMin();

	return osg::Vec2f(width, height);
}

bool osgHelper::pointInRect(const osg::Vec2f& point, const osg::Vec2f& leftbottom, const osg::Vec2f& righttop)
{
	return (point.x() >= leftbottom.x() && point.y() >= leftbottom.y()
		&& point.x() <= righttop.x() && point.y() <= righttop.y());
}

bool osgHelper::sphereLineIntersection(const osg::Vec3f& sphereCenter,
                                       float sphereRadius,
                                       const osg::Vec3f& lineOrigin,
                                       const osg::Vec3f& lineDirectionNormalized,
                                       osg::Vec3f& result)
{
  const auto a = lineDirectionNormalized * lineDirectionNormalized;
  const auto b = lineDirectionNormalized * ((lineOrigin - sphereCenter) * 2.0f);
  const auto c = (sphereCenter * sphereCenter) + (lineOrigin * lineOrigin) -
                 2.0f * (lineOrigin * sphereCenter) -
                 sphereRadius * sphereRadius;

  auto D = b * b + (-4.0f) * a * c;

  if (D < 0) {
    return false;
  }

  D = sqrtf(D);

  const auto t = a == 0.0f ? 1.0f : (-0.5f) * (b + D) / a;
  if (t > 0.0f) {
    result = lineOrigin + lineDirectionNormalized * t;
  } else {
    return false;
  }

  return true;
}
float osgHelper::pointLineDistance(const osg::Vec3f& origin, const osg::Vec3& direction, const osg::Vec3f& point)
{
	const auto vec = direction ^ (point - origin);

	return vec.length();
}

bool osgHelper::lineBoxIntersection(const osg::BoundingBox& bb, const osg::Vec3f& l1,
	                                  const osg::Vec3f& l2, osg::Vec3f& hit)
{
	const auto& b1 = bb.corner(0);
	const auto& b2 = bb.corner(7);

	if (l2.x() < b1.x() && l1.x() < b1.x()) return false;
	if (l2.x() > b2.x() && l1.x() > b2.x()) return false;
	if (l2.y() < b1.y() && l1.y() < b1.y()) return false;
	if (l2.y() > b2.y() && l1.y() > b2.y()) return false;
	if (l2.z() < b1.z() && l1.z() < b1.z()) return false;
	if (l2.z() > b2.z() && l1.z() > b2.z()) return false;

	if (l1.x() > b1.x() && l1.x() < b2.x() &&
		l1.y() > b1.y() && l1.y() < b2.y() &&
		l1.z() > b1.z() && l1.z() < b2.z()) 
	{
		hit = l1; 
		return true;
	}

	if ((getIntersection(l1.x()-b1.x(), l2.x()-b1.x(), l1, l2, hit) && isInBox(hit, b1, b2, 1))
		|| (getIntersection(l1.y()-b1.y(), l2.y()-b1.y(), l1, l2, hit) && isInBox(hit, b1, b2, 2)) 
		|| (getIntersection(l1.z()-b1.z(), l2.z()-b1.z(), l1, l2, hit) && isInBox(hit, b1, b2, 3)) 
		|| (getIntersection(l1.x()-b2.x(), l2.x()-b2.x(), l1, l2, hit) && isInBox(hit, b1, b2, 1)) 
		|| (getIntersection(l1.y()-b2.y(), l2.y()-b2.y(), l1, l2, hit) && isInBox(hit, b1, b2, 2)) 
		|| (getIntersection(l1.z()-b2.z(), l2.z()-b2.z(), l1, l2, hit) && isInBox(hit, b1, b2, 3)))
	{
		return true;
	}

	return false;
}

void osgHelper::generateTangentAndBinormal(osg::Node* node)
{
	osg::ref_ptr<osg::Group> group = node->asGroup();
	osg::ref_ptr<osg::Geode> geode = node->asGeode();

  if (geode)
  {
    for (unsigned int i = 0; i<geode->getNumDrawables(); i++)
    {
			osg::Geometry *geometry = geode->getDrawable(i)->asGeometry();
      if (geometry)
      {
				osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator();
        tsg->generate(geometry);

        geometry->setVertexAttribArray(6, tsg->getTangentArray());
        geometry->setVertexAttribBinding(6, osg::Geometry::BIND_PER_VERTEX);

        geometry->setVertexAttribArray(7, tsg->getBinormalArray());
        geometry->setVertexAttribBinding(7, osg::Geometry::BIND_PER_VERTEX);
        geometry->setVertexAttribNormalize(7, GL_FALSE);

        geometry->setUseVertexBufferObjects(true);

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

osg::StateAttribute::GLModeValue osgHelper::glModeValueFromBool(bool on)
{
	return on ? osg::StateAttribute::ON : osg::StateAttribute::OFF;
}

std::string osgHelper::lowerString(std::string str)
{
#ifdef WIN32
	transform(str.begin(), str.end(), str.begin(), tolower);
#else
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
#endif

	return str;
}

osg::ref_ptr<osgText::Text> osgHelper::createTextNode(const std::string& text, float characterSize, const osg::ref_ptr<osgText::Font>& font)
{
	osg::ref_ptr<osgText::Text> textNode = new osgText::Text();

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

osg::ref_ptr<osg::Geometry>
osgHelper::createQuadGeometry(float left, float right, float bottom, float top,
                              float z, QuadOrientation orientation,
                              bool flipped)
{
  osg::ref_ptr<osg::Geometry> geo = new osg::Geometry();

  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();

  const auto normal = flipped ? 1.0f : -1.0f;

  switch (orientation)
  {
  case QuadOrientation::XY:
    verts->push_back(osg::Vec3(left, bottom, z));
    verts->push_back(osg::Vec3(left, top, z));
    verts->push_back(osg::Vec3(right, top, z));
    verts->push_back(osg::Vec3(right, bottom, z));

    normals->push_back(osg::Vec3(0.0f, 0.0f, normal));
    break;
  case QuadOrientation::XZ:
    verts->push_back(osg::Vec3(left, z, bottom));
    verts->push_back(osg::Vec3(right, z, bottom));
    verts->push_back(osg::Vec3(right, z, top));
    verts->push_back(osg::Vec3(left, z, top));

    normals->push_back(osg::Vec3(0.0f, normal, 0.0f));
    break;
  case QuadOrientation::YZ:
    verts->push_back(osg::Vec3(z, left, bottom));
    verts->push_back(osg::Vec3(z, left, top));
    verts->push_back(osg::Vec3(z, right, top));
    verts->push_back(osg::Vec3(z, right, bottom));

    normals->push_back(osg::Vec3(normal, 0.0f, 0.0f));
    break;
  }

  osg::ref_ptr<osg::DrawElementsUInt> indices =
      new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);

  if (flipped) {
    indices->push_back(3);
    indices->push_back(2);
    indices->push_back(1);
    indices->push_back(0);
  } else {
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(2);
    indices->push_back(3);
  }

  // ref_ptr<Vec4Array> colors = new Vec4Array();
  // colors->push_back(color);

  osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array();
  texcoords->push_back(osg::Vec2(0.0f, 1.0f));
  texcoords->push_back(osg::Vec2(0.0f, 0.0f));
  texcoords->push_back(osg::Vec2(1.0f, 0.0f));
  texcoords->push_back(osg::Vec2(1.0f, 1.0f));

	geo->setTexCoordArray(0, texcoords);
	geo->addPrimitiveSet(indices);
	geo->setVertexArray(verts);
	geo->setNormalArray(normals);
	geo->setNormalBinding(osg::Geometry::BIND_OVERALL);
	//geo->setColorArray(colors);
	//geo->setColorBinding(osg::Geometry::BIND_OVERALL);

	return geo;
}

osgHelper::StringList& osgHelper::splitString(const std::string &s, char delim, StringList &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (getline(ss, item, delim))
	{
		elems.push_back(item);
	}

	return elems;
}

osgHelper::StringList osgHelper::splitString(const std::string &s, char delim)
{
	StringList elems;
	splitString(s, delim, elems);
	return elems;
}

std::string osgHelper::utf8ToLatin1(const char* in)
{
	std::string out;
	if (in == nullptr)
	{
		return out;
	}

	unsigned int codepoint = 0;
	while (*in != 0)
	{
		const auto ch = static_cast<unsigned char>(*in);
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
