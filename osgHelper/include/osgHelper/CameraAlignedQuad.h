#pragma once

#include <osg/Geode>
#include <osg/Geometry>

namespace osgHelper
{
	class CameraAlignedQuad : public osg::Geode
	{
	public:
    using Ptr = osg::ref_ptr<CameraAlignedQuad>;

		explicit CameraAlignedQuad(int renderBin = 10);

		osg::ref_ptr<osg::Geometry> getGeometry() const;
		osg::ref_ptr<osg::Vec3Array> getVertexArray() const;
		osg::ref_ptr<osg::Vec3Array> getNormalArray() const;

	private:
		void makeQuad(int renderBin);

		osg::ref_ptr<osg::Geometry> m_geometry;
		osg::ref_ptr<osg::Vec3Array> m_vertexArray;
		osg::ref_ptr<osg::Vec3Array> m_normalArray;
	};
}